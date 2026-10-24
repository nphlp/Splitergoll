// Attache chaque réponse GATT à la génération qui l’a demandée, même après reconnexion au même clavier.
import BatteryCore
import CoreBluetooth

@MainActor
final class BatteryPeripheralDelegate: NSObject, @preconcurrency CBPeripheralDelegate {
    private weak var monitor: BatteryMonitor?
    private let token: BatterySessionGate.Token

    // Une instance est créée pour chaque connexion cliente, jamais réutilisée après fermeture.
    init(monitor: BatteryMonitor, token: BatterySessionGate.Token) {
        self.monitor = monitor
        self.token = token
    }

    // Centralise le refus des callbacks tardifs avant tout traitement, y compris celui des erreurs.
    private func reader(for peripheral: CBPeripheral) -> BatteryMonitor? {
        guard let monitor, monitor.accepts(token, peripheral: peripheral) else { return nil }
        return monitor
    }

    // Transmet seulement les services de la session active.
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        reader(for: peripheral)?.peripheral(peripheral, didDiscoverServices: error)
    }

    // Transmet les caractéristiques de cette génération.
    func peripheral(
        _ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?
    ) {
        reader(for: peripheral)?.peripheral(peripheral, didDiscoverCharacteristicsFor: service, error: error)
    }

    // Transmet les descripteurs de cette génération.
    func peripheral(
        _ peripheral: CBPeripheral, didDiscoverDescriptorsFor characteristic: CBCharacteristic, error: Error?
    ) {
        reader(for: peripheral)?.peripheral(
            peripheral, didDiscoverDescriptorsFor: characteristic, error: error)
    }

    // Associe les côtés uniquement dans la session qui les découvre.
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor descriptor: CBDescriptor, error: Error?) {
        reader(for: peripheral)?.peripheral(peripheral, didUpdateValueFor: descriptor, error: error)
    }

    // Refuse les niveaux ou erreurs reçus pour un ancien lecteur.
    func peripheral(
        _ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?
    ) {
        reader(for: peripheral)?.peripheral(peripheral, didUpdateValueFor: characteristic, error: error)
    }

    // Une invalidation ancienne ne ferme pas le nouveau lecteur.
    func peripheral(_ peripheral: CBPeripheral, didModifyServices invalidatedServices: [CBService]) {
        reader(for: peripheral)?.peripheral(peripheral, didModifyServices: invalidatedServices)
    }
}
