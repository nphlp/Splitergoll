// Réunit le clavier suivi, ses batteries et le diagnostic des deux liaisons dans une fenêtre compacte.
import BatteryCore
import Foundation
import SwiftUI

struct BatteriesView: View {
    static let width: CGFloat = 380
    @ObservedObject var monitor: BatteryMonitor
    @ObservedObject var diagnostic: HostIdentityMonitor
    let select: (UUID) -> Void
    let restart: () -> Void
    let resize: () -> Void
    @State private var detailsExpanded = false

    var body: some View {
        VStack(alignment: .leading, spacing: 16) {
            HStack {
                Text("Splitergoll").font(.title3.bold())
                Spacer()
                HStack(spacing: 5) {
                    Circle()
                        .fill(monitor.state.phase == .connected ? Color.green : .secondary)
                        .frame(width: 6, height: 6)
                    Text(monitor.state.connectionTitle).font(.caption)
                }.foregroundStyle(.secondary)
            }

            KeyboardConnectionsView(
                rows: KeyboardConnections.rows(
                    selection: monitor.selection, usb: diagnostic.usb, ble: diagnostic.ble,
                    runtime: diagnostic.currentRuntime, at: diagnostic.now), select: select)

            VStack(alignment: .leading, spacing: 8) {
                Text("Batteries · \(monitor.selection.displayName)")
                    .font(.caption).foregroundStyle(.secondary).lineLimit(1)
                    .help(monitor.selection.displayName)
                BatteryDashboard(state: monitor.state, now: monitor.now)
            }
            Divider()
            HostIdentityView(
                diagnostic: diagnostic, restart: restart, resize: resize, expanded: $detailsExpanded)
        }
        .padding(18)
        .frame(width: Self.width)
        .fixedSize(horizontal: false, vertical: true)
        .background(Color(nsColor: .windowBackgroundColor))
    }
}

// Les cartes restent indépendantes des lecteurs pour vérifier leur rendu sans accès au clavier.
struct BatteryDashboard: View {
    let state: BatteryState
    let now: Date

    var body: some View {
        HStack(spacing: 10) {
            ForEach(BatterySide.allCases, id: \.self) { side in
                batteryCard(side)
            }
        }
    }

    // Une lecture indisponible est masquée. Cent pour cent reste une charge possible, jamais certaine.
    private func batteryCard(_ side: BatterySide) -> some View {
        let indication = state.indication(for: side, at: now)

        return VStack(alignment: .leading, spacing: 10) {
            HStack {
                Image(nsImage: MenuBarBadge.contour(for: side))
                    .resizable().scaledToFit().frame(width: 26, height: 22)
                    .accessibilityHidden(true)
                Spacer()
                Text(side.name).font(.caption).foregroundStyle(.secondary)
            }
            Group {
                if indication == .possibleCharge {
                    Image(systemName: "bolt.fill")
                        .accessibilityLabel("Charge possible, niveau annoncé à 100 pour cent")
                } else {
                    Text(indication.menuValue)
                        .monospacedDigit()
                        .lineLimit(1)
                        .minimumScaleFactor(0.8)
                }
            }
            .font(.system(size: 25, weight: .semibold, design: .rounded))
            .foregroundStyle(indication == .unavailable ? Color.secondary : .primary)
            .frame(height: 32, alignment: .leading)
        }
        .padding(12)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(Color(nsColor: .controlBackgroundColor), in: RoundedRectangle(cornerRadius: 10))
        .help(
            indication == .possibleCharge
                ? "100 % annoncé : USB ou batterie pleine. La charge n’est pas confirmée."
                : (indication == .unavailable
                    ? "Batterie indisponible" : "Batterie \(side.name.lowercased())")
        )
    }
}
