// Présente uniquement la connexion et les deux batteries dans une fenêtre compacte.
import BatteryCore
import Foundation
import SwiftUI

struct BatteriesView: View {
    @ObservedObject var monitor: BatteryMonitor

    var body: some View {
        BatteryDashboard(state: monitor.state, now: monitor.now)
    }
}

// Une vue de données permet de contrôler le rendu sans créer de connexion Bluetooth.
struct BatteryDashboard: View {
    static let size = NSSize(width: 320, height: 174)

    let state: BatteryState
    let now: Date

    var body: some View {
        VStack(spacing: 14) {
            HStack {
                Text("Splitergoll").font(.headline)
                Spacer()
                HStack(spacing: 5) {
                    Circle()
                        .fill(state.phase == .connected ? Color.green : .secondary)
                        .frame(width: 6, height: 6)
                    Text(state.connectionTitle).font(.caption)
                }
                .foregroundStyle(.secondary)
            }

            HStack(spacing: 10) {
                ForEach(BatterySide.allCases, id: \.self) { side in
                    batteryCard(side)
                }
            }
        }
        .padding(16)
        .frame(width: Self.size.width, height: Self.size.height)
        .background(Color(nsColor: .windowBackgroundColor))
    }

    // Une lecture indisponible est masquée. Cent pour cent reste une charge possible, jamais certaine.
    private func batteryCard(_ side: BatterySide) -> some View {
        let indication = state.indication(for: side, at: now)

        return VStack(alignment: .leading, spacing: 10) {
            Text(side.name).font(.caption).foregroundStyle(.secondary)
            Group {
                if indication == .possibleCharge {
                    Image(systemName: "battery.100percent.bolt")
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
