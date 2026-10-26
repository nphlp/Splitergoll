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
    private let identityMonitor = HostIdentityMonitor()
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
        monitor.$state.combineLatest(monitor.$now, monitor.$selection)
            .receive(on: RunLoop.main)
            .sink { [weak self] _, _, _ in
                // Attend la fin des affectations @Published avant de relire le modèle sur la file principale.
                MainActor.assumeIsolated {
                    self?.updateMenu()
                    self?.followKeyboardSelection()
                    self?.fitWindow()
                }
            }.store(in: &subscriptions)
        identityMonitor.objectWillChange.receive(on: RunLoop.main)
            .sink { [weak self] _ in
                MainActor.assumeIsolated { self?.fitWindow() }
            }.store(in: &subscriptions)
        updateMenu()
        monitor.start()
        NSWorkspace.shared.notificationCenter.addObserver(
            self, selector: #selector(identityWillSleep), name: NSWorkspace.willSleepNotification, object: nil
        )
        NSWorkspace.shared.notificationCenter.addObserver(
            self, selector: #selector(identityDidWake), name: NSWorkspace.didWakeNotification, object: nil
        )
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
        identityMonitor.stop()
        NSWorkspace.shared.notificationCenter.removeObserver(self)
        subscriptions.removeAll()
        if let statusItem { NSStatusBar.system.removeStatusItem(statusItem) }
    }

    // La largeur reste compacte et la hauteur suit les informations et les détails dépliés.
    private func configureWindow(_ monitor: BatteryMonitor) {
        let window = NSWindow(
            contentRect: NSRect(x: 0, y: 0, width: BatteriesView.width, height: 380),
            styleMask: [.titled, .closable, .miniaturizable], backing: .buffered, defer: false
        )
        window.title = "Splitergoll"
        window.delegate = self
        window.contentViewController = NSHostingController(
            rootView: BatteriesView(
                monitor: monitor, diagnostic: identityMonitor,
                select: { [weak self] id in self?.chooseKeyboard(id) },
                restart: { [weak self] in self?.startIdentity() },
                resize: { [weak self] in
                    Task { @MainActor in self?.fitWindow() }
                }))
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
            "\(monitor.selection.displayName)\n\(description)\nÉclair : 100 % annoncé, charge non confirmée."
        statusItem.button?.setAccessibilityLabel("Batteries \(monitor.selection.displayName)\n\(description)")
        let menu = NSMenu()
        let status = NSMenuItem(title: monitor.state.connectionTitle, action: nil, keyEquivalent: "")
        menu.addItem(status)
        menu.addItem(
            KeyboardMenu.item(selection: monitor.selection, target: self, action: #selector(selectKeyboard)))
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

    // Le menu et le sélecteur de fenêtre utilisent la même sélection, sans rediriger les frappes.
    @objc private func selectKeyboard(_ sender: NSMenuItem) {
        guard let id = sender.representedObject as? UUID else { return }
        chooseKeyboard(id)
    }

    // La sélection persistée ne concerne que les informations suivies par l’application.
    private func chooseKeyboard(_ id: UUID) {
        monitor?.selectKeyboard(id)
        followKeyboardSelection()
    }

    // Le choix suivi renouvelle aussi l’identité en arrière-plan pour autoriser la priorité USB.
    private func followKeyboardSelection() {
        identityMonitor.maintain(
            selected: monitor?.selection.selected?.id,
            available: monitor?.selection.selectedIsAvailable == true && monitor?.state.phase == .connected)
    }

    // Réaffiche la même fenêtre et donne le focus à l’app, sans toucher au clavier connecté.
    @objc private func showWindow() {
        NSApp.setActivationPolicy(.regular)
        window?.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)
        followKeyboardSelection()
        fitWindow()
    }

    // Fermer la fenêtre retire l’app du Dock et conserve son indicateur dans la barre des menus.
    func windowWillClose(_ notification: Notification) {
        guard let closingWindow = notification.object as? NSWindow else { return }
        guard closingWindow === window else { return }
        NSApp.setActivationPolicy(.accessory)
    }

    // Relancer réactive explicitement le compagnon arrêté, sans changer l’appareil sélectionné par le clavier.
    private func startIdentity() {
        identityMonitor.start(selected: monitor?.selection.selected?.id)
    }

    // Ne prolonge ni l’activité de la machine ni les annonces du diagnostic pendant le sommeil.
    @objc private func identityWillSleep() { identityMonitor.pauseForSleep() }

    // Le réveil ne force pas de reconnexion système, le lecteur attend le clavier déjà disponible.
    @objc private func identityDidWake() {
        identityMonitor.resumeAfterSleep()
        followKeyboardSelection()
    }

    // Les messages longs du diagnostic ajustent la hauteur au lieu de rogner leur texte.
    private func fitWindow() {
        guard let window, let view = window.contentViewController?.view else { return }
        view.layoutSubtreeIfNeeded()
        let size = NSSize(width: BatteriesView.width, height: max(340, view.fittingSize.height))
        if window.contentLayoutRect.size != size { window.setContentSize(size) }
    }

    // Quitte proprement sans arrêter la liaison HID ni supprimer d’appairage.
    @objc private func quitApplication() { NSApp.terminate(nil) }
}
