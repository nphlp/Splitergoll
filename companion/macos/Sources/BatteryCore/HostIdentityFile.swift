// Stocke un identifiant d’installation utilisateur, jamais une clé de sécurité ou une adresse matérielle.
import Foundation

public struct HostIdentityFile {
    private struct Document: Codable {
        let version: Int
        let purpose: String
        let identity: UUID
    }
    public enum Failure: Error { case invalidFile }

    // Un fichier corrompu reste intact. Une nouvelle identité n’est pas créée silencieusement.
    public static func loadOrCreate(at url: URL, generate: () -> UUID = UUID.init) throws -> UUID {
        let existing: [FileAttributeKey: Any]?
        do { existing = try FileManager.default.attributesOfItem(atPath: url.path) } catch let error
            as CocoaError where error.code == .fileReadNoSuchFile
        { existing = nil }
        if let attributes = existing {
            guard attributes[.type] as? FileAttributeType == .typeRegular,
                let length = attributes[.size] as? NSNumber, length.intValue <= 4096
            else { throw Failure.invalidFile }
            let data = try Data(contentsOf: url)
            guard data.count <= 4096, let document = try? JSONDecoder().decode(Document.self, from: data),
                document.version == 1, document.identity.uuidString != "00000000-0000-0000-0000-000000000000"
            else { throw Failure.invalidFile }
            return document.identity
        }
        let identity = generate()
        guard identity.uuidString != "00000000-0000-0000-0000-000000000000" else { throw Failure.invalidFile }
        let document = Document(
            version: 1,
            purpose:
                "Créé par l’application Splitergoll pour reconnaître cette installation utilisateur en USB et Bluetooth. Identifiant local, pas un mot de passe ni une clé d’appairage.",
            identity: identity)
        let directory = url.deletingLastPathComponent()
        try FileManager.default.createDirectory(
            at: directory, withIntermediateDirectories: true, attributes: [.posixPermissions: 0o700])
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.prettyPrinted, .sortedKeys]
        let data = try encoder.encode(document)
        // withoutOverwriting empêche deux lancements simultanés de remplacer leur identité respective.
        do { try data.write(to: url, options: [.withoutOverwriting]) } catch let error as CocoaError
            where error.code == .fileWriteFileExists
        {
            return try loadOrCreate(at: url, generate: generate)
        }
        try FileManager.default.setAttributes([.posixPermissions: 0o600], ofItemAtPath: url.path)
        return identity
    }
}
