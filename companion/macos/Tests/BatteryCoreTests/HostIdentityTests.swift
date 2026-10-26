import Foundation
import XCTest

@testable import BatteryCore

final class HostIdentityTests: XCTestCase {
    // Réponse synthétique publique, sans capture matérielle ou identifiant d’ordinateur réel.
    private func frame(
        transport: UInt8 = 0, status: UInt8 = 0, matched: UInt8 = 0, claimed: UInt8 = 0, usb: UInt8 = 1
    ) -> Data {
        var bytes = [UInt8](repeating: 0, count: 64)
        bytes.replaceSubrange(0..<4, with: [83, 71, 73, 68])
        bytes[4] = 1
        bytes[5] = 1
        bytes[6] = transport
        bytes[7] = status
        bytes[8] = 1
        bytes[24] = transport + 1
        bytes[56] = matched
        bytes[57] = claimed
        bytes[58] = usb
        bytes[59] = status == 0 ? 0 : usb
        bytes[60] = transport == 0 ? 255 : 0
        return Data(bytes)
    }

    // Les deux transports acceptent le même format mais pas le tag de l’autre canal.
    func testTransportAndLengthAreStrict() throws {
        let data = frame()
        XCTAssertEqual(try HostIdentityProtocol.Report(data, transport: .usb).transport, .usb)
        XCTAssertThrowsError(try HostIdentityProtocol.Report(data, transport: .bluetooth))
        for length in 0..<64 {
            XCTAssertThrowsError(try HostIdentityProtocol.Report(data.prefix(length), transport: .usb))
        }
        XCTAssertThrowsError(try HostIdentityProtocol.Report(data + Data([0]), transport: .usb))
    }

    // Les champs réservés et la confidentialité ne sont pas assouplis en cas de trame malformée.
    func testMalformedHeadersAndPrivateDataAreRefused() {
        for index in Array(0...5) + Array(40..<56) + Array(61..<64) {
            var data = frame()
            data[index] ^= 0x80
            XCTAssertThrowsError(try HostIdentityProtocol.Report(data, transport: .usb))
        }
        for range in [8..<24, 24..<40] {
            var data = frame()
            data.replaceSubrange(range, with: repeatElement(UInt8(0), count: 16))
            XCTAssertThrowsError(try HostIdentityProtocol.Report(data, transport: .usb))
        }
    }

    // Des masques contradictoires ne doivent jamais produire une correspondance verte.
    func testImpossibleStatusesAreRefused() {
        for data in [
            frame(status: 1), frame(status: 1, matched: 3, claimed: 3),
            frame(status: 2), frame(status: 3, claimed: 1), frame(status: 1, matched: 1, claimed: 2),
            frame(status: 1, matched: 1, claimed: 1, usb: 0),
        ] {
            XCTAssertThrowsError(try HostIdentityProtocol.Report(data, transport: .usb))
        }
    }

    // L’annonce conserve le contexte mais efface les champs de statut et réserve sa seule charge utile.
    func testAnnouncementEchoesContextAndClearsStatus() throws {
        let report = try HostIdentityProtocol.Report(
            frame(status: 1, matched: 1, claimed: 1), transport: .usb)
        let id = UUID(uuidString: "01020304-0506-0708-090A-0B0C0D0E0F10")!
        let data = try report.announcement(identity: id)
        XCTAssertEqual(data.count, 64)
        XCTAssertEqual(data[5], 2)
        XCTAssertEqual(data[7], 0)
        XCTAssertEqual(data.subdata(in: 8..<40), report.bytes.subdata(in: 8..<40))
        XCTAssertEqual(Array(data[40..<56]), Array(1...16).map(UInt8.init))
        XCTAssertTrue(data[56..<64].allSatisfy { $0 == 0 })
        XCTAssertThrowsError(
            try report.announcement(identity: UUID(uuidString: "00000000-0000-0000-0000-000000000000")!))
    }

    // Le slot de la session BLE, et non le premier bit annoncé, doit correspondre à l’USB.
    func testVerdictUsesSameControllerAndActualSlot() throws {
        let now = Date(timeIntervalSince1970: 100)
        let usb = try HostIdentityProtocol.Report(frame(status: 1, matched: 1, claimed: 1), transport: .usb)
        let ble = try HostIdentityProtocol.Report(
            frame(transport: 1, status: 1, matched: 1, claimed: 1), transport: .bluetooth)
        let u = HostIdentityProtocol.Observation(report: usb, received: now)
        let b = HostIdentityProtocol.Observation(report: ble, received: now)
        XCTAssertEqual(HostIdentityProtocol.verdict(usb: u, ble: b, now: now), "Même ordinateur déclaré")
        var other = ble.bytes
        other[8] = 2
        let otherReport = try HostIdentityProtocol.Report(other, transport: .bluetooth)
        XCTAssertEqual(
            HostIdentityProtocol.verdict(usb: u, ble: .init(report: otherReport, received: now), now: now),
            "Claviers différents")
        other = ble.bytes
        other[60] = 1
        let otherSlot = try HostIdentityProtocol.Report(other, transport: .bluetooth)
        XCTAssertNotEqual(
            HostIdentityProtocol.verdict(usb: u, ble: .init(report: otherSlot, received: now), now: now),
            "Même ordinateur déclaré")
        XCTAssertEqual(HostIdentityProtocol.verdict(usb: nil, ble: b, now: now), "Correspondance non établie")
    }

    // Une observation ancienne ou située dans le futur doit être invalidée.
    func testLeaseAndClockBoundaries() throws {
        let now = Date(timeIntervalSince1970: 100)
        let u = try HostIdentityProtocol.Report(frame(status: 1, matched: 1, claimed: 1), transport: .usb)
        let b = try HostIdentityProtocol.Report(
            frame(transport: 1, status: 1, matched: 1, claimed: 1), transport: .bluetooth)
        let usb = HostIdentityProtocol.Observation(report: u, received: now)
        let ble = HostIdentityProtocol.Observation(report: b, received: now)
        XCTAssertEqual(
            HostIdentityProtocol.verdict(usb: usb, ble: ble, now: now.addingTimeInterval(15)),
            "Correspondance expirée")
        XCTAssertEqual(
            HostIdentityProtocol.verdict(usb: usb, ble: ble, now: now.addingTimeInterval(-0.1)),
            "Correspondance expirée")
        XCTAssertEqual(
            HostIdentityProtocol.verdict(usb: usb, ble: ble, now: now.addingTimeInterval(14.9)),
            "Même ordinateur déclaré")
    }

    // Une identité déclarée sur deux pairs empêche une association, même avec un USB valide.
    func testClonedIdentityConflict() throws {
        let now = Date()
        let u = try HostIdentityProtocol.Report(frame(status: 3, claimed: 3), transport: .usb)
        let b = try HostIdentityProtocol.Report(
            frame(transport: 1, status: 1, matched: 1, claimed: 1), transport: .bluetooth)
        XCTAssertEqual(
            HostIdentityProtocol.verdict(
                usb: .init(report: u, received: now), ble: .init(report: b, received: now), now: now),
            "Identités en conflit")
    }

    // Chaque test de stockage reste dans son propre dossier temporaire.
    private func temporaryFile() throws -> URL {
        let directory = FileManager.default.temporaryDirectory.appendingPathComponent(
            "splitergoll-identity-tests-" + UUID().uuidString)
        addTeardownBlock { try? FileManager.default.removeItem(at: directory) }
        return directory.appendingPathComponent("host-identity.json")
    }

    // Le remplacement de l’app réutilise le même fichier, sans générer ni réécrire une identité.
    func testFilePersistsAndExplainsItsPurpose() throws {
        let file = try temporaryFile()
        let expected = UUID()
        var generated = 0
        let first = try HostIdentityFile.loadOrCreate(at: file) {
            generated += 1
            return expected
        }
        let before = try Data(contentsOf: file)
        let second = try HostIdentityFile.loadOrCreate(at: file) {
            generated += 1
            return UUID()
        }
        XCTAssertEqual(first, second)
        XCTAssertEqual(second, expected)
        XCTAssertEqual(generated, 1)
        XCTAssertEqual(try Data(contentsOf: file), before)
        XCTAssertTrue(String(decoding: before, as: UTF8.self).contains("pas un mot de passe"))
        let attrs = try FileManager.default.attributesOfItem(atPath: file.path)
        XCTAssertEqual((attrs[.posixPermissions] as? NSNumber)?.intValue, 0o600)
    }

    // Une corruption ou version inconnue est signalée sans régénération ni écrasement.
    func testInvalidFileIsNotReplaced() throws {
        let file = try temporaryFile()
        _ = try HostIdentityFile.loadOrCreate(at: file)
        for invalid in [
            Data("broken".utf8), Data(repeating: 65, count: 4097),
            Data(
                "{\"version\":2,\"purpose\":\"test\",\"identity\":\"01020304-0506-0708-090A-0B0C0D0E0F10\"}"
                    .utf8),
        ] {
            try invalid.write(to: file)
            XCTAssertThrowsError(try HostIdentityFile.loadOrCreate(at: file))
            XCTAssertEqual(try Data(contentsOf: file), invalid)
        }
    }

    // Un fichier d’identité symbolique et un générateur vide sont refusés.
    func testSymlinkAndZeroIdentityAreRefused() throws {
        let file = try temporaryFile()
        XCTAssertThrowsError(
            try HostIdentityFile.loadOrCreate(at: file) {
                UUID(uuidString: "00000000-0000-0000-0000-000000000000")!
            })
        _ = try HostIdentityFile.loadOrCreate(at: file)
        let symbolic = file.deletingLastPathComponent().appendingPathComponent("link.json")
        try FileManager.default.createSymbolicLink(at: symbolic, withDestinationURL: file)
        XCTAssertThrowsError(try HostIdentityFile.loadOrCreate(at: symbolic))
        let dangling = file.deletingLastPathComponent().appendingPathComponent("dangling.json")
        try FileManager.default.createSymbolicLink(
            at: dangling, withDestinationURL: file.appendingPathExtension("absent"))
        XCTAssertThrowsError(try HostIdentityFile.loadOrCreate(at: dangling))
    }
}
