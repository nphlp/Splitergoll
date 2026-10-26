import Foundation
import XCTest

@testable import BatteryCore

final class KeyboardRuntimeTests: XCTestCase {
    private let date = Date(timeIntervalSince1970: 1_000)

    // Le contexte synthétique n’est pas l’identité d’un clavier ou ordinateur réel.
    private func frame(os: UInt8 = 1, transport: UInt8 = 2, profile: UInt8 = 0) -> Data {
        var bytes = [UInt8](repeating: 0, count: 32)
        bytes.replaceSubrange(0..<8, with: [83, 71, 83, 84, 1, os, transport, profile])
        bytes[8] = 1
        return Data(bytes)
    }

    // L’OS est celui annoncé par le clavier et peut être Windows même dans un test exécuté sur Mac.
    func testOperatingSystemAndTransportAreExplicit() throws {
        for (value, title) in [(UInt8(0), "Inconnu"), (1, "macOS"), (2, "Windows")] {
            XCTAssertEqual(try KeyboardRuntime(frame(os: value), received: date).os.title, title)
        }
        for (value, title) in [(UInt8(0), "Aucune"), (1, "USB"), (2, "Bluetooth")] {
            XCTAssertEqual(
                try KeyboardRuntime(frame(transport: value), received: date).transport.title, title)
        }
        XCTAssertNil(try KeyboardRuntime(frame(profile: 255), received: date).profile)
        XCTAssertEqual(try KeyboardRuntime(frame(profile: 7), received: date).profile, 7)
    }

    // Une valeur inconnue, un champ réservé ou un protocole tronqué n’est jamais deviné.
    func testMalformedStatusesAreRejected() {
        for size in 0..<32 {
            XCTAssertThrowsError(try KeyboardRuntime(frame().prefix(size), received: date))
        }
        XCTAssertThrowsError(try KeyboardRuntime(frame() + Data([0]), received: date))
        for index in Array(0...4) + Array(24..<32) {
            var data = frame()
            data[index] ^= 0x80
            XCTAssertThrowsError(try KeyboardRuntime(data, received: date))
        }
        for data in [frame(os: 3), frame(transport: 3), frame(profile: 8)] {
            XCTAssertThrowsError(try KeyboardRuntime(data, received: date))
        }
        var emptyBoot = frame()
        emptyBoot[8] = 0
        XCTAssertThrowsError(try KeyboardRuntime(emptyBoot, received: date))
    }

    // La limite de quinze secondes et le jeton de démarrage excluent les statuts anciens.
    func testFreshnessAndControllerContextAreMandatory() throws {
        let data = frame()
        let report = try KeyboardRuntime(data, received: date)
        let boot = data.subdata(in: 8..<24)
        XCTAssertTrue(report.isCurrent(boot: boot, at: date))
        XCTAssertTrue(report.isCurrent(boot: boot, at: date.addingTimeInterval(14.999)))
        XCTAssertFalse(report.isCurrent(boot: boot, at: date.addingTimeInterval(15)))
        XCTAssertFalse(report.isCurrent(boot: boot, at: date.addingTimeInterval(-1)))
        XCTAssertFalse(report.isCurrent(boot: nil, at: date))
        XCTAssertFalse(report.isCurrent(boot: Data(repeating: 2, count: 16), at: date))
    }

    // Le nouveau statut annonce sa capacité, sans attribuer la priorité aux anciens firmwares.
    func testVersionTwoPriorityCapabilityIsExplicitAndStrict() throws {
        XCTAssertFalse(try KeyboardRuntime(frame(), received: date).usbPriority)
        var data = frame()
        data[4] = 2
        data[24] = 1
        XCTAssertTrue(try KeyboardRuntime(data, received: date).usbPriority)
        data[24] = 2
        XCTAssertThrowsError(try KeyboardRuntime(data, received: date))
        data[24] = 1
        data[4] = 1
        XCTAssertThrowsError(try KeyboardRuntime(data, received: date))
        data[4] = 3
        XCTAssertThrowsError(try KeyboardRuntime(data, received: date))
    }
}
