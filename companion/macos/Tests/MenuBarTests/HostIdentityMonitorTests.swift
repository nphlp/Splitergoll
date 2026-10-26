import Foundation
import XCTest

@testable import SplitergollBatteries

final class HostIdentityMonitorTests: XCTestCase {
    // Les erreurs système peuvent contenir des identités : leur message libre n’est jamais exporté.
    @MainActor func testErrorDoesNotExportPrivateUserInfo() {
        let error = NSError(
            domain: "private-domain", code: 8, userInfo: [NSLocalizedDescriptionKey: "private-identity"])
        XCTAssertEqual(HostIdentityMonitor.errorLabel(error), "BLE 8")
    }
    // Sans sélection, aucune ouverture Bluetooth/USB ou création d’identité n’est déclenchée.
    @MainActor func testNoSelectionDoesNotStartDiagnostic() {
        let monitor = HostIdentityMonitor()
        monitor.start(selected: nil)
        XCTAssertFalse(monitor.running)
        XCTAssertNil(monitor.usb)
        XCTAssertNil(monitor.ble)
        XCTAssertEqual(monitor.bluetoothText, "Choisir d’abord le clavier suivi")
    }

    // Un arrêt répété ou le sommeil garde le diagnostic inactif et ses preuves invalidées.
    @MainActor func testStopAndSleepAreIdempotent() {
        let monitor = HostIdentityMonitor()
        monitor.stop()
        monitor.stop()
        monitor.pauseForSleep()
        XCTAssertFalse(monitor.running)
        XCTAssertNil(monitor.usb)
        XCTAssertNil(monitor.ble)
        XCTAssertEqual(monitor.verdict, "Correspondance non établie")
    }
}
