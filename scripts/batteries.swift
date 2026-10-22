// Lit les deux batteries exposées par la centrale déjà connectée au Mac.
// snapshot affiche les dernières valeurs. average fait une moyenne par moitié sur 30 s.
// Les échanges sont des lectures GATT, sans modification du firmware ni des appairages.
import CoreBluetooth
import Darwin
import Foundation

enum ReadMode: String {
    case snapshot
    case average
}

enum BatterySide: String, CaseIterable, Hashable {
    case left = "Gauche"
    case right = "Droite"
}

final class BatteryReader: NSObject, CBCentralManagerDelegate, CBPeripheralDelegate {
    private let targetName: String
    private let mode: ReadMode
    private let batteryService = CBUUID(string: "180F")
    private let batteryLevel = CBUUID(string: "2A19")
    private let presentation = CBUUID(string: "2904")
    private var manager: CBCentralManager!
    private var device: CBPeripheral?
    private var characteristics: [CBCharacteristic] = []
    private var sides: [ObjectIdentifier: BatterySide] = [:]
    private var readings: [ObjectIdentifier: [UInt8]] = [:]
    private var timeoutTimer: Timer?
    private var pollTimer: Timer?
    private var averageTimer: Timer?

    // Initialise la cible et limite la connexion et la première lecture à 15 secondes.
    init(targetName: String, mode: ReadMode) {
        self.targetName = targetName
        self.mode = mode
        super.init()

        timeoutTimer = Timer.scheduledTimer(withTimeInterval: 15, repeats: false) { [weak self] _ in
            self?.fail("Les deux mesures n’ont pas été reçues. Réveille les deux moitiés et vérifie la connexion au Mac.")
        }
        manager = CBCentralManager(delegate: self, queue: nil)
    }

    // Ferme uniquement la connexion du lecteur et arrête ses délais.
    private func close() {
        timeoutTimer?.invalidate()
        pollTimer?.invalidate()
        averageTimer?.invalidate()
        if let device { manager.cancelPeripheralConnection(device) }
    }

    // Affiche une erreur explicite et retourne un statut non nul à Make.
    private func fail(_ message: String) {
        close()
        FileHandle.standardError.write(Data("Batteries : \(message)\n".utf8))
        exit(1)
    }

    // Vérifie que deux sources identifiées ont chacune une mesure valide.
    private func hasBothBatteries() -> Bool {
        characteristics.count == 2
            && Set(sides.values) == Set(BatterySide.allCases)
            && characteristics.allSatisfy { !(readings[ObjectIdentifier($0)] ?? []).isEmpty }
    }

    // Affiche immédiatement les valeurs ou démarre la fenêtre de moyenne après la première paire.
    private func reportIfReady() {
        guard hasBothBatteries() else { return }

        if mode == .snapshot {
            finish()
            return
        }

        guard averageTimer == nil else { return }
        timeoutTimer?.invalidate()
        print("Moyenne par moitié sur 30 secondes, lecture toutes les 5 secondes…")
        fflush(stdout)

        pollTimer = Timer.scheduledTimer(withTimeInterval: 5, repeats: true) { [weak self] _ in
            guard let self, let device = self.device else { return }
            for characteristic in self.characteristics { device.readValue(for: characteristic) }
        }
        averageTimer = Timer.scheduledTimer(withTimeInterval: 30, repeats: false) { [weak self] _ in
            self?.finish()
        }
    }

    // Affiche gauche et droite séparément, sans mélanger leurs niveaux dans une moyenne globale.
    private func finish() {
        guard hasBothBatteries() else { fail("Lecture des deux batteries incomplète."); return }

        for side in BatterySide.allCases {
            guard let id = sides.first(where: { $0.value == side })?.key,
                  let values = readings[id], let last = values.last else {
                fail("Mesure absente pour \(side.rawValue.lowercased()).")
                return
            }

            if mode == .snapshot {
                print("\(side.rawValue) : \(last) %")
            } else {
                guard values.count >= 2 else { fail("Pas assez de lectures pour calculer la moyenne."); return }
                let average = Double(values.reduce(0) { $0 + Int($1) }) / Double(values.count)
                let formatted = String(format: "%.1f", average).replacingOccurrences(of: ".", with: ",")
                print("\(side.rawValue) : \(formatted) % (\(values.count) lectures)")
            }
        }

        close()
        exit(0)
    }

    // Choisit uniquement le clavier déjà connecté, sans scan des appareils proches.
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
        case .poweredOn:
            guard device == nil else { return }
            let prefix = targetName + " "
            let candidates = central.retrieveConnectedPeripherals(withServices: [batteryService])
                .filter { peripheral in
                    guard !targetName.isEmpty, let name = peripheral.name else { return false }
                    return name == targetName
                        || (name.hasPrefix(prefix) && !name.dropFirst(prefix.count).allSatisfy(\.isWhitespace))
                }

            guard candidates.count == 1, let peripheral = candidates.first else {
                fail("Il faut un seul \(targetName) connecté et éveillé. Claviers trouvés : \(candidates.count).")
                return
            }
            device = peripheral
            peripheral.delegate = self
            central.connect(peripheral, options: nil)
        case .unauthorized:
            fail("Autorise l’accès Bluetooth dans les réglages de confidentialité macOS.")
        case .poweredOff:
            fail("Active le Bluetooth du Mac.")
        case .unsupported:
            fail("CoreBluetooth n’est pas accessible dans cet environnement.")
        case .unknown, .resetting:
            break
        @unknown default:
            fail("État Bluetooth non reconnu.")
        }
    }

    // Découvre les services batterie de la centrale après la connexion du lecteur.
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        peripheral.discoverServices([batteryService])
    }

    // Signale un refus de connexion sans effacer les clés Bluetooth.
    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
        fail("Connexion impossible : \(error?.localizedDescription ?? "cause inconnue").")
    }

    // Arrête la mesure si le clavier se déconnecte pendant la lecture.
    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        fail("Connexion perdue : \(error?.localizedDescription ?? "clavier déconnecté").")
    }

    // Vérifie les deux services attendus, puis cherche leur caractéristique de niveau.
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        if let error { fail("Découverte des services impossible : \(error.localizedDescription)."); return }

        let services = (peripheral.services ?? []).filter { $0.uuid == batteryService }
        guard services.count == 2 else {
            fail("Deux services batterie attendus, \(services.count) reçus. Vérifie le firmware Bluetooth avec proxy batterie.")
            return
        }
        for service in services { peripheral.discoverCharacteristics([batteryLevel], for: service) }
    }

    // Lit le niveau de chaque service et recherche son descripteur d’identification.
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        if let error { fail("Découverte des niveaux impossible : \(error.localizedDescription)."); return }

        let levels = (service.characteristics ?? []).filter { $0.uuid == batteryLevel }
        guard levels.count == 1, let characteristic = levels.first, characteristic.properties.contains(.read) else {
            fail("Une mesure lisible est attendue dans chaque service batterie.")
            return
        }
        characteristics.append(characteristic)
        peripheral.readValue(for: characteristic)
        peripheral.discoverDescriptors(for: characteristic)
    }

    // Lit uniquement la présentation GATT, sans changer l’abonnement aux notifications.
    func peripheral(_ peripheral: CBPeripheral, didDiscoverDescriptorsFor characteristic: CBCharacteristic, error: Error?) {
        if let error { fail("Découverte des descripteurs impossible : \(error.localizedDescription)."); return }

        guard let descriptor = characteristic.descriptors?.first(where: { $0.uuid == presentation }) else {
            fail("La batterie ne fournit pas son descripteur d’identification.")
            return
        }
        peripheral.readValue(for: descriptor)
    }

    // Associe main à gauche et auxiliary à droite, sans utiliser l’ordre de découverte.
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor descriptor: CBDescriptor, error: Error?) {
        if let error { fail("Identification impossible : \(error.localizedDescription)."); return }

        guard let characteristic = descriptor.characteristic,
              let data = descriptor.value as? Data, data.count == 7,
              data[0] == 0x04, data[1] == 0, data[2] == 0xAD, data[3] == 0x27, data[4] == 1 else {
            fail("Présentation de batterie invalide.")
            return
        }

        let source = UInt16(data[5]) | UInt16(data[6]) << 8
        let side: BatterySide
        switch source {
        case 0x0106: side = .left
        case 0x0108: side = .right
        default: fail("Source de batterie non reconnue."); return
        }
        sides[ObjectIdentifier(characteristic)] = side
        reportIfReady()
    }

    // Contrôle le format et mémorise les valeurs annoncées, même si elles sont identiques.
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        if let error { fail("Lecture impossible : \(error.localizedDescription)."); return }

        guard let data = characteristic.value, data.count == 1, let level = data.first, level <= 100 else {
            fail("Niveau batterie invalide.")
            return
        }
        readings[ObjectIdentifier(characteristic), default: []].append(level)
        reportIfReady()
    }
}

guard CommandLine.arguments.count == 3, let mode = ReadMode(rawValue: CommandLine.arguments[2]) else {
    FileHandle.standardError.write(Data("Usage : batteries NOM_DU_CLAVIER snapshot|average\n".utf8))
    exit(2)
}

let reader = BatteryReader(targetName: CommandLine.arguments[1], mode: mode)
RunLoop.main.run()
