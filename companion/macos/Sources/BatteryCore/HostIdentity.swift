// Format commun au canal USB vendor et au service BLE, sans identifiant matériel.
import Foundation

public enum HostIdentityProtocol {
    public static let service = "7D2F6767-4C2B-4D13-A528-80B87AC03810"
    public static let characteristic = "7D2F6767-4C2B-4D13-A528-80B87AC03811"
    public static let usagePage = 0xFF31
    public static let usage = 0x5350
    public static let size = 64
    public static let lease: TimeInterval = 15

    public enum Failure: Error { case malformed, wrongTransport, invalidIdentity }
    public enum Transport: UInt8, Sendable {
        case usb = 0
        case bluetooth = 1
    }
    public enum Status: UInt8, Sendable {
        case pending = 0
        case same = 1
        case different = 2
        case conflict = 3
    }

    public struct Report: Sendable, Equatable {
        public let bytes: Data
        public let transport: Transport
        public let status: Status
        public var boot: Data { bytes.subdata(in: 8..<24) }
        public var nonce: Data { bytes.subdata(in: 24..<40) }
        public var matched: UInt8 { bytes[56] }
        public var claimed: UInt8 { bytes[57] }
        public var usbReady: Bool { bytes[58] == 1 }
        public var usbClaimed: Bool { bytes[59] == 1 }
        public var slot: UInt8 { bytes[60] }

        // Valide aussi les états contradictoires, pas seulement la longueur de la réponse.
        public init(_ data: Data, transport expected: Transport) throws {
            let b = [UInt8](data)
            guard b.count == size, Array(b[0..<4]) == [83, 71, 73, 68], b[4] == 1, b[5] == 1,
                let transport = Transport(rawValue: b[6]), let status = Status(rawValue: b[7]),
                b[8..<24].contains(where: { $0 != 0 }), b[24..<40].contains(where: { $0 != 0 }),
                b[40..<56].allSatisfy({ $0 == 0 }), b[61..<64].allSatisfy({ $0 == 0 }),
                b[58] <= 1, b[59] <= b[58], b[56] & ~b[57] == 0,
                transport == .usb ? b[60] == 255 : b[60] < 8
            else { throw Failure.malformed }
            guard transport == expected else { throw Failure.wrongTransport }
            switch status {
            case .same:
                guard b[59] == 1, b[56].nonzeroBitCount == 1 else { throw Failure.malformed }
            case .different:
                guard b[59] == 1, b[57] != 0, b[56] == 0 else { throw Failure.malformed }
            case .conflict:
                guard b[56] == 0, b[57].nonzeroBitCount >= 2 else { throw Failure.malformed }
            case .pending:
                guard b[56] == 0, b[59] == 0 || b[57] == 0 else { throw Failure.malformed }
            }
            self.bytes = Data(b)
            self.transport = transport
            self.status = status
        }

        // La déclaration écho du contexte ne peut être rejouée dans un autre démarrage ou lien.
        public func announcement(identity: UUID) throws -> Data {
            var uuid = identity.uuid
            let id = withUnsafeBytes(of: &uuid) { Array($0) }
            guard id.contains(where: { $0 != 0 }) else { throw Failure.invalidIdentity }
            var b = [UInt8](bytes)
            b[5] = 2
            b[7] = 0
            b.replaceSubrange(40..<56, with: id)
            b.replaceSubrange(56..<64, with: repeatElement(UInt8(0), count: 8))
            return Data(b)
        }
    }

    public struct Observation: Sendable {
        public let report: Report
        public let received: Date
        public init(report: Report, received: Date) {
            self.report = report
            self.received = received
        }
    }

    // Les deux canaux doivent décrire ce même contrôleur et rester frais, sans ambiguïté de slot.
    public static func verdict(usb: Observation?, ble: Observation?, now: Date) -> String {
        guard let usb, let ble else { return "Correspondance non établie" }
        for entry in [usb, ble] {
            let age = now.timeIntervalSince(entry.received)
            guard age >= 0, age < lease else { return "Correspondance expirée" }
        }
        guard usb.report.transport == .usb, ble.report.transport == .bluetooth,
            usb.report.boot == ble.report.boot
        else { return "Claviers différents" }
        if usb.report.status == .conflict || ble.report.status == .conflict { return "Identités en conflit" }
        let bit = UInt8(1) << ble.report.slot
        guard usb.report.usbReady, usb.report.usbClaimed, ble.report.claimed & bit != 0 else {
            return "Correspondance non établie"
        }
        return usb.report.status == .same && usb.report.matched & bit != 0
            ? "Même ordinateur déclaré" : "Ordinateurs différents ou annonce en attente"
    }
}
