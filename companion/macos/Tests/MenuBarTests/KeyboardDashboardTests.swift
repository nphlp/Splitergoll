import AppKit
import BatteryCore
import SwiftUI
import XCTest

@testable import SplitergollBatteries

@MainActor
private final class DashboardKeyboardPreferences: KeyboardPreferenceStorage {
    var data: Data?
}

final class KeyboardDashboardTests: XCTestCase {
    // Le tableau est rendu hors écran, sans démarrage des lecteurs ni ouverture de fenêtre.
    @MainActor func testUnifiedDashboardFitsAtFixedWidthWithoutStartingReaders() {
        let monitor = BatteryMonitor(targetName: "Splitergoll", preferences: DashboardKeyboardPreferences())
        let diagnostic = HostIdentityMonitor()
        monitor.updateKeyboards([
            KeyboardIdentity(id: UUID(), name: "Splitergoll V2 avec un nom particulièrement long"),
            KeyboardIdentity(id: UUID(), name: "Splitergoll V1"),
        ])
        let view = NSHostingView(
            rootView: BatteriesView(
                monitor: monitor, diagnostic: diagnostic,
                select: { monitor.selectKeyboard($0) }, restart: {}, resize: {}))
        let size = view.fittingSize
        XCTAssertEqual(size.width, BatteriesView.width, accuracy: 1)
        XCTAssertGreaterThan(size.height, 300)
        XCTAssertLessThan(size.height, 520)
        view.frame = NSRect(origin: .zero, size: size)
        view.layoutSubtreeIfNeeded()
        XCTAssertFalse(diagnostic.running)
        XCTAssertNil(diagnostic.currentRuntime)
        XCTAssertTrue(diagnostic.diagnosticText.contains("OS actif : Non communiqué"))
        XCTAssertTrue(diagnostic.diagnosticText.contains("Saisie : Non communiquée"))
    }

    // Un statut sans preuve BLE courante ne peut apparaître comme celui du clavier suivi.
    @MainActor func testRuntimeWithoutCurrentBluetoothContextIsNotDisplayed() {
        let diagnostic = HostIdentityMonitor()
        var frame = [UInt8](repeating: 0, count: 32)
        frame.replaceSubrange(0..<8, with: [83, 71, 83, 84, 1, 2, 2, 1])
        frame[8] = 1
        diagnostic.receivedRuntime(Data(frame), error: nil)
        XCTAssertNil(diagnostic.runtime)
        XCTAssertNil(diagnostic.currentRuntime)
        diagnostic.receivedRuntime(Data(frame), error: NSError(domain: "private", code: 1))
        XCTAssertNil(diagnostic.runtime)
        XCTAssertFalse(diagnostic.diagnosticText.contains("private"))
    }

    // Les détails techniques prennent leur hauteur naturelle, sans largeur supplémentaire ni fenêtre dédiée.
    @MainActor func testExpandedDiagnosticKeepsItsContentInsideTheDashboardWidth() {
        let diagnostic = HostIdentityMonitor()
        let content = HostIdentityView(
            diagnostic: diagnostic, restart: {}, resize: {}, expanded: .constant(true)
        )
        .padding(18).frame(width: BatteriesView.width).fixedSize(horizontal: false, vertical: true)
        let view = NSHostingView(rootView: content)
        let size = view.fittingSize
        XCTAssertEqual(size.width, BatteriesView.width, accuracy: 1)
        XCTAssertGreaterThan(size.height, 240)
        XCTAssertLessThan(size.height, 500)
        view.frame = NSRect(origin: .zero, size: size)
        view.layoutSubtreeIfNeeded()
        XCTAssertFalse(diagnostic.running)
    }

    // Un inventaire long défile dans le tableau plutôt que d’étirer la fenêtre au-delà de l’écran.
    @MainActor func testConnectionTableBoundsLongInventoriesAndHandlesAnEmptyList() {
        var selection = KeyboardSelection()
        let keyboards = (1...10).map {
            KeyboardIdentity(id: UUID(), name: "Splitergoll V\($0) avec un nom particulièrement long")
        }
        selection.update(keyboards, targetName: "Splitergoll")
        let rows = KeyboardConnections.rows(
            selection: selection, usb: nil, ble: nil, runtime: nil, at: Date())
        XCTAssertEqual(rows.count, 10)
        for values in [[], rows] {
            let table = KeyboardConnectionsView(
                rows: values, select: { _ in XCTFail("Aucune sélection au rendu") }
            )
            .padding(18).frame(width: BatteriesView.width).fixedSize(horizontal: false, vertical: true)
            let view = NSHostingView(rootView: table)
            let size = view.fittingSize
            XCTAssertEqual(size.width, BatteriesView.width, accuracy: 1)
            XCTAssertGreaterThan(size.height, 65)
            XCTAssertLessThan(size.height, 255)
        }
    }
}
