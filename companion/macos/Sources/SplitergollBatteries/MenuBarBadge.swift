// Dessine les contours des deux moitiés et leurs niveaux dans la barre des menus.
import AppKit
import BatteryCore

@MainActor
enum MenuBarBadge {
    // Les SVG embarqués restent vectoriels, transparents et indépendants du dépôt installé.
    static func contour(for side: BatterySide) -> NSImage {
        guard
            let url = Bundle.module.url(
                forResource: "keyboard-\(side.rawValue)", withExtension: "svg", subdirectory: "Resources"),
            let image = NSImage(contentsOf: url), image.isValid
        else { preconditionFailure("Contour du clavier absent du bundle") }
        return image
    }

    private static let contours = BatterySide.allCases.map { contour(for: $0) }

    // Couche l’éclair sans le déformer pour utiliser la largeur disponible du pourcentage.
    private static func drawChargingSymbol(_ symbol: NSImage, in rect: NSRect) {
        NSGraphicsContext.saveGraphicsState()
        let rotation = NSAffineTransform()
        rotation.translateX(by: rect.maxX, yBy: rect.minY)
        rotation.rotate(byDegrees: 90)
        rotation.concat()
        symbol.draw(in: NSRect(x: 0, y: 0, width: rect.height, height: rect.width))
        NSGraphicsContext.restoreGraphicsState()
    }

    // Les symboles sont dessinés comme images, avec un équivalent textuel séparé pour l’accessibilité.
    static func image(state: BatteryState, at now: Date) -> NSImage {
        let attributes: [NSAttributedString.Key: Any] = [
            .font: NSFont.monospacedDigitSystemFont(ofSize: 9, weight: .medium),
            .foregroundColor: NSColor.black,
        ]
        let symbol = NSImage(systemSymbolName: "bolt.fill", accessibilityDescription: nil)?
            .withSymbolConfiguration(.init(pointSize: 12, weight: .medium))
        let indications = BatterySide.allCases.map { state.indication(for: $0, at: now) }
        let labels = indications.map { indication in
            let value = indication == .possibleCharge && symbol != nil ? "" : indication.menuValue
            return NSAttributedString(string: value, attributes: attributes)
        }
        let valueWidth = ceil(
            NSAttributedString(string: "99 %", attributes: attributes).size().width)
        let symbolSize = BatterySymbolLayout.fittedSize(
            NSSize(width: symbol?.size.height ?? 0, height: symbol?.size.width ?? 0),
            within: NSSize(width: valueWidth, height: 9))
        let contourSize = NSSize(width: 12, height: 10)
        let gap: CGFloat = 3
        let padding: CGFloat = 2
        let width = padding * 2 + contourSize.width + gap + valueWidth
        let image = NSImage(size: NSSize(width: width, height: 22), flipped: false) { bounds in
            let rowHeight = bounds.height / CGFloat(labels.count)
            for (index, label) in labels.enumerated() {
                let isLeft = BatterySide.allCases[index] == .left
                let rowBottom = bounds.height - CGFloat(index + 1) * rowHeight
                let contourX = isLeft ? padding : bounds.width - padding - contourSize.width
                let contourOrigin = NSPoint(
                    x: contourX, y: rowBottom + (rowHeight - contourSize.height) / 2)
                contours[index].draw(in: NSRect(origin: contourOrigin, size: contourSize))

                let isCharge = indications[index] == .possibleCharge && symbol != nil
                let valueSize = isCharge ? symbolSize : label.size()
                let valueX = isLeft ? contourX + contourSize.width + gap : contourX - gap - valueSize.width
                let valueOrigin = NSPoint(x: valueX, y: rowBottom + (rowHeight - valueSize.height) / 2)
                if indications[index] == .possibleCharge, let symbol {
                    drawChargingSymbol(symbol, in: NSRect(origin: valueOrigin, size: symbolSize))
                } else {
                    label.draw(at: valueOrigin)
                }
            }
            return true
        }
        image.isTemplate = true
        return image
    }
}
