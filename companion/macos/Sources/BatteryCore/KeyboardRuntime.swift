// Statut de saisie lu dans le clavier, indépendant de l’OS qui exécute l’app.
import Foundation

public struct KeyboardRuntime: Equatable, Sendable {
    public static let characteristic = "7D2F6767-4C2B-4D13-A528-80B87AC03812"
    public enum OperatingSystem: UInt8, Sendable {
        case unknown = 0
        case macos = 1
        case windows = 2

        public var title: String {
            switch self {
            case .unknown: return "Inconnu"
            case .macos: return "macOS"
            case .windows: return "Windows"
            }
        }
    }
    public enum Transport: UInt8, Sendable {
        case none = 0
        case usb = 1
        case bluetooth = 2

        public var title: String {
            switch self {
            case .none: return "Aucune"
            case .usb: return "USB"
            case .bluetooth: return "Bluetooth"
            }
        }
    }

    public let os: OperatingSystem
    public let transport: Transport
    public let profile: UInt8?
    public let usbPriority: Bool
    private let boot: Data
    private let received: Date

    // Valide le format et les champs réservés sans accepter d’identité ou de frappe.
    public init(_ data: Data, received: Date) throws {
        let b = [UInt8](data)
        guard b.count == 32, Array(b[0..<4]) == [83, 71, 83, 84], b[4] == 1 || b[4] == 2,
            let os = OperatingSystem(rawValue: b[5]), let transport = Transport(rawValue: b[6]),
            b[7] < 8 || b[7] == 255, b[8..<24].contains(where: { $0 != 0 }),
            b[24] <= 1, b[4] == 2 || b[24] == 0, b[25..<32].allSatisfy({ $0 == 0 })
        else { throw HostIdentityProtocol.Failure.malformed }
        self.os = os
        self.transport = transport
        profile = b[7] == 255 ? nil : b[7]
        usbPriority = b[24] == 1
        boot = Data(b[8..<24])
        self.received = received
    }

    // Un ancien statut disparaît après une coupure, un redémarrage ou quinze secondes sans réponse.
    public func isCurrent(boot: Data?, at now: Date) -> Bool {
        let age = now.timeIntervalSince(received)
        return self.boot == boot && age >= 0 && age < HostIdentityProtocol.lease
    }
}
