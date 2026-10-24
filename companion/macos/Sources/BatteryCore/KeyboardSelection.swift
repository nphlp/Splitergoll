// Sépare le choix du clavier physique de ses mesures et des destinations de saisie du firmware.
import Foundation

public struct KeyboardIdentity: Codable, Equatable, Sendable {
    public let id: UUID
    public let name: String

    // Le nom est un libellé. Seul l’identifiant local CoreBluetooth détermine l’identité.
    public init(id: UUID, name: String) {
        self.id = id
        self.name = name
    }
}

public struct KeyboardSelection: Sendable {
    private struct Preference: Codable {
        let version: Int
        let keyboard: KeyboardIdentity
    }

    public private(set) var preferred: KeyboardIdentity?
    public private(set) var available: [KeyboardIdentity] = []
    private var automaticID: UUID?

    // Une préférence absente ou illisible ne choisit jamais arbitrairement entre plusieurs claviers.
    public init(preferenceData: Data? = nil) {
        if let preferenceData,
            let value = try? JSONDecoder().decode(Preference.self, from: preferenceData),
            value.version == 1, !value.keyboard.name.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty
        {
            preferred = value.keyboard
        }
    }

    public var selected: KeyboardIdentity? {
        preferred ?? available.first(where: { $0.id == automaticID })
    }

    public var selectedIsAvailable: Bool {
        selected.map { choice in available.contains { $0.id == choice.id } } ?? false
    }

    public var displayName: String { selected?.name ?? "Splitergoll" }
    public var needsChoice: Bool { selected == nil && available.count > 1 }

    // Ne persiste ni les mesures, ni la liste éphémère des périphériques système.
    public var preferenceData: Data? {
        preferred.flatMap { try? JSONEncoder().encode(Preference(version: 1, keyboard: $0)) }
    }

    // Filtre les nouveaux noms et reconnaît aussi un clavier préféré renommé par son identifiant.
    public mutating func update(_ candidates: [KeyboardIdentity], targetName: String) {
        var seen: Set<UUID> = []
        available = candidates.map { candidate in
            if let preferred, candidate.id == preferred.id,
                candidate.name.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty
            {
                return preferred
            }
            return candidate
        }.filter { candidate in
            let recognized =
                candidate.id == preferred?.id
                || ConnectedKeyboardChoice.select(names: [candidate.name], targetName: targetName)
                    == .single(0)
            return recognized && seen.insert(candidate.id).inserted
        }.sorted {
            $0.name == $1.name ? $0.id.uuidString < $1.id.uuidString : $0.name < $1.name
        }
        if preferred == nil, !available.contains(where: { $0.id == automaticID }) {
            automaticID = available.count == 1 ? available.first?.id : nil
        }
        if let renamed = available.first(where: { $0.id == preferred?.id }),
            !renamed.name.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty
        {
            preferred = renamed
        }
    }

    // Seul un choix explicite d’un clavier disponible devient une préférence durable.
    @discardableResult public mutating func select(_ id: UUID) -> Bool {
        guard let candidate = available.first(where: { $0.id == id }) else { return false }
        preferred = candidate
        return true
    }

    // Distingue les noms identiques dans le menu sans utiliser le nom comme clé de sélection.
    public func label(for keyboard: KeyboardIdentity) -> String {
        guard available.contains(where: { $0.id != keyboard.id && $0.name == keyboard.name }) else {
            return keyboard.name
        }
        let prefix = String(keyboard.id.uuidString.prefix(8))
        let collision = available.contains {
            $0.id != keyboard.id && $0.name == keyboard.name && $0.id.uuidString.hasPrefix(prefix)
        }
        return "\(keyboard.name) (\(collision ? keyboard.id.uuidString : prefix))"
    }
}

// Chaque lecture appartient à une génération, même lorsqu’on retrouve le même périphérique après une coupure.
public struct BatterySessionGate: Sendable {
    public struct Token: Equatable, Sendable {
        fileprivate let generation: UUID
        fileprivate let peripheralID: UUID
    }

    private var current: Token?

    public init() {}

    // Commence une nouvelle génération distincte des callbacks déjà en attente.
    public mutating func begin(peripheralID: UUID) -> Token {
        let token = Token(generation: UUID(), peripheralID: peripheralID)
        current = token
        return token
    }

    // Invalide immédiatement les réponses à la fermeture, avant toute reconnexion cliente.
    public mutating func end() { current = nil }

    // Refuse une ancienne génération ou une réponse provenant d’un autre clavier.
    public func accepts(_ token: Token, peripheralID: UUID) -> Bool {
        current == token && token.peripheralID == peripheralID
    }
}
