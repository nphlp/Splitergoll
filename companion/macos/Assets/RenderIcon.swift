// Convertit le SVG retenu en ICNS avec son ombre extérieure centrée, via les API natives macOS.
// AppKit dessine le vectoriel, Core Image applique le flou du SVG que CoreSVG ignore.
// Exécuté uniquement au build, sans dépendance externe, Bluetooth ou réglage système.
import AppKit
import CoreImage

@main
enum RenderIcon {
    // Lit le filtre extérieur du SVG, sans inventer de réglage ni ignorer un effet non pris en charge.
    private static func shadow(in document: XMLDocument) throws -> (radius: Double, opacity: Double) {
        let filters = try document.nodes(forXPath: "//*[local-name()='filter']")
        guard let filter = filters.first as? XMLElement, filters.count == 1 else {
            throw CocoaError(.fileReadCorruptFile)
        }
        let effects = filter.children?.compactMap { $0 as? XMLElement } ?? []
        guard
            effects.map(\.localName) == ["feGaussianBlur", "feOffset", "feFlood", "feComposite", "feMerge"],
            effects[0].attribute(forName: "in")?.stringValue == "SourceAlpha",
            let radius = Double(effects[0].attribute(forName: "stdDeviation")?.stringValue ?? ""),
            radius.isFinite, radius > 0,
            effects[1].attribute(forName: "dx")?.stringValue == "0",
            effects[1].attribute(forName: "dy")?.stringValue == "0",
            effects[2].attribute(forName: "flood-color")?.stringValue == "#000000",
            let opacity = Double(effects[2].attribute(forName: "flood-opacity")?.stringValue ?? ""),
            opacity.isFinite, (0...1).contains(opacity),
            effects[3].attribute(forName: "operator")?.stringValue == "in"
        else { throw CocoaError(.fileReadCorruptFile) }
        return (radius, opacity)
    }

    // Sépare le fond et le contour pour que l’ombre suive seulement le clavier.
    @MainActor private static func layer(_ name: String, in document: XMLDocument) throws -> NSImage {
        guard let root = document.rootElement()?.copy() as? XMLElement else {
            throw CocoaError(.fileReadCorruptFile)
        }
        for child in root.children?.compactMap({ $0 as? XMLElement }) ?? [] {
            if child.localName != name { child.detach() }
            child.removeAttribute(forName: "filter")
        }
        let data = XMLDocument(rootElement: root).xmlData
        guard let image = NSImage(data: data), image.isValid else {
            throw CocoaError(.fileReadCorruptFile)
        }
        return image
    }

    // Rasterise une couche à la taille demandée en conservant la transparence des coins.
    @MainActor private static func bitmap(_ image: NSImage, pixels: Int) throws -> NSBitmapImageRep {
        guard
            let bitmap = NSBitmapImageRep(
                bitmapDataPlanes: nil, pixelsWide: pixels, pixelsHigh: pixels,
                bitsPerSample: 8, samplesPerPixel: 4, hasAlpha: true, isPlanar: false,
                colorSpaceName: .deviceRGB, bytesPerRow: 0, bitsPerPixel: 0),
            let context = NSGraphicsContext(bitmapImageRep: bitmap)
        else { throw CocoaError(.fileWriteUnknown) }
        NSGraphicsContext.saveGraphicsState()
        NSGraphicsContext.current = context
        context.imageInterpolation = .high
        image.draw(in: NSRect(x: 0, y: 0, width: pixels, height: pixels))
        NSGraphicsContext.restoreGraphicsState()
        return bitmap
    }

    // Reproduit SourceAlpha, le flou gaussien noir et la fusion derrière le contour du SVG.
    @MainActor private static func render(_ document: XMLDocument) throws -> NSImage {
        let shadow = try shadow(in: document)
        let foregroundBitmap = try bitmap(layer("g", in: document), pixels: 1024)
        let backgroundBitmap = try bitmap(layer("rect", in: document), pixels: 1024)
        guard let foregroundCG = foregroundBitmap.cgImage, let backgroundCG = backgroundBitmap.cgImage,
            let colorSpace = CGColorSpace(name: CGColorSpace.sRGB)
        else { throw CocoaError(.fileWriteUnknown) }
        let foreground = CIImage(cgImage: foregroundCG)
        let background = CIImage(cgImage: backgroundCG)
        let zero = CIVector(x: 0, y: 0, z: 0, w: 0)
        let blurred = foreground.applyingFilter(
            "CIColorMatrix",
            parameters: [
                "inputRVector": zero, "inputGVector": zero, "inputBVector": zero,
                "inputAVector": CIVector(x: 0, y: 0, z: 0, w: shadow.opacity),
            ]
        ).applyingFilter("CIGaussianBlur", parameters: [kCIInputRadiusKey: shadow.radius])
        let result = foreground.composited(over: blurred.composited(over: background))
        let context = CIContext(options: [
            .useSoftwareRenderer: true, .workingColorSpace: colorSpace, .outputColorSpace: colorSpace,
        ])
        guard let rendered = context.createCGImage(result, from: background.extent) else {
            throw CocoaError(.fileWriteUnknown)
        }
        return NSImage(cgImage: rendered, size: NSSize(width: 1024, height: 1024))
    }

    // Écrit les dix représentations utilisées par iconutil dans le dossier reçu du script de build.
    @MainActor static func main() throws {
        guard CommandLine.arguments.count == 3 else { throw CocoaError(.fileWriteInvalidFileName) }
        let source = URL(fileURLWithPath: CommandLine.arguments[1])
        let document = try XMLDocument(contentsOf: source)
        let image = try render(document)
        let directory = URL(fileURLWithPath: CommandLine.arguments[2], isDirectory: true)
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        for points in [16, 32, 128, 256, 512] {
            for scale in [1, 2] {
                let bitmap = try bitmap(image, pixels: points * scale)
                guard let png = bitmap.representation(using: .png, properties: [:]) else {
                    throw CocoaError(.fileWriteUnknown)
                }
                let suffix = scale == 2 ? "@2x" : ""
                try png.write(to: directory.appendingPathComponent("icon_\(points)x\(points)\(suffix).png"))
            }
        }
    }
}
