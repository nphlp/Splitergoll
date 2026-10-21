// Identifie les deux batteries et conserve des lectures distinctes, sans dépendre de CoreBluetooth.
import Foundation

public enum BatterySide: String, CaseIterable, Sendable {
    case left
    case right

    public var name: String { self == .left ? "Gauche" : "Droite" }
    public var shortName: String { self == .left ? "G" : "D" }
}

public enum BatteryDataError: Error, Equatable {
    case invalidPresentation
    case unknownSource
    case duplicateSide
    case invalidLevel
}

public enum BatteryProtocol {
    // Décode la présentation GATT 2904, avec les mêmes identifiants que le lecteur make bat.
    public static func side(from data: Data) throws -> BatterySide {
        let bytes = Array(data)
        guard bytes.count == 7, Array(bytes.prefix(5)) == [0x04, 0, 0xAD, 0x27, 1] else {
            throw BatteryDataError.invalidPresentation
        }

        let source = UInt16(bytes[5]) | UInt16(bytes[6]) << 8
        switch source {
        case 0x0106: return .left
        case 0x0108: return .right
        default: throw BatteryDataError.unknownSource
        }
    }

    // Refuse toute valeur absente, multioctet ou extérieure au pourcentage GATT.
    public static func level(from data: Data) throws -> Int {
        guard data.count == 1, let level = data.first, level <= 100 else {
            throw BatteryDataError.invalidLevel
        }
        return Int(level)
    }
}

public struct BatteryReading: Equatable, Sendable {
    public let level: Int
    // Réception par le Mac, pas date d’échantillonnage ADC du firmware.
    public let receivedAt: Date
}

public enum BatteryIndication: Equatable, Sendable {
    case unavailable
    case level(Int)
    // Le firmware n’expose pas la charge : cent pour cent est une indication ambiguë.
    case possibleCharge

    public var menuValue: String {
        switch self {
        case .unavailable: return "-"
        case .level(let level): return "\(level) %"
        case .possibleCharge: return "Charge possible"
        }
    }
}

public enum BatteryPolling {
    public static let interval: TimeInterval = 5
}

public enum MonitorPhase: Equatable, Sendable {
    case waiting
    case loading
    case connected
    case disconnected
    case bluetoothOff
    case permissionDenied
    case unavailable
    case ambiguous
    case sleeping
    case failed(String)
}

public struct BatteryState: Sendable {
    public static let staleAfter: TimeInterval = 90

    public var phase: MonitorPhase = .waiting
    public private(set) var readings: [BatterySide: BatteryReading] = [:]
    private var channels: [Int: BatterySide] = [:]
    private var pendingReadings: [Int: BatteryReading] = [:]
    private var currentSides: Set<BatterySide> = []

    public init() {}

    // Vide uniquement la session GATT. Les anciennes lectures restent en mémoire, sans affichage.
    public mutating func beginSession() {
        channels.removeAll()
        pendingReadings.removeAll()
        currentSides.removeAll()
        phase = .loading
    }

    // Identifie un canal sans présumer l’ordre des deux services. Accepte une lecture arrivée avant son descripteur.
    public mutating func identify(channel: Int, presentation: Data) throws {
        let side = try BatteryProtocol.side(from: presentation)
        guard !channels.contains(where: { $0.key != channel && $0.value == side }),
            channels[channel] == nil || channels[channel] == side
        else {
            throw BatteryDataError.duplicateSide
        }

        channels[channel] = side
        if let reading = pendingReadings.removeValue(forKey: channel) {
            readings[side] = reading
            currentSides.insert(side)
        }
    }

    // Conserve la dernière lecture par moitié. Une valeur invalide ne remplace jamais une valeur valide.
    public mutating func receive(channel: Int, data: Data, at date: Date) throws {
        let reading = BatteryReading(level: try BatteryProtocol.level(from: data), receivedAt: date)
        if let side = channels[channel] {
            readings[side] = reading
            currentSides.insert(side)
        } else {
            pendingReadings[channel] = reading
        }
    }

    public var hasCurrentReading: Bool { !currentSides.isEmpty }

    public var connectionTitle: String { phase == .connected ? "Connecté" : "Déconnecté" }

    // Retire les anciennes lectures de l’indicateur, sans effacer l’historique de la fenêtre.
    public func isCurrent(_ side: BatterySide, at now: Date) -> Bool {
        guard phase == .connected, currentSides.contains(side), let reading = readings[side] else {
            return false
        }
        let age = now.timeIntervalSince(reading.receivedAt)
        return age >= 0 && age < Self.staleAfter
    }

    // Distingue niveau lisible et charge possible, sans transformer cent pour cent en charge certaine.
    public func indication(for side: BatterySide, at now: Date) -> BatteryIndication {
        guard isCurrent(side, at: now), let reading = readings[side] else { return .unavailable }
        return reading.level == 100 ? .possibleCharge : .level(reading.level)
    }

    // Les lignes gardent gauche au-dessus de droite, indépendamment de l’ordre des réponses GATT.
    public func menuLines(at now: Date) -> [String] {
        BatterySide.allCases.map { side in
            "\(side.shortName) \(indication(for: side, at: now).menuValue)"
        }
    }

    // Fournit aussi le texte complet aux outils d’accessibilité et aux infobulles.
    public func menuTitle(at now: Date) -> String {
        menuLines(at: now).joined(separator: "\n")
    }
}

public enum ConnectedKeyboardChoice: Equatable {
    case none
    case single(Int)
    case ambiguous

    // Refuse de choisir arbitrairement entre V1 et V2 s’ils annoncent le même nom.
    public static func select(names: [String?], targetName: String) -> Self {
        let matches = names.indices.filter { names[$0] == targetName }
        if matches.count > 1 { return .ambiguous }
        return matches.first.map(Self.single) ?? .none
    }
}
