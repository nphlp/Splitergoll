// Gère une fenêtre compacte et un indicateur permanent, avec ouverture de session en arrière-plan.
import AppKit
import Combine
import SwiftUI

@main
enum SplitergollApplication {
    // Le bundle est requis pour le nom du clavier et l’autorisation Bluetooth macOS.
    @MainActor static func main() {
        let application = NSApplication.shared
        let delegate = ApplicationDelegate()
        application.delegate = delegate
        application.setActivationPolicy(.accessory)
        application.run()
        withExtendedLifetime(delegate) {}
    }
}

@MainActor
final class ApplicationDelegate: NSObject, NSApplicationDelegate, NSWindowDelegate {
    private var monitor: BatteryMonitor?
    private var window: NSWindow?
    private var statusItem: NSStatusItem?
    private var subscriptions: Set<AnyCancellable> = []

    // Valide la configuration du bundle avant de demander tout accès Bluetooth.
    func applicationDidFinishLaunching(_ notification: Notification) {
        guard let name = Bundle.main.object(forInfoDictionaryKey: "SplitergollKeyboardName") as? String,
            !name.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty
        else {
            let alert = NSAlert()
            alert.messageText = "Configuration de l’application absente"
            alert.informativeText =
                "Reconstruisez l’application avec make app build, puis ouvrez son fichier .app."
            alert.runModal()
            NSApp.terminate(nil)
            return
        }

        let monitor = BatteryMonitor(targetName: name)
        self.monitor = monitor
        configureMenu()
        configureWindow(monitor)
        monitor.$state.combineLatest(monitor.$now)
            .receive(on: RunLoop.main)
            .sink { [weak self] _, _ in
                // Attend la fin des affectations @Published avant de relire le modèle sur la file principale.
                MainActor.assumeIsolated { self?.updateMenu() }
            }.store(in: &subscriptions)
        updateMenu()
        monitor.start()
        LaunchAtLogin.configure()
    }

    // Une seule fenêtre est réutilisée lorsqu’on relance l’app depuis le Finder.
    func applicationShouldHandleReopen(_ sender: NSApplication, hasVisibleWindows flag: Bool) -> Bool {
        showWindow()
        return true
    }

    // Fermer la fenêtre laisse l’indicateur disponible, sans quitter la lecture.
    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool { false }

    // Libère la connexion cliente et les timers à la fermeture explicite de l’app.
    func applicationWillTerminate(_ notification: Notification) {
        monitor?.stop()
        subscriptions.removeAll()
        if let statusItem { NSStatusBar.system.removeStatusItem(statusItem) }
    }

    // Garde une petite fenêtre fixe, sans redimensionnement ni passage en plein écran.
    private func configureWindow(_ monitor: BatteryMonitor) {
        let window = NSWindow(
            contentRect: NSRect(origin: .zero, size: BatteryDashboard.size),
            styleMask: [.titled, .closable, .miniaturizable], backing: .buffered, defer: false
        )
        window.title = "Splitergoll"
        window.delegate = self
        window.contentViewController = NSHostingController(rootView: BatteriesView(monitor: monitor))
        window.contentMinSize = BatteryDashboard.size
        window.contentMaxSize = BatteryDashboard.size
        window.collectionBehavior = [.fullScreenNone]
        window.isReleasedWhenClosed = false
        window.center()
        self.window = window
    }

    // Ajoute le menu natif et l’indicateur, disponibles même quand la fenêtre est fermée.
    private func configureMenu() {
        let mainMenu = NSMenu()
        let applicationItem = NSMenuItem()
        let applicationMenu = NSMenu()
        let quit = NSMenuItem(
            title: "Quitter Splitergoll", action: #selector(quitApplication), keyEquivalent: "q")
        quit.target = self
        applicationMenu.addItem(quit)
        applicationItem.submenu = applicationMenu
        mainMenu.addItem(applicationItem)
        NSApp.mainMenu = mainMenu

        statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
    }

    // Met à jour le texte et les entrées du menu, en retirant les anciens niveaux de l’indicateur.
    private func updateMenu() {
        guard let monitor, let statusItem else { return }
        statusItem.button?.title = ""
        statusItem.button?.image = MenuBarBadge.image(state: monitor.state, at: monitor.now)
        statusItem.button?.imagePosition = .imageOnly
        let description = monitor.state.menuTitle(at: monitor.now)
        statusItem.button?.toolTip =
            "Splitergoll\n\(description)\nBatterie avec éclair : 100 % annoncé, charge non confirmée."
        statusItem.button?.setAccessibilityLabel("Batteries Splitergoll\n\(description)")
        let menu = NSMenu()
        let status = NSMenuItem(title: monitor.state.connectionTitle, action: nil, keyEquivalent: "")
        menu.addItem(status)
        menu.addItem(NSMenuItem.separator())
        for (title, action) in [
            ("Ouvrir l’app", #selector(showWindow)),
            ("Quitter l’app", #selector(quitApplication)),
        ] {
            let item = NSMenuItem(title: title, action: action, keyEquivalent: "")
            item.target = self
            menu.addItem(item)
        }
        menu.autoenablesItems = false
        status.isEnabled = false
        statusItem.menu = menu
    }

    // Réaffiche la même fenêtre et donne le focus à l’app, sans toucher au clavier connecté.
    @objc private func showWindow() {
        NSApp.setActivationPolicy(.regular)
        window?.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)
    }

    // Fermer la fenêtre retire l’app du Dock et conserve son indicateur dans la barre des menus.
    func windowWillClose(_ notification: Notification) {
        guard let closingWindow = notification.object as? NSWindow, closingWindow === window else { return }
        NSApp.setActivationPolicy(.accessory)
    }

    // Quitte proprement sans arrêter la liaison HID ni supprimer d’appairage.
    @objc private func quitApplication() { NSApp.terminate(nil) }
}
