// Présente un diagnostic explicite, sans prétendre que le transport de saisie a changé.
import AppKit
import SwiftUI

struct HostIdentityView: View {
    @ObservedObject var diagnostic: HostIdentityMonitor
    let restart: () -> Void

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Identité USB/Bluetooth").font(.headline)
            Text("Diagnostic uniquement. La saisie reste en Bluetooth.").font(.caption).foregroundStyle(
                .secondary)
            Grid(alignment: .leading, horizontalSpacing: 14, verticalSpacing: 10) {
                GridRow {
                    Text("USB")
                    Text(diagnostic.usbText).foregroundStyle(.secondary)
                }
                GridRow {
                    Text("Bluetooth")
                    Text(diagnostic.bluetoothText).foregroundStyle(.secondary)
                }
            }.font(.callout)
            Text(diagnostic.verdict).font(.callout.bold())
            Text(
                "L’app conserve un identifiant local dans ses données utilisateur. Ce n’est ni un mot de passe ni une clé d’appairage."
            )
            .font(.caption).foregroundStyle(.secondary)
            HStack {
                Button(diagnostic.running ? "Arrêter" : "Relancer") {
                    if diagnostic.running { diagnostic.stop() } else { restart() }
                }
                Spacer()
                Button("Copier le diagnostic") {
                    let text =
                        "USB : \(diagnostic.usbText)\nBluetooth : \(diagnostic.bluetoothText)\nRésultat : \(diagnostic.verdict)\nAucune bascule de saisie. Aucun identifiant exporté."
                    NSPasteboard.general.clearContents()
                    NSPasteboard.general.setString(text, forType: .string)
                }
            }
        }.padding(18).frame(width: 390).fixedSize(horizontal: false, vertical: true)
    }
}
