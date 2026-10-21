import XCTest

@testable import BatteryCore

final class LoginRegistrationTests: XCTestCase {
    // L’inscription initiale vérifie le résultat réel avant de mémoriser sa réussite.
    @MainActor func testFirstRegistrationSucceeds() {
        var status = LoginRegistration.Status.notRegistered
        var attempts = 0
        let outcome = LoginRegistration.configure(
            previouslyRegistered: false, status: { status },
            register: {
                attempts += 1
                status = .enabled
            })
        XCTAssertEqual(attempts, 1)
        XCTAssertEqual(outcome, .enabled)
        XCTAssertTrue(outcome.registrationCompleted)
    }

    // Une désactivation après inscription n’est jamais annulée automatiquement.
    @MainActor func testUserRemovalIsRespected() {
        let outcome = LoginRegistration.configure(
            previouslyRegistered: true, status: { .notRegistered },
            register: { XCTFail("Ne pas réinscrire une app désactivée") })
        XCTAssertEqual(outcome, .disabled)
        XCTAssertFalse(outcome.registrationCompleted)
    }

    // Les services déjà inscrits ou en attente d’autorisation ne sont pas inscrits une seconde fois.
    @MainActor func testExistingRegistrationDoesNotRepeatRequest() {
        for previouslyRegistered in [false, true] {
            for status in [LoginRegistration.Status.enabled, .requiresApproval] {
                let outcome = LoginRegistration.configure(
                    previouslyRegistered: previouslyRegistered, status: { status },
                    register: { XCTFail("Inscription déjà présente") })
                XCTAssertEqual(outcome, status == .enabled ? .enabled : .requiresApproval)
                XCTAssertTrue(outcome.registrationCompleted)
            }
        }
    }

    // Une erreur sans inscription reste un échec réessayable au prochain lancement.
    @MainActor func testRegistrationFailureDoesNotClaimSuccess() {
        let outcome = LoginRegistration.configure(
            previouslyRegistered: false, status: { .notRegistered },
            register: { throw TestError.denied })
        guard case .failed = outcome else { return XCTFail("Une erreur doit être signalée") }
        XCTAssertFalse(outcome.registrationCompleted)
    }

    // Une demande d’approbation créée malgré une erreur reste distincte d’une activation effective.
    @MainActor func testApprovalAfterRegistrationError() {
        var status = LoginRegistration.Status.notRegistered
        let outcome = LoginRegistration.configure(
            previouslyRegistered: false, status: { status },
            register: {
                status = .requiresApproval
                throw TestError.denied
            })
        XCTAssertEqual(outcome, .requiresApproval)
        XCTAssertTrue(outcome.registrationCompleted)
    }

    // Un retour sans erreur ne suffit pas si le système n’a pas enregistré le service.
    @MainActor func testMissingRegistrationAfterSuccessfulCallIsReported() {
        let outcome = LoginRegistration.configure(
            previouslyRegistered: false, status: { .notRegistered }, register: {})
        guard case .failed = outcome else { return XCTFail("Inscription absente") }
        XCTAssertFalse(outcome.registrationCompleted)
    }

    // Un bundle introuvable ne doit ni lancer une inscription ni être présenté comme actif.
    @MainActor func testMissingBundleIsReported() {
        let outcome = LoginRegistration.configure(
            previouslyRegistered: false, status: { .notFound },
            register: { XCTFail("Bundle introuvable") })
        guard case .failed = outcome else { return XCTFail("Bundle introuvable") }
        XCTAssertFalse(outcome.registrationCompleted)
    }

    private enum TestError: Error { case denied }
}
