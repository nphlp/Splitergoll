// Inscrit le bundle signé comme application d’ouverture de session via l’API native de macOS.
import AppKit
import BatteryCore
import ServiceManagement

@MainActor
enum LaunchAtLogin {
    private static let registrationKey = "launchAtLoginRegistered"

    // Une inscription réussie n’est pas répétée, pour respecter une désactivation ultérieure par l’utilisateur.
    static func configure() {
        let defaults = UserDefaults.standard
        let previousRegistration = defaults.bool(forKey: registrationKey)
        let service = SMAppService.mainApp
        let outcome = LoginRegistration.configure(
            previouslyRegistered: previousRegistration,
            status: { status(of: service) },
            register: { try service.register() }
        )
        if outcome.registrationCompleted { defaults.set(true, forKey: registrationKey) }

        switch outcome {
        case .enabled, .disabled: break
        case .requiresApproval:
            if !previousRegistration {
                showNotice("Autorise Splitergoll dans les réglages d’ouverture de session de macOS.")
            }
        case .failed(let message):
            showNotice("Le lancement automatique n’a pas pu être activé.\n\(message)")
        }
    }

    // Transpose uniquement les états publics du service, sans changer les préférences système.
    private static func status(of service: SMAppService) -> LoginRegistration.Status {
        switch service.status {
        case .notRegistered: return .notRegistered
        case .enabled: return .enabled
        case .requiresApproval: return .requiresApproval
        case .notFound: return .notFound
        @unknown default: return .notFound
        }
    }

    // Signale une autorisation ou une erreur sans ajouter de réglage permanent à la petite fenêtre.
    private static func showNotice(_ message: String) {
        let alert = NSAlert()
        alert.messageText = "Lancement automatique"
        alert.informativeText = message
        alert.addButton(withTitle: "Ouvrir les réglages")
        alert.addButton(withTitle: "Plus tard")
        NSApp.activate(ignoringOtherApps: true)
        if alert.runModal() == .alertFirstButtonReturn { SMAppService.openSystemSettingsLoginItems() }
    }
}
