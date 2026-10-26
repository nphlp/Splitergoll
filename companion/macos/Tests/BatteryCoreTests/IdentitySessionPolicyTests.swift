import Foundation
import XCTest

@testable import BatteryCore

final class IdentitySessionPolicyTests: XCTestCase {
    private let date = Date(timeIntervalSince1970: 1_000)
    private let id = UUID()

    // Aucune fenêtre n’est nécessaire, mais le clavier doit être choisi et réellement disponible.
    func testBackgroundStartRequiresAnAvailableSelectionAndNoExistingReader() {
        let policy = IdentitySessionPolicy()
        XCTAssertTrue(policy.permitsStart(selected: id, available: true, running: false, at: date))
        XCTAssertFalse(policy.permitsStart(selected: nil, available: true, running: false, at: date))
        XCTAssertFalse(policy.permitsStart(selected: id, available: false, running: false, at: date))
        XCTAssertFalse(policy.permitsStart(selected: id, available: true, running: true, at: date))
    }

    // Les erreurs attendent trente secondes, pas une boucle de tentatives au battement de cinq secondes.
    func testFailureCooldownHasAnExactBoundary() {
        var policy = IdentitySessionPolicy()
        policy.stopped(at: date)
        XCTAssertFalse(
            policy.permitsStart(
                selected: id, available: true, running: false, at: date.addingTimeInterval(29.999)))
        XCTAssertTrue(
            policy.permitsStart(
                selected: id, available: true, running: false, at: date.addingTimeInterval(30)))
    }

    // Le sommeil est prioritaire même sur une relance, et le réveil autorise la reconnexion déjà disponible.
    func testSleepStopsBackgroundWorkUntilWake() {
        var policy = IdentitySessionPolicy()
        policy.sleep()
        policy.stopped(at: date)
        policy.resume()
        XCTAssertFalse(policy.permitsStart(selected: id, available: true, running: false, at: date))
        policy.wake()
        XCTAssertTrue(policy.permitsStart(selected: id, available: true, running: false, at: date))
    }

    // Arrêter reste respecté après sommeil et après expiration du délai de reprise.
    func testExplicitPauseSurvivesWakeAndRequiresResume() {
        var policy = IdentitySessionPolicy()
        policy.pause()
        policy.sleep()
        policy.wake()
        XCTAssertFalse(
            policy.permitsStart(
                selected: id, available: true, running: false, at: date.addingTimeInterval(300)))
        policy.resume()
        XCTAssertTrue(policy.permitsStart(selected: id, available: true, running: false, at: date))
    }
}
