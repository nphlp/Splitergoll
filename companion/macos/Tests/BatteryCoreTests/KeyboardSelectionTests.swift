import Foundation
import XCTest

@testable import BatteryCore

final class KeyboardSelectionTests: XCTestCase {
    private let first = KeyboardIdentity(
        id: UUID(uuidString: "11111111-0000-0000-0000-000000000001")!, name: "Splitergoll V1")
    private let second = KeyboardIdentity(
        id: UUID(uuidString: "22222222-0000-0000-0000-000000000002")!, name: "Splitergoll V2")

    // Aucun candidat laisse le lecteur sans cible et un seul candidat reste utilisable sans préférence.
    func testAutomaticSelectionRequiresExactlyOneCompatibleDevice() {
        var selection = KeyboardSelection()
        selection.update([], targetName: "Splitergoll")
        XCTAssertNil(selection.selected)
        selection.update(
            [first, KeyboardIdentity(id: UUID(), name: "Autre clavier")], targetName: "Splitergoll")
        XCTAssertEqual(selection.selected, first)
        XCTAssertTrue(selection.selectedIsAvailable)
        XCTAssertNil(selection.preferenceData)
        selection.update([first, second], targetName: "Splitergoll")
        XCTAssertEqual(selection.selected, first)
        XCTAssertFalse(selection.needsChoice)
        selection.update([second], targetName: "Splitergoll")
        XCTAssertEqual(selection.selected, second)
    }

    // Le tri, les doublons de découverte et l’ordre des périphériques ne deviennent pas une préférence.
    func testReorderingAndRepeatedCandidatesDoNotChooseBetweenTwoKeyboards() {
        var selection = KeyboardSelection()
        selection.update([second, first, second], targetName: "Splitergoll")
        XCTAssertEqual(selection.available, [first, second])
        XCTAssertNil(selection.selected)
        selection.update([first, second], targetName: "Splitergoll")
        XCTAssertNil(selection.selected)
        XCTAssertTrue(selection.select(second.id))
        selection.update([second, first], targetName: "Splitergoll")
        XCTAssertEqual(selection.selected, second)
    }

    // Le choix explicite survit à une nouvelle instance, pas les candidats ni les niveaux batterie.
    func testExplicitChoiceSurvivesRestartAndDoesNotFallBackWhenAbsent() throws {
        var selection = KeyboardSelection()
        selection.update([first, second], targetName: "Splitergoll")
        XCTAssertTrue(selection.select(second.id))
        var restarted = KeyboardSelection(preferenceData: try XCTUnwrap(selection.preferenceData))
        restarted.update([first], targetName: "Splitergoll")
        XCTAssertEqual(restarted.selected, second)
        XCTAssertEqual(restarted.displayName, second.name)
        XCTAssertFalse(restarted.selectedIsAvailable)
        XCTAssertFalse(restarted.select(UUID()))
        XCTAssertEqual(restarted.preferred, second)
        restarted.update([second, first], targetName: "Splitergoll")
        XCTAssertTrue(restarted.selectedIsAvailable)
        XCTAssertEqual(restarted.selected, second)
    }

    // Un nom nouveau ne remplace pas une identité connue, et un nom identique ne vole pas sa sélection.
    func testRenameUsesUUIDRatherThanTheName() throws {
        var selection = KeyboardSelection()
        selection.update([first], targetName: "Splitergoll")
        selection.select(first.id)
        let renamed = KeyboardIdentity(id: first.id, name: "Mon clavier personnel")
        let impostor = KeyboardIdentity(id: second.id, name: renamed.name)
        selection.update([impostor, renamed], targetName: "Splitergoll")
        XCTAssertEqual(selection.available, [renamed])
        XCTAssertEqual(selection.selected, renamed)
        let restarted = KeyboardSelection(preferenceData: try XCTUnwrap(selection.preferenceData))
        XCTAssertEqual(restarted.selected, renamed)
    }

    // Un nom momentanément absent ne vide pas le libellé mémorisé du clavier reconnu.
    func testMissingNameKeepsSavedLabelForTheSameIdentityOnly() {
        var selection = KeyboardSelection()
        selection.update([first], targetName: "Splitergoll")
        selection.select(first.id)
        selection.update(
            [
                KeyboardIdentity(id: first.id, name: " "),
                KeyboardIdentity(id: second.id, name: ""),
            ], targetName: "Splitergoll")
        XCTAssertEqual(selection.available, [first])
        XCTAssertEqual(selection.selected, first)
    }

    // Deux noms identiques restent deux destinations du menu avec des identifiants différents.
    func testDuplicateNamesHaveDistinctLabelsIncludingUUIDPrefixCollisions() {
        var selection = KeyboardSelection()
        let twin = KeyboardIdentity(id: second.id, name: first.name)
        let collision = KeyboardIdentity(
            id: UUID(uuidString: "11111111-0000-0000-0000-000000000003")!, name: first.name)
        selection.update([first, twin, collision], targetName: "Splitergoll")
        XCTAssertEqual(Set(selection.available.map { selection.label(for: $0) }).count, 3)
        XCTAssertTrue(selection.select(twin.id))
        XCTAssertEqual(selection.selected, twin)
    }

    // Une préférence corrompue ou provenant d’un autre format ne force aucune cible arbitraire.
    func testInvalidPreferenceFallsBackToNormalChoiceRules() {
        let invalid = [
            "not json", "{}",
            "{\"version\":2,\"keyboard\":{\"id\":\"\(first.id)\",\"name\":\"Splitergoll\"}}",
            "{\"version\":1,\"keyboard\":{\"id\":\"\(first.id)\",\"name\":\" \"}}",
        ]
        for value in invalid {
            var selection = KeyboardSelection(preferenceData: Data(value.utf8))
            XCTAssertNil(selection.preferred)
            selection.update([first, second], targetName: "Splitergoll")
            XCTAssertTrue(selection.needsChoice)
        }
    }

    // Le sélecteur conserve le filtre strict du lecteur CLI pour tous les appareils inconnus.
    func testUnrecognizedNamesAreNotAcceptedAsNewKeyboards() {
        var selection = KeyboardSelection()
        for name in ["", "SplitergollV2", "Splitergoll_Left", "splitergoll V1", "Splitergoll "] {
            selection.update([KeyboardIdentity(id: UUID(), name: name)], targetName: "Splitergoll")
            XCTAssertTrue(selection.available.isEmpty)
        }
    }

    // Les générations bloquent aussi une réponse du même identifiant après fermeture puis reconnexion.
    func testSessionRejectsOldResponsesAfterSwitchAndReconnect() {
        var gate = BatterySessionGate()
        let old = gate.begin(peripheralID: first.id)
        XCTAssertTrue(gate.accepts(old, peripheralID: first.id))
        XCTAssertFalse(gate.accepts(old, peripheralID: second.id))
        gate.end()
        XCTAssertFalse(gate.accepts(old, peripheralID: first.id))
        let next = gate.begin(peripheralID: second.id)
        XCTAssertFalse(gate.accepts(old, peripheralID: first.id))
        XCTAssertTrue(gate.accepts(next, peripheralID: second.id))
        let reconnected = gate.begin(peripheralID: first.id)
        XCTAssertFalse(gate.accepts(old, peripheralID: first.id))
        XCTAssertFalse(gate.accepts(next, peripheralID: second.id))
        XCTAssertTrue(gate.accepts(reconnected, peripheralID: first.id))
    }
}
