import BatteryCore
import Foundation
import XCTest

@testable import SplitergollBatteries

@MainActor
private final class MemoryKeyboardPreferences: KeyboardPreferenceStorage {
    var writes = 0
    var data: Data? { didSet { writes += 1 } }
}

final class BatteryMonitorSelectionTests: XCTestCase {
    // Le vrai lecteur sauvegarde le choix explicite une seule fois et le retrouve après recréation.
    @MainActor func testMonitorPersistsExplicitChoiceWithoutPeriodicRewrites() {
        let first = KeyboardIdentity(id: UUID(), name: "Splitergoll V1")
        let second = KeyboardIdentity(id: UUID(), name: "Splitergoll V2")
        let storage = MemoryKeyboardPreferences()
        let monitor = BatteryMonitor(targetName: "Splitergoll", preferences: storage)
        monitor.updateKeyboards([first])
        XCTAssertEqual(storage.writes, 0)
        monitor.updateKeyboards([first, second])
        monitor.selectKeyboard(second.id)
        XCTAssertEqual(monitor.selection.selected, second)
        XCTAssertEqual(storage.writes, 1)
        for _ in 0..<10 { monitor.updateKeyboards([second, first]) }
        monitor.selectKeyboard(second.id)
        monitor.selectKeyboard(UUID())
        XCTAssertEqual(storage.writes, 1)
        let restarted = BatteryMonitor(targetName: "Splitergoll", preferences: storage)
        restarted.updateKeyboards([first])
        XCTAssertEqual(restarted.selection.selected, second)
        XCTAssertFalse(restarted.selection.selectedIsAvailable)
        XCTAssertEqual(restarted.state.indication(for: .left, at: Date()), .unavailable)
        XCTAssertEqual(restarted.state.indication(for: .right, at: Date()), .unavailable)
    }

    // Un renommage connu actualise le libellé durable sans accepter un clavier inconnu de même nom.
    @MainActor func testMonitorPersistsRenamedChoiceAndKeepsItWhenDisconnected() {
        let original = KeyboardIdentity(id: UUID(), name: "Splitergoll V2")
        let renamed = KeyboardIdentity(id: original.id, name: "Mon V2")
        let storage = MemoryKeyboardPreferences()
        let monitor = BatteryMonitor(targetName: "Splitergoll", preferences: storage)
        monitor.updateKeyboards([original])
        monitor.selectKeyboard(original.id)
        monitor.updateKeyboards([renamed])
        XCTAssertEqual(storage.writes, 2)
        XCTAssertEqual(monitor.selection.displayName, "Mon V2")
        monitor.updateKeyboards([KeyboardIdentity(id: UUID(), name: "Mon V2")])
        XCTAssertTrue(monitor.selection.available.isEmpty)
        XCTAssertFalse(monitor.selection.selectedIsAvailable)
        XCTAssertEqual(storage.writes, 2)
        let restarted = BatteryMonitor(targetName: "Splitergoll", preferences: storage)
        XCTAssertEqual(restarted.selection.selected, renamed)
    }
}
