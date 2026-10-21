// Prépare l’ouverture de session sans réinscrire une app désactivée dans les réglages système.
public enum LoginRegistration {
    public enum Status: Equatable {
        case notRegistered
        case enabled
        case requiresApproval
        case notFound
    }

    public enum Outcome: Equatable {
        case enabled
        case requiresApproval
        case disabled
        case failed(String)

        public var registrationCompleted: Bool {
            self == .enabled || self == .requiresApproval
        }
    }

    // Les callbacks isolent ServiceManagement et permettent de tester les erreurs sans modifier macOS.
    @MainActor public static func configure(
        previouslyRegistered: Bool,
        status: () -> Status,
        register: () throws -> Void
    ) -> Outcome {
        if status() == .notRegistered {
            guard !previouslyRegistered else { return .disabled }
            do {
                try register()
            } catch {
                // Le système peut demander une approbation même si register a levé une erreur.
                if status() == .requiresApproval { return .requiresApproval }
                return .failed(String(describing: error))
            }
        }

        switch status() {
        case .enabled: return .enabled
        case .requiresApproval: return .requiresApproval
        case .notRegistered: return .failed("L’inscription à l’ouverture de session n’a pas abouti.")
        case .notFound: return .failed("macOS ne trouve pas le bundle de l’application.")
        }
    }
}
