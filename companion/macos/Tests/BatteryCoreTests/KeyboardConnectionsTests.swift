import Foundation
import XCTest

@testable import BatteryCore

final class KeyboardConnectionsTests: XCTestCase {
    private let now = Date(timeIntervalSince1970: 1_000)
    private let v1 = KeyboardIdentity(id: UUID(), name: "Splitergoll V1")
    private let v2 = KeyboardIdentity(id: UUID(), name: "Splitergoll V2")

    // Le tableau reçoit les mêmes identités que le sélecteur persistant, pas une correspondance par nom.
    private func selection() -> KeyboardSelection {
        var selection = KeyboardSelection()
        selection.update([v1, v2], targetName: "Splitergoll")
        selection.select(v2.id)
        return selection
    }

    // Simule une observation publique du diagnostic sans identité d’ordinateur réelle.
    private func observation(
        _ transport: HostIdentityProtocol.Transport, boot: UInt8 = 1, age: TimeInterval = 0
    ) throws -> HostIdentityProtocol.Observation {
        var bytes = [UInt8](repeating: 0, count: 64)
        bytes.replaceSubrange(0..<8, with: [83, 71, 73, 68, 1, 1, transport.rawValue, 0])
        bytes[8] = boot
        bytes[24] = 2
        bytes[58] = 1
        bytes[60] = transport == .usb ? 255 : 0
        return .init(
            report: try HostIdentityProtocol.Report(Data(bytes), transport: transport),
            received: now.addingTimeInterval(-age))
    }

    // Le statut natif peut cibler un autre ordinateur que celui qui exécute l’app.
    private func runtime(
        _ transport: KeyboardRuntime.Transport, profile: UInt8 = 0, age: TimeInterval = 0,
        priority: Bool = false
    ) throws -> KeyboardRuntime {
        var bytes = [UInt8](repeating: 0, count: 32)
        bytes.replaceSubrange(0..<8, with: [83, 71, 83, 84, 1, 1, transport.rawValue, profile])
        bytes[8] = 1
        if priority {
            bytes[4] = 2
            bytes[24] = 1
        }
        return try KeyboardRuntime(Data(bytes), received: now.addingTimeInterval(-age))
    }

    // Tous les claviers BLE disponibles apparaissent même sans diagnostic ou OS communiqué.
    func testAllKnownBluetoothKeyboardsAreListedWithoutInventingUSB() {
        let rows = KeyboardConnections.rows(selection: selection(), usb: nil, ble: nil, runtime: nil, at: now)
        XCTAssertEqual(rows.map(\.keyboardID), [v1.id, v2.id])
        XCTAssertEqual(rows.map(\.link), [.bluetooth, .bluetooth])
        XCTAssertEqual(rows.map(\.state), [.connected, .connected])
        XCTAssertEqual(rows.map(\.followed), [false, true])
    }

    // Le même contrôleur peut avoir deux liaisons visibles, sans annoncer une priorité USB inexistante.
    func testMatchedControllerAddsUSBButKeepsRealBluetoothInput() throws {
        let rows = KeyboardConnections.rows(
            selection: selection(), usb: try observation(.usb), ble: try observation(.bluetooth),
            runtime: try runtime(.bluetooth), at: now)
        XCTAssertEqual(rows.map(\.keyboardID), [v1.id, v2.id, v2.id])
        XCTAssertEqual(rows.map(\.link), [.bluetooth, .usb, .bluetooth])
        XCTAssertEqual(rows.map(\.state), [.connected, .diagnostic, .input])
        XCTAssertEqual(rows.map(\.isSelected), [false, false, true])
        XCTAssertEqual(rows[2].state.rawValue, "Sélectionné")
        XCTAssertEqual(Set(rows.map(\.id)).count, 3)
    }

    // Une sortie USB effective signifie que BLE attend, pas que sa radio est désactivée.
    func testActualUSBInputIsNeverPresentedAsBluetoothDisabled() throws {
        let rows = KeyboardConnections.rows(
            selection: selection(), usb: try observation(.usb), ble: try observation(.bluetooth),
            runtime: try runtime(.usb), at: now)
        XCTAssertEqual(rows.map(\.state), [.connected, .input, .standby])
        XCTAssertEqual(rows.map(\.isSelected), [false, true, false])
        XCTAssertEqual(rows[2].state.rawValue, "Désactivé")
    }

    // Un Mac lecteur GATT ne reçoit pas nécessairement les frappes du clavier sélectionné sur Windows.
    func testBluetoothToAnotherHostIsNotMarkedAsLocalInput() throws {
        let rows = KeyboardConnections.rows(
            selection: selection(), usb: try observation(.usb), ble: try observation(.bluetooth),
            runtime: try runtime(.bluetooth, profile: 1), at: now)
        XCTAssertEqual(rows.map(\.state), [.connected, .diagnostic, .otherHost])
        XCTAssertFalse(rows.contains(where: \.isSelected))
    }

    // Ancien contexte, mauvais transport et horodatage futur ne créent aucun faux rapprochement.
    func testStaleOrDifferentControllerDoesNotCreateUSBRow() throws {
        let ble = try observation(.bluetooth)
        for usb in [
            try observation(.usb, boot: 2), try observation(.usb, age: 15),
            try observation(.usb, age: -1), ble,
        ] {
            let rows = KeyboardConnections.rows(
                selection: selection(), usb: usb, ble: ble, runtime: nil, at: now)
            XCTAssertEqual(rows.map(\.link), [.bluetooth, .bluetooth])
        }
        let rows = KeyboardConnections.rows(
            selection: selection(), usb: try observation(.usb), ble: try observation(.bluetooth, age: 15),
            runtime: try runtime(.bluetooth), at: now)
        XCTAssertEqual(rows.map(\.link), [.bluetooth, .bluetooth])
        XCTAssertEqual(rows.map(\.state), [.connected, .connected])
    }

    // Le clavier préféré absent reste visible et ne peut pas être sélectionné comme une liaison connectée.
    func testAbsentPreferenceAndDuplicateNamesStayDistinct() {
        var selection = selection()
        selection.update([v1], targetName: "Splitergoll")
        let rows = KeyboardConnections.rows(selection: selection, usb: nil, ble: nil, runtime: nil, at: now)
        XCTAssertEqual(rows.map(\.state), [.connected, .disconnected])
        XCTAssertFalse(rows[1].canFollow)
        XCTAssertTrue(rows[1].followed)

        selection.update([v1, KeyboardIdentity(id: v2.id, name: v1.name)], targetName: "Splitergoll")
        let duplicates = KeyboardConnections.rows(
            selection: selection, usb: nil, ble: nil, runtime: nil, at: now)
        XCTAssertEqual(Set(duplicates.map(\.name)).count, 2)
        XCTAssertEqual(Set(duplicates.map(\.keyboardID)), [v1.id, v2.id])
    }

    // L’expiration du statut supprime seulement sa prétention de saisie active, pas les liens observés frais.
    func testExpiredRuntimeDoesNotClaimActiveInput() throws {
        let rows = KeyboardConnections.rows(
            selection: selection(), usb: try observation(.usb), ble: try observation(.bluetooth),
            runtime: try runtime(.bluetooth, age: 15), at: now)
        XCTAssertEqual(rows.map(\.state), [.connected, .diagnostic, .connected])
        XCTAssertFalse(rows.contains(where: \.isSelected))
    }

    // Le mode réel expose USB désactivé pendant l’attente de preuve, puis une seule sortie sélectionnée.
    func testPriorityCapabilityRemovesTheDiagnosticLabel() throws {
        for (transport, states) in [
            (KeyboardRuntime.Transport.bluetooth, [KeyboardConnection.State.connected, .standby, .input]),
            (.usb, [.connected, .input, .standby]),
        ] {
            let rows = KeyboardConnections.rows(
                selection: selection(), usb: try observation(.usb), ble: try observation(.bluetooth),
                runtime: try runtime(transport, priority: true), at: now)
            XCTAssertEqual(rows.map(\.state), states)
            XCTAssertEqual(rows.filter(\.isSelected).count, 1)
            XCTAssertFalse(rows.contains { $0.state == .diagnostic })
        }
    }
}
