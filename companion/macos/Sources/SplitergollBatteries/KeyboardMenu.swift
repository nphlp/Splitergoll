// Construit le choix du clavier suivi sans lancer le lecteur ou l’application.
import AppKit
import BatteryCore

@MainActor
enum KeyboardMenu {
    // Les identifiants sont transmis à l’action, les noms restent de simples libellés.
    static func item(selection: KeyboardSelection, target: AnyObject, action: Selector) -> NSMenuItem {
        let root = NSMenuItem(title: "Clavier suivi", action: nil, keyEquivalent: "")
        let menu = NSMenu()
        menu.autoenablesItems = false
        if let selected = selection.selected, !selection.selectedIsAvailable {
            let absent = NSMenuItem(title: "\(selected.name) (déconnecté)", action: nil, keyEquivalent: "")
            absent.state = .on
            absent.isEnabled = false
            menu.addItem(absent)
        }
        for keyboard in selection.available {
            let item = NSMenuItem(title: selection.label(for: keyboard), action: action, keyEquivalent: "")
            item.target = target
            item.representedObject = keyboard.id
            item.state = keyboard.id == selection.selected?.id ? .on : .off
            menu.addItem(item)
        }
        if menu.items.isEmpty {
            let empty = NSMenuItem(title: "Aucun clavier disponible", action: nil, keyEquivalent: "")
            empty.isEnabled = false
            menu.addItem(empty)
        }
        root.submenu = menu
        return root
    }
}
