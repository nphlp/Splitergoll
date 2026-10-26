// Présente les liaisons observées, sans confondre le clavier suivi et la destination des frappes.
import Foundation

public struct KeyboardConnection: Identifiable, Equatable, Sendable {
    public enum Link: String, Sendable {
        case usb = "USB"
        case bluetooth = "Bluetooth"
    }
    public enum State: String, Sendable {
        case connected = "Connecté"
        case input = "Sélectionné"
        case otherHost = "Autre ordinateur"
        case standby = "Désactivé"
        case diagnostic = "Diagnostic"
        case disconnected = "Déconnecté"
    }

    public let keyboardID: UUID
    public let name: String
    public let link: Link
    public let state: State
    public let followed: Bool
    public var id: String { "\(keyboardID.uuidString)/\(link.rawValue)" }
    public var canFollow: Bool { state != .disconnected }
    public var isSelected: Bool { state == .input }
}

public enum KeyboardConnections {
    // Les noms ne rapprochent jamais les liaisons. Seul le contexte du même contrôleur le permet.
    public static func rows(
        selection: KeyboardSelection,
        usb: HostIdentityProtocol.Observation?, ble: HostIdentityProtocol.Observation?,
        runtime: KeyboardRuntime?, at now: Date
    ) -> [KeyboardConnection] {
        let currentBLE = fresh(ble, transport: .bluetooth, at: now)
        let currentUSB = fresh(usb, transport: .usb, at: now)
        let status = runtime.flatMap { $0.isCurrent(boot: currentBLE?.boot, at: now) ? $0 : nil }
        let hasUSB =
            currentUSB != nil && currentBLE != nil
            && currentUSB?.boot == currentBLE?.boot && currentUSB?.usbReady == true
        var rows: [KeyboardConnection] = []

        for keyboard in selection.available {
            let followed = keyboard.id == selection.selected?.id
            let name = selection.label(for: keyboard)
            if followed && hasUSB {
                rows.append(
                    KeyboardConnection(
                        keyboardID: keyboard.id, name: name, link: .usb,
                        state: status?.transport == .usb
                            ? .input : status?.usbPriority == true ? .standby : .diagnostic,
                        followed: true))
            }
            var state = KeyboardConnection.State.connected
            if followed, let status {
                if status.transport == .bluetooth, let profile = status.profile {
                    state = profile == currentBLE?.slot ? .input : .otherHost
                } else if status.transport == .usb && hasUSB {
                    state = .standby
                }
            }
            rows.append(
                KeyboardConnection(
                    keyboardID: keyboard.id, name: name, link: .bluetooth, state: state, followed: followed))
        }
        if let absent = selection.selected, !selection.selectedIsAvailable {
            rows.append(
                KeyboardConnection(
                    keyboardID: absent.id, name: absent.name, link: .bluetooth,
                    state: .disconnected, followed: true))
        }
        return rows
    }

    // Une preuve ancienne, future ou provenant de l’autre canal ne crée jamais une liaison visible.
    private static func fresh(
        _ observation: HostIdentityProtocol.Observation?,
        transport: HostIdentityProtocol.Transport, at now: Date
    ) -> HostIdentityProtocol.Report? {
        guard let observation, observation.report.transport == transport else { return nil }
        let age = now.timeIntervalSince(observation.received)
        return age >= 0 && age < HostIdentityProtocol.lease ? observation.report : nil
    }
}
