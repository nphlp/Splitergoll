// Isole la préférence locale pour tester le lecteur sans écrire dans les réglages du Mac.
import Foundation

@MainActor
protocol KeyboardPreferenceStorage: AnyObject {
    var data: Data? { get set }
}

@MainActor
final class KeyboardPreferences: KeyboardPreferenceStorage {
    private let defaults: UserDefaults
    private static let key = "followedKeyboard.v1"

    // Le stockage réel n’est utilisé qu’à l’ouverture de l’application, jamais par les tests simulés.
    init(defaults: UserDefaults = .standard) { self.defaults = defaults }

    var data: Data? {
        get { defaults.data(forKey: Self.key) }
        set { defaults.set(newValue, forKey: Self.key) }
    }
}
