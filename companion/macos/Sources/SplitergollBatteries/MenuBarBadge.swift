// Dessine les deux lignes et la batterie avec éclair dans une image adaptée à la barre des menus.
import AppKit
import BatteryCore

@MainActor
enum MenuBarBadge {
    // Les symboles sont dessinés comme images, avec un équivalent textuel séparé pour l’accessibilité.
    static func image(state: BatteryState, at now: Date) -> NSImage {
        let attributes: [NSAttributedString.Key: Any] = [
            .font: NSFont.monospacedDigitSystemFont(ofSize: 9, weight: .medium),
            .foregroundColor: NSColor.black,
        ]
        let symbol = NSImage(systemSymbolName: "battery.100percent.bolt", accessibilityDescription: nil)?
            .withSymbolConfiguration(.init(pointSize: 9, weight: .medium))
        let indications = BatterySide.allCases.map { state.indication(for: $0, at: now) }
        let labels = zip(BatterySide.allCases, indications).map { side, indication in
            let value = indication == .possibleCharge && symbol != nil ? "" : indication.menuValue
            return NSAttributedString(string: "\(side.shortName) \(value)", attributes: attributes)
        }
        let symbolSize = NSSize(width: 17, height: 9)
        let widths = labels.enumerated().map { index, label in
            label.size().width
                + (indications[index] == .possibleCharge && symbol != nil ? symbolSize.width : 0)
        }
        let width = ceil(widths.max() ?? 0) + 4
        let image = NSImage(size: NSSize(width: width, height: 22), flipped: false) { bounds in
            let rowHeight = bounds.height / CGFloat(labels.count)
            for (index, label) in labels.enumerated() {
                let x = (bounds.width - widths[index]) / 2
                let rowBottom = bounds.height - CGFloat(index + 1) * rowHeight
                label.draw(at: NSPoint(x: x, y: rowBottom + (rowHeight - label.size().height) / 2))
                if indications[index] == .possibleCharge, let symbol {
                    let origin = NSPoint(
                        x: x + label.size().width, y: rowBottom + (rowHeight - symbolSize.height) / 2)
                    symbol.draw(in: NSRect(origin: origin, size: symbolSize))
                }
            }
            return true
        }
        image.isTemplate = true
        return image
    }
}
