// Lit uniquement les services batterie du clavier déjà connecté au Mac, sans scan ni notifications.
import AppKit
import BatteryCore
import CoreBluetooth
import Foundation

@MainActor
final class BatteryMonitor: NSObject, ObservableObject {
    @Published private(set) var state = BatteryState()
    @Published private(set) var now = Date()

    let targetName: String
    private let serviceID = CBUUID(string: "180F")
    private let levelID = CBUUID(string: "2A19")
    private let presentationID = CBUUID(string: "2904")
    private var central: CBCentralManager?
    private var device: CBPeripheral?
    private var closingDevice: CBPeripheral?
    private var lastDeviceID: UUID?
    private var levels: [ObjectIdentifier: CBCharacteristic] = [:]
    private var channels: [ObjectIdentifier: Int] = [:]
    private var pendingReads: Set<ObjectIdentifier> = []
    private var heartbeat: Timer?
    private var connectionStarted: Date?
    private var readStarted: Date?
    private var retryAfter = Date.distantPast
    private var suspended = false
    private var stopped = false

    // Le nom vient du bundle préparé depuis la configuration du clavier, jamais d’un nom supposé.
    init(targetName: String) {
        self.targetName = targetName
        super.init()
    }

    // Démarre le lecteur à l’ouverture de l’app, sur la file principale de CoreBluetooth.
    func start() {
        guard central == nil else { return }
        central = CBCentralManager(
            delegate: self, queue: .main,
            options: [CBCentralManagerOptionShowPowerAlertKey: false]
        )
        let workspace = NSWorkspace.shared.notificationCenter
        workspace.addObserver(
            self, selector: #selector(willSleep), name: NSWorkspace.willSleepNotification, object: nil)
        workspace.addObserver(
            self, selector: #selector(didWake), name: NSWorkspace.didWakeNotification, object: nil)
        startHeartbeat()
    }

    // Arrête les seules ressources du lecteur. Les bonds et la connexion HID ne sont pas modifiés.
    func stop() {
        stopped = true
        heartbeat?.invalidate()
        heartbeat = nil
        NSWorkspace.shared.notificationCenter.removeObserver(self)
        releaseDevice(phase: .disconnected)
        central?.delegate = nil
    }

    // Le timer est tolérant et ne pose aucune assertion empêchant le sommeil du Mac.
    private func startHeartbeat() {
        heartbeat?.invalidate()
        let timer = Timer(timeInterval: BatteryPolling.interval, repeats: true) { [weak self] _ in
            Task { @MainActor in self?.tick() }
        }
        timer.tolerance = 0.25
        RunLoop.main.add(timer, forMode: .common)
        heartbeat = timer
    }

    // Chaque battement relit les niveaux disponibles, sans attendre un second délai sujet aux décalages d’horloge.
    private func tick() {
        guard !suspended, !stopped else { return }
        now = Date()
        guard central?.state == .poweredOn else { return }

        if let started = connectionStarted, now.timeIntervalSince(started) >= 15 {
            failSession("La connexion du lecteur n’a pas abouti. Réveille le clavier.")
            return
        }
        if let started = readStarted, now.timeIntervalSince(started) >= 15 {
            pendingReads.removeAll()
            readStarted = nil
            if !state.hasCurrentReading {
                failSession("Les batteries n’ont pas répondu. Réveille les deux moitiés.")
                return
            }
        }

        if device == nil {
            findConnectedKeyboard()
        } else if device?.state == .connected {
            readLevels()
        }
    }

    // Retrouve les périphériques déjà reliés au système, en refusant les noms identiques ambigus.
    private func findConnectedKeyboard() {
        guard let central, central.state == .poweredOn, device == nil,
            closingDevice == nil, now >= retryAfter
        else { return }

        let candidates = central.retrieveConnectedPeripherals(withServices: [serviceID])
        switch ConnectedKeyboardChoice.select(names: candidates.map(\.name), targetName: targetName) {
        case .none:
            state.phase = .waiting
        case .ambiguous:
            state.phase = .ambiguous
        case .single(let index):
            let peripheral = candidates[index]
            if peripheral.identifier != lastDeviceID { state = BatteryState() }
            lastDeviceID = peripheral.identifier
            state.beginSession()
            device = peripheral
            connectionStarted = now
            peripheral.delegate = self
            central.connect(peripheral, options: nil)
        }
    }

    // Ferme uniquement la connexion cliente de l’app et attend sa clôture avant toute nouvelle tentative.
    private func releaseDevice(phase: MonitorPhase) {
        let previous = device
        device = nil
        previous?.delegate = nil
        levels.removeAll()
        channels.removeAll()
        pendingReads.removeAll()
        connectionStarted = nil
        readStarted = nil
        state.phase = phase

        if let previous, let central, central.state == .poweredOn {
            closingDevice = previous
            central.cancelPeripheralConnection(previous)
        }
    }

    // Conserve l’erreur et les anciennes lectures. Limite les reprises après un firmware incompatible.
    private func failSession(_ message: String) {
        retryAfter = Date().addingTimeInterval(30)
        releaseDevice(phase: .failed(message))
    }

    // Ne lit pas un second lot tant que le premier attend encore sa réponse.
    private func readLevels() {
        guard let device, device.state == .connected, pendingReads.isEmpty, !levels.isEmpty else { return }
        readStarted = now
        pendingReads = Set(levels.keys)
        for characteristic in levels.values { device.readValue(for: characteristic) }
    }

    // Une lecture valide identifiée suffit à afficher sa moitié, sans inventer la batterie manquante.
    private func publishReadiness() {
        if state.hasCurrentReading { state.phase = .connected }
        if pendingReads.isEmpty, state.hasCurrentReading { readStarted = nil }
    }

    // Suspend toutes les lectures avant le sommeil et conserve seulement leur historique en mémoire.
    @objc private func willSleep() {
        suspended = true
        heartbeat?.invalidate()
        heartbeat = nil
        releaseDevice(phase: .sleeping)
    }

    // Le retour réutilise une connexion système existante, sans lancer de scan ou de nouvel appairage.
    @objc private func didWake() {
        guard !stopped else { return }
        suspended = false
        now = Date()
        retryAfter = .distantPast
        if closingDevice?.state == .disconnected { closingDevice = nil }
        state.phase = .waiting
        startHeartbeat()
        findConnectedKeyboard()
    }
}

// Tous les callbacks ci-dessous sont délivrés sur la file principale choisie au démarrage.
extension BatteryMonitor: @preconcurrency CBCentralManagerDelegate, @preconcurrency CBPeripheralDelegate {
    // Réagit aux permissions et à l’alimentation Bluetooth, sans afficher de dialogue système forcé.
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        guard !stopped else { return }
        if central.state != .poweredOn {
            releaseDevice(phase: .waiting)
            closingDevice = nil
        }
        guard !suspended else {
            state.phase = .sleeping
            return
        }
        switch central.state {
        case .poweredOn: findConnectedKeyboard()
        case .poweredOff: state.phase = .bluetoothOff
        case .unauthorized: state.phase = .permissionDenied
        case .unsupported: state.phase = .unavailable
        case .unknown, .resetting: state.phase = .waiting
        @unknown default: state.phase = .unavailable
        }
    }

    // Ne découvre que les services batterie, jamais les services HID ou les données de saisie.
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        guard peripheral === device, !suspended, !stopped else { return }
        connectionStarted = nil
        readStarted = Date()
        peripheral.discoverServices([serviceID])
    }

    // Une tentative échouée est bornée et ne déclenche aucun reset ou effacement d’appairage.
    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?)
    {
        guard peripheral === device else { return }
        failSession("Connexion du lecteur impossible. Vérifie la connexion Bluetooth du clavier.")
        // didFailToConnect signifie que cette connexion est déjà terminée.
        closingDevice = nil
    }

    // Ignore la clôture volontaire et rend les anciens niveaux indisponibles après une vraie déconnexion.
    func centralManager(
        _ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?
    ) {
        if peripheral === closingDevice {
            closingDevice = nil
            return
        }
        guard peripheral === device else { return }
        retryAfter = Date().addingTimeInterval(10)
        releaseDevice(phase: .disconnected)
        closingDevice = nil
    }

    // Accepte aussi un firmware avec la seule batterie principale, en laissant l’autre inconnue.
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard peripheral === device else { return }
        let services = (peripheral.services ?? []).filter { $0.uuid == serviceID }
        guard error == nil, !services.isEmpty, services.count <= 2 else {
            failSession("Les services batterie ne sont pas disponibles. Vérifie le firmware Bluetooth.")
            return
        }
        for service in services { peripheral.discoverCharacteristics([levelID], for: service) }
    }

    // Identifie chaque caractéristique sans utiliser l’ordre d’arrivée des services pour déterminer le côté.
    func peripheral(
        _ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?
    ) {
        guard peripheral === device else { return }
        let values = (service.characteristics ?? []).filter { $0.uuid == levelID }
        guard error == nil, values.count == 1, let value = values.first, value.properties.contains(.read)
        else {
            failSession("Le firmware ne fournit pas de niveau batterie lisible.")
            return
        }
        let id = ObjectIdentifier(value)
        guard levels[id] == nil else { return }
        channels[id] = channels.count
        levels[id] = value
        pendingReads.insert(id)
        peripheral.discoverDescriptors(for: value)
        peripheral.readValue(for: value)
    }

    // Lit uniquement le descripteur de présentation, sans modifier les abonnements GATT.
    func peripheral(
        _ peripheral: CBPeripheral, didDiscoverDescriptorsFor characteristic: CBCharacteristic, error: Error?
    ) {
        guard peripheral === device, channels[ObjectIdentifier(characteristic)] != nil else { return }
        guard error == nil,
            let descriptor = characteristic.descriptors?.first(where: { $0.uuid == presentationID })
        else {
            failSession("Les batteries ne peuvent pas être identifiées comme gauche et droite.")
            return
        }
        peripheral.readValue(for: descriptor)
    }

    // Associe principale et auxiliaire à gauche et droite après validation du format et de l’unicité.
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor descriptor: CBDescriptor, error: Error?) {
        guard peripheral === device, descriptor.uuid == presentationID,
            let characteristic = descriptor.characteristic,
            let channel = channels[ObjectIdentifier(characteristic)]
        else { return }
        guard error == nil, let data = descriptor.value as? Data else {
            failSession("L’identification des batteries a échoué.")
            return
        }
        do {
            try state.identify(channel: channel, presentation: data)
            publishReadiness()
        } catch {
            failSession("Le firmware présente des batteries inconnues ou des identifiants en doublon.")
        }
    }

    // Horodate une lecture reçue par le Mac sans prétendre connaître la date de mesure dans le clavier.
    func peripheral(
        _ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?
    ) {
        guard peripheral === device, let channel = channels[ObjectIdentifier(characteristic)] else { return }
        pendingReads.remove(ObjectIdentifier(characteristic))
        guard error == nil, let data = characteristic.value else {
            failSession("Une batterie n’a pas pu être lue. La lecture sera retentée.")
            return
        }
        do {
            now = Date()
            try state.receive(channel: channel, data: data, at: now)
            publishReadiness()
        } catch {
            failSession("Un niveau batterie invalide a été reçu.")
        }
    }

    // Invalide les identités GATT après un changement de firmware, avant toute nouvelle lecture.
    func peripheral(_ peripheral: CBPeripheral, didModifyServices invalidatedServices: [CBService]) {
        guard peripheral === device else { return }
        failSession("Les services du clavier ont changé. Le lecteur va les rechercher à nouveau.")
    }
}
