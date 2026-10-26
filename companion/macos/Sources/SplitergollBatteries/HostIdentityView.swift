// Affiche les informations du clavier et réserve les détails techniques à une section repliable.
import AppKit
import SwiftUI

struct HostIdentityView: View {
    @ObservedObject var diagnostic: HostIdentityMonitor
    let restart: () -> Void
    let resize: () -> Void
    @Binding var expanded: Bool

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Grid(alignment: .leading, horizontalSpacing: 18, verticalSpacing: 10) {
                row("OS actif", diagnostic.currentRuntime?.os.title ?? "Non communiqué")
                row("Saisie", diagnostic.currentRuntime?.transport.title ?? "Non communiquée")
            }.font(.callout)

            Text(diagnostic.verdict).font(.callout.bold()).fixedSize(horizontal: false, vertical: true)
            DisclosureGroup("Diagnostic", isExpanded: $expanded) {
                VStack(alignment: .leading, spacing: 10) {
                    Grid(alignment: .leading, horizontalSpacing: 18, verticalSpacing: 8) {
                        row("USB", diagnostic.usbText)
                        row("Bluetooth", diagnostic.bluetoothText)
                    }
                    Text(
                        diagnostic.currentRuntime?.usbPriority == true
                            ? "USB prioritaire pour le même ordinateur. Bluetooth reste connecté, sans saisie en double."
                            : "Comparaison uniquement, aucune bascule de saisie.")
                    Text(
                        "L’identité locale conservée par l’app n’est ni un mot de passe ni une clé d’appairage."
                    )
                    HStack {
                        Button(diagnostic.running ? "Arrêter" : "Relancer") {
                            if diagnostic.running { diagnostic.pauseByUser() } else { restart() }
                        }
                        Spacer()
                        Button("Copier le diagnostic") {
                            NSPasteboard.general.clearContents()
                            NSPasteboard.general.setString(diagnostic.diagnosticText, forType: .string)
                        }
                    }
                }.font(.caption).foregroundStyle(.secondary).padding(.top, 8)
            }
            .font(.caption)
            .onChange(of: expanded) { _ in resize() }
        }
    }

    // Une valeur longue peut grandir en hauteur sans empiéter sur le libellé de sa ligne.
    private func row(_ title: String, _ value: String) -> some View {
        GridRow(alignment: .top) {
            Text(title).foregroundStyle(.secondary)
            Text(value).frame(maxWidth: .infinity, alignment: .leading)
                .fixedSize(horizontal: false, vertical: true)
        }
    }
}
