import AppKit
import BatteryCore
import XCTest

@testable import SplitergollBatteries

final class MenuBarBadgeTests: XCTestCase {
    // Les ressources sont deux vrais contours transparents, symétriques, sans silhouette pleine.
    @MainActor func testContourResourcesAreTransparentMirrors() throws {
        let left = try rasterize(MenuBarBadge.contour(for: .left), size: NSSize(width: 24, height: 20))
        let right = try rasterize(MenuBarBadge.contour(for: .right), size: NSSize(width: 24, height: 20))
        var difference = 0.0
        var visiblePixels = 0
        for y in 0..<left.pixelsHigh {
            for x in 0..<left.pixelsWide {
                let alpha = try XCTUnwrap(left.colorAt(x: x, y: y)).alphaComponent
                let mirrored = try XCTUnwrap(right.colorAt(x: left.pixelsWide - 1 - x, y: y)).alphaComponent
                difference += abs(alpha - mirrored)
                if alpha > 0.01 { visiblePixels += 1 }
            }
        }
        let pixels = Double(left.pixelsWide * left.pixelsHigh)
        XCTAssertLessThan(difference / pixels, 0.02)
        XCTAssertGreaterThan(visiblePixels, 0)
        XCTAssertLessThan(Double(visiblePixels) / pixels, 0.4)
        XCTAssertEqual(try XCTUnwrap(left.colorAt(x: 0, y: 0)).alphaComponent, 0)
        XCTAssertEqual(
            try XCTUnwrap(left.colorAt(x: left.pixelsWide / 2, y: left.pixelsHigh / 2)).alphaComponent, 0)
    }

    // Charge, zéro et absence de données gardent chaque moitié du bon côté de sa valeur.
    @MainActor func testMenuKeepsContoursOnExpectedSideForEveryIndication() throws {
        let now = Date(timeIntervalSince1970: 1_000)
        for levels in [(63, 67), (100, 0), (0, 100), (1, 99)] {
            let image = MenuBarBadge.image(
                state: try state(left: levels.0, right: levels.1, at: now), at: now)
            for side in BatterySide.allCases {
                let source = NSRect(
                    x: side == .left ? 2 : image.size.width - 14,
                    y: side == .left ? 11.5 : 0.5, width: 12, height: 10)
                let actual = try rasterize(image, size: source.size, source: source)
                let expected = try rasterize(MenuBarBadge.contour(for: side), size: source.size)
                var difference = 0.0
                for y in 0..<actual.pixelsHigh {
                    for x in 0..<actual.pixelsWide {
                        difference += abs(
                            try XCTUnwrap(actual.colorAt(x: x, y: y)).alphaComponent
                                - XCTUnwrap(expected.colorAt(x: x, y: y)).alphaComponent)
                    }
                }
                XCTAssertLessThan(difference / Double(actual.pixelsWide * actual.pixelsHigh), 0.02)
            }
        }
    }

    // Cent pour cent occupe la colonne avec un éclair horizontal, sans étirement ni pourcentage.
    @MainActor func testFullLevelUsesOnlyAHorizontalBoltOnBothSides() throws {
        let now = Date(timeIntervalSince1970: 1_000)
        let image = MenuBarBadge.image(state: try state(left: 100, right: 100, at: now), at: now)
        let symbol = try XCTUnwrap(
            NSImage(systemSymbolName: "bolt.fill", accessibilityDescription: nil)?
                .withSymbolConfiguration(.init(pointSize: 12, weight: .medium)))
        let rotatedSize = NSSize(width: symbol.size.height, height: symbol.size.width)
        let symbolSize = BatterySymbolLayout.fittedSize(
            rotatedSize, within: NSSize(width: image.size.width - 19, height: 9))
        XCTAssertGreaterThan(symbolSize.width, symbolSize.height)
        XCTAssertEqual(symbolSize.height, 9, accuracy: 0.001)
        XCTAssertEqual(
            symbolSize.width / symbolSize.height, symbol.size.height / symbol.size.width, accuracy: 0.001)

        for side in BatterySide.allCases {
            let valueSize = NSSize(width: image.size.width - 19, height: 11)
            let source = NSRect(
                x: side == .left ? 17 : 2, y: side == .left ? 11 : 0,
                width: valueSize.width, height: valueSize.height)
            let expectedImage = NSImage(size: valueSize, flipped: false) { _ in
                NSGraphicsContext.saveGraphicsState()
                let rotation = NSAffineTransform()
                rotation.translateX(
                    by: (side == .left ? 0 : valueSize.width - symbolSize.width) + symbolSize.width,
                    yBy: (valueSize.height - symbolSize.height) / 2)
                rotation.rotate(byDegrees: 90)
                rotation.concat()
                symbol.draw(
                    in: NSRect(
                        x: 0, y: 0, width: symbolSize.height, height: symbolSize.width))
                NSGraphicsContext.restoreGraphicsState()
                return true
            }
            let actual = try rasterize(image, size: valueSize, source: source)
            let expected = try rasterize(expectedImage, size: valueSize)
            var difference = 0.0
            var visibleAlpha = 0.0
            for y in 0..<actual.pixelsHigh {
                for x in 0..<actual.pixelsWide {
                    let alpha = try XCTUnwrap(actual.colorAt(x: x, y: y)).alphaComponent
                    let expectedAlpha = try XCTUnwrap(expected.colorAt(x: x, y: y)).alphaComponent
                    difference += abs(alpha - expectedAlpha)
                    visibleAlpha += alpha
                }
            }
            XCTAssertGreaterThan(visibleAlpha, 1)
            XCTAssertLessThan(difference / Double(actual.pixelsWide * actual.pixelsHigh), 0.02)
        }
    }

    // Les variations de batterie ou de connexion ne déplacent pas les autres éléments de la barre.
    @MainActor func testMenuWidthRemainsStableWhenChargeOrConnectionChanges() throws {
        let now = Date(timeIntervalSince1970: 1_000)
        let reference = MenuBarBadge.image(state: BatteryState(), at: now)
        for levels in [(63, 67), (100, 100), (0, 0), (1, 99)] {
            var state = try state(left: levels.0, right: levels.1, at: now)
            for phase in [MonitorPhase.connected, .disconnected, .sleeping] {
                state.phase = phase
                let image = MenuBarBadge.image(state: state, at: now)
                XCTAssertTrue(image.isTemplate)
                XCTAssertEqual(image.size, reference.size)
                XCTAssertEqual(image.size.height, 22)
            }
        }
    }

    // Produit des lectures fictives sans instancier le lecteur Bluetooth ni l’application.
    private func state(left: Int, right: Int, at now: Date) throws -> BatteryState {
        var state = BatteryState()
        state.beginSession()
        for (index, level) in [left, right].enumerated() {
            let source: UInt8 = index == 0 ? 6 : 8
            try state.identify(channel: index, presentation: Data([4, 0, 0xAD, 0x27, 1, source, 1]))
            try state.receive(channel: index, data: Data([UInt8(level)]), at: now)
        }
        state.phase = .connected
        return state
    }

    // Rend une image ou une zone en mémoire, sans fenêtre, avec quatre pixels par point.
    @MainActor private func rasterize(_ image: NSImage, size: NSSize, source: NSRect = .zero) throws
        -> NSBitmapImageRep
    {
        let bitmap = try XCTUnwrap(
            NSBitmapImageRep(
                bitmapDataPlanes: nil, pixelsWide: Int(size.width * 4), pixelsHigh: Int(size.height * 4),
                bitsPerSample: 8, samplesPerPixel: 4, hasAlpha: true, isPlanar: false,
                colorSpaceName: .deviceRGB, bytesPerRow: 0, bitsPerPixel: 0))
        let context = try XCTUnwrap(NSGraphicsContext(bitmapImageRep: bitmap))
        NSGraphicsContext.saveGraphicsState()
        NSGraphicsContext.current = context
        context.cgContext.scaleBy(x: 4, y: 4)
        image.draw(in: NSRect(origin: .zero, size: size), from: source, operation: .sourceOver, fraction: 1)
        NSGraphicsContext.restoreGraphicsState()
        return bitmap
    }
}
