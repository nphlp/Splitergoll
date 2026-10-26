// Une ligne par liaison connue. Cliquer suit le clavier, sans changer le transport de saisie.
import BatteryCore
import SwiftUI

struct KeyboardConnectionsView: View {
    let rows: [KeyboardConnection]
    let select: (UUID) -> Void

    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            HStack {
                Text("Clavier").frame(maxWidth: .infinity, alignment: .leading)
                Text("Liaison").frame(width: 66, alignment: .leading)
                Text("État").frame(width: 83, alignment: .leading)
            }
            .font(.caption).foregroundStyle(.secondary).padding(.horizontal, 10).padding(.vertical, 8)
            Divider()
            if rows.isEmpty {
                Text("Aucun clavier connecté")
                    .font(.callout).foregroundStyle(.secondary)
                    .frame(maxWidth: .infinity, minHeight: 44)
            } else {
                ScrollView {
                    VStack(spacing: 0) {
                        ForEach(rows) { row in
                            connection(row)
                            if row.id != rows.last?.id { Divider() }
                        }
                    }
                }
                .frame(height: min(CGFloat(rows.count) * 45 - 1, 179))
            }
        }
        .background(Color(nsColor: .controlBackgroundColor), in: RoundedRectangle(cornerRadius: 10))
        .overlay(RoundedRectangle(cornerRadius: 10).stroke(.secondary.opacity(0.15)))
        .accessibilityLabel("Connexions des claviers")
    }

    // La coche désigne uniquement la liaison qui reçoit les frappes, jamais les deux lignes du même clavier.
    private func connection(_ row: KeyboardConnection) -> some View {
        Button {
            select(row.keyboardID)
        } label: {
            HStack {
                HStack(spacing: 5) {
                    Image(systemName: row.isSelected ? "checkmark.circle.fill" : "circle")
                        .foregroundStyle(row.isSelected ? Color.accentColor : .secondary)
                        .font(.caption)
                    Text(row.name).lineLimit(1).truncationMode(.tail)
                        .frame(maxWidth: .infinity, alignment: .leading)
                }
                .frame(maxWidth: .infinity, alignment: .leading)
                Text(row.link.rawValue).frame(width: 66, alignment: .leading)
                Text(row.state.rawValue).font(.caption).foregroundStyle(.secondary)
                    .frame(width: 83, alignment: .leading)
            }
            .font(.callout).padding(.horizontal, 10).frame(height: 44)
            .contentShape(Rectangle())
            .background(row.isSelected ? Color.accentColor.opacity(0.07) : .clear)
        }
        .buttonStyle(.plain).disabled(!row.canFollow)
        .help("\(row.name), \(row.link.rawValue), \(row.state.rawValue). Suivre ce clavier dans l’app.")
        .accessibilityLabel("\(row.name), \(row.link.rawValue), \(row.state.rawValue)")
        .accessibilityValue(row.isSelected ? "Liaison sélectionnée" : "")
    }
}
