import AppKit
import BatteryCore
import XCTest

@testable import SplitergollBatteries

@MainActor
private final class KeyboardMenuReceiver: NSObject {
    var selected: UUID?

    // Reçoit l’action native du menu sans créer de lecteur Bluetooth.
    @objc func choose(_ sender: NSMenuItem) { selected = sender.representedObject as? UUID }
}

final class KeyboardMenuTests: XCTestCase {
    // Le menu présente deux choix distincts et transmet le bon UUID à son action native.
    @MainActor func testMenuDispatchesSelectionByIdentityAndChecksOnlyChosenKeyboard() throws {
        let first = KeyboardIdentity(id: UUID(), name: "Splitergoll V1")
        let second = KeyboardIdentity(id: UUID(), name: "Splitergoll V2")
        var selection = KeyboardSelection()
        selection.update([first, second], targetName: "Splitergoll")
        selection.select(second.id)
        let receiver = KeyboardMenuReceiver()
        let root = KeyboardMenu.item(
            selection: selection, target: receiver, action: #selector(KeyboardMenuReceiver.choose))
        let menu = try XCTUnwrap(root.submenu)
        XCTAssertEqual(root.title, "Clavier suivi")
        XCTAssertEqual(menu.items.map(\.title), [first.name, second.name])
        XCTAssertEqual(menu.items.map(\.state), [.off, .on])
        for (index, id) in [first.id, second.id].enumerated() {
            let item = menu.items[index]
            XCTAssertTrue(item.isEnabled)
            XCTAssertTrue(item.target === receiver)
            XCTAssertTrue(
                NSApplication.shared.sendAction(try XCTUnwrap(item.action), to: item.target, from: item))
            XCTAssertEqual(receiver.selected, id)
        }
    }

    // L’absence du préféré garde sa coche et son nom, sans attribuer ses valeurs à l’autre clavier.
    @MainActor func testAbsentPreferredKeyboardRemainsVisibleButCannotBeSelectedAsAvailable() throws {
        let first = KeyboardIdentity(id: UUID(), name: "Splitergoll V1")
        let second = KeyboardIdentity(id: UUID(), name: "Splitergoll V2")
        var selection = KeyboardSelection()
        selection.update([second], targetName: "Splitergoll")
        selection.select(second.id)
        selection.update([first], targetName: "Splitergoll")
        let receiver = KeyboardMenuReceiver()
        let menu = try XCTUnwrap(
            KeyboardMenu.item(
                selection: selection, target: receiver, action: #selector(KeyboardMenuReceiver.choose)
            ).submenu)
        XCTAssertEqual(menu.items.map(\.title), ["Splitergoll V2 (déconnecté)", first.name])
        XCTAssertEqual(menu.items.map(\.state), [.on, .off])
        XCTAssertFalse(menu.items[0].isEnabled)
        XCTAssertNil(menu.items[0].action)
        XCTAssertTrue(menu.items[1].isEnabled)
    }

    // Aucun clavier ne crée un faux choix ni une action sans cible.
    @MainActor func testEmptyMenuExplainsAbsenceWithoutAnAction() throws {
        let receiver = KeyboardMenuReceiver()
        let menu = try XCTUnwrap(
            KeyboardMenu.item(
                selection: KeyboardSelection(), target: receiver,
                action: #selector(KeyboardMenuReceiver.choose)
            ).submenu)
        XCTAssertEqual(menu.items.count, 1)
        XCTAssertEqual(menu.items[0].title, "Aucun clavier disponible")
        XCTAssertFalse(menu.items[0].isEnabled)
        XCTAssertNil(menu.items[0].action)
    }
}
