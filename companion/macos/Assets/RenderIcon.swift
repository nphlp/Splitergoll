// Convertit le SVG du PCB en tailles PNG pour l’icône macOS, sans dépendance externe.
// Exécuté par app.sh avant l’assemblage ICNS. Aucun accès Bluetooth ou réglage système.
import AppKit

@main
enum RenderIcon {
    // Écrit les dix représentations utilisées par iconutil dans le dossier reçu du script de build.
    @MainActor static func main() throws {
        guard CommandLine.arguments.count == 3 else { throw CocoaError(.fileWriteInvalidFileName) }
        let source = URL(fileURLWithPath: CommandLine.arguments[1])
        guard let image = NSImage(contentsOf: source), image.isValid else {
            throw CocoaError(.fileReadCorruptFile)
        }
        let directory = URL(fileURLWithPath: CommandLine.arguments[2], isDirectory: true)
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        for points in [16, 32, 128, 256, 512] {
            for scale in [1, 2] {
                let pixels = points * scale
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
                context.cgContext.scaleBy(x: CGFloat(pixels) / 1024, y: CGFloat(pixels) / 1024)
                image.draw(in: NSRect(x: 0, y: 0, width: 1024, height: 1024))
                NSGraphicsContext.restoreGraphicsState()
                guard let png = bitmap.representation(using: .png, properties: [:]) else {
                    throw CocoaError(.fileWriteUnknown)
                }
                let suffix = scale == 2 ? "@2x" : ""
                try png.write(to: directory.appendingPathComponent("icon_\(points)x\(points)\(suffix).png"))
            }
        }
    }

}
