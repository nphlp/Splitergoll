// Diagnostic opt-in indépendant des batteries et de la destination des frappes.
import AppKit
import BatteryCore
import CoreBluetooth
import Foundation

@MainActor
final class HostIdentityMonitor: NSObject, ObservableObject, @preconcurrency CBCentralManagerDelegate {
    @Published private(set) var running = false
    @Published private(set) var usbText = "Non testé"
    @Published private(set) var bluetoothText = "Non testé"
    @Published private(set) var now = Date()
    @Published private(set) var usb: HostIdentityProtocol.Observation?
    @Published private(set) var ble: HostIdentityProtocol.Observation?
    @Published private(set) var runtime: KeyboardRuntime?
    var verdict: String { HostIdentityProtocol.verdict(usb: usb, ble: ble, now: now) }
    var currentRuntime: KeyboardRuntime? {
        runtime.flatMap { $0.isCurrent(boot: ble?.report.boot, at: now) ? $0 : nil }
    }

    private var central: CBCentralManager?
    private var device: CBPeripheral?
    private var proxy: IdentityPeripheralDelegate?
    private var characteristic: CBCharacteristic?
    private var runtimeCharacteristic: CBCharacteristic?
    private var identity: UUID?
    private var selected: UUID?
    private var timer: Timer?
    private var generation: UInt64 = 0
    private var challenge: HostIdentityProtocol.Report?
    private var pendingSince: Date?
    private var waitingForStatus = false
    private var usbBusy = false
    private let usbProbe = HostUSBProbe()
    private var policy = IdentitySessionPolicy()

    // Le compagnon renouvelle l’identité en arrière-plan pour maintenir la sortie USB autorisée.
    func start(selected: UUID?) {
        stop()
        policy.resume()
        guard !policy.sleeping else { return }
        guard let selected else {
            bluetoothText = "Choisir d’abord le clavier suivi"
            return
        }
        do {
            let directory = try FileManager.default.url(
                for: .applicationSupportDirectory, in: .userDomainMask,
                appropriateFor: nil, create: false
            ).appendingPathComponent("Splitergoll", isDirectory: true)
            identity = try HostIdentityFile.loadOrCreate(
                at: directory.appendingPathComponent("host-identity.json"))
        } catch {
            policy.stopped(at: Date())
            bluetoothText = "Identité locale indisponible, aucun échange effectué"
            return
        }
        self.selected = selected
        running = true
        bluetoothText = "Recherche du clavier déjà connecté"
        central = CBCentralManager(
            delegate: self, queue: .main, options: [CBCentralManagerOptionShowPowerAlertKey: false])
        let timer = Timer(timeInterval: 5, repeats: true) { [weak self] _ in
            Task { @MainActor in self?.tick() }
        }
        timer.tolerance = 0.25
        RunLoop.main.add(timer, forMode: .common)
        self.timer = timer
    }

    // Annule les générations en vol. Le firmware expire seul les annonces après quinze secondes.
    func stop() {
        generation &+= 1
        running = false
        timer?.invalidate()
        timer = nil
        device?.delegate = nil
        if let device { central?.cancelPeripheralConnection(device) }
        central?.delegate = nil
        central = nil
        device = nil
        proxy = nil
        characteristic = nil
        runtimeCharacteristic = nil
        runtime = nil
        challenge = nil
        pendingSince = nil
        waitingForStatus = false
        usb = nil
        ble = nil
        now = Date()
        policy.stopped(at: now)
        usbText = "Non testé"
        bluetoothText = "Arrêté"
        // Ne pas remettre usbBusy à zéro : un appel IOKit synchrone peut encore appartenir à l’ancienne génération.
    }

    // La pause n’empêche pas le sommeil du Mac et n’entretient pas d’annonces après sa reprise.
    func pauseForSleep() {
        policy.sleep()
        stop()
    }

    // L’arrêt manuel ne doit pas être annulé par le battement de lecture des batteries.
    func pauseByUser() {
        policy.pause()
        stop()
    }

    // Le lecteur de batteries signalera le clavier quand sa reconnexion sera terminée.
    func resumeAfterSleep() { policy.wake() }

    // Fermer la fenêtre ne modifie pas ce cycle. Aucun scan ou changement de destination n’est envoyé par l’app.
    func maintain(selected: UUID?, available: Bool, at date: Date = Date()) {
        followSelection(selected)
        if policy.permitsStart(selected: selected, available: available, running: running, at: date) {
            start(selected: selected)
        }
    }

    // Une sélection automatique ou explicite différente ne retargete jamais des annonces déjà en cours.
    func followSelection(_ id: UUID?) {
        if selected != id {
            stop()
            selected = id
            policy.resume()
        }
    }

    // Réessaie les lectures sans chevauchement et refuse une session BLE bloquée.
    private func tick() {
        guard running else { return }
        now = Date()
        if let pendingSince, now.timeIntervalSince(pendingSince) >= 15 {
            stop()
            bluetoothText = "Bluetooth sans réponse, relancer le diagnostic"
            return
        }
        if let device, let characteristic, pendingSince == nil {
            challenge = nil
            waitingForStatus = false
            pendingSince = now
            device.readValue(for: characteristic)
        } else if device == nil {
            connectKnown()
        }
    }

    // Le même clavier est retrouvé par son UUID CoreBluetooth local, sans scan ni nouvel appairage.
    private func connectKnown() {
        guard running, let central, central.state == .poweredOn, device == nil, let selected else { return }
        guard
            let peripheral = central.retrieveConnectedPeripherals(withServices: [CBUUID(string: "180F")])
                .first(where: { $0.identifier == selected })
        else {
            bluetoothText = "Clavier suivi non connecté"
            return
        }
        device = peripheral
        let proxy = IdentityPeripheralDelegate(owner: self, generation: generation)
        self.proxy = proxy
        peripheral.delegate = proxy
        pendingSince = Date()
        central.connect(peripheral, options: nil)
    }

    // Une ancienne découverte, écriture ou lecture ne peut se rattacher à une nouvelle sélection.
    func accepts(_ peripheral: CBPeripheral, generation: UInt64) -> Bool {
        running && self.generation == generation && peripheral === device
    }

    // Démarre seulement après autorisation système Bluetooth déjà disponible.
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        guard central === self.central, running else { return }
        if central.state == .poweredOn {
            connectKnown()
        } else if central.state == .unknown {
            bluetoothText = "Initialisation Bluetooth"
        } else {
            stop()
            bluetoothText = "Bluetooth indisponible, relancer le diagnostic"
        }
    }

    // Découvre le seul service diagnostique, sans toucher au lecteur BAS existant.
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        guard central === self.central, peripheral === device, running else { return }
        peripheral.discoverServices([CBUUID(string: HostIdentityProtocol.service)])
    }

    // Une perte de lien invalide immédiatement les observations avant toute nouvelle tentative.
    func centralManager(
        _ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?
    ) {
        guard central === self.central, peripheral === device else { return }
        stop()
        bluetoothText = "Clavier déconnecté, relancer le diagnostic"
    }

    // Un refus de connexion ne déclenche ni appairage ni réinitialisation.
    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?)
    {
        guard central === self.central, peripheral === device else { return }
        stop()
        bluetoothText = "Lecteur Bluetooth indisponible"
    }

    // Refuse les anciens firmwares et les services en doublon.
    func services(_ peripheral: CBPeripheral, error: Error?) {
        guard error == nil,
            let services = peripheral.services?.filter({
                $0.uuid == CBUUID(string: HostIdentityProtocol.service)
            }),
            services.count == 1, let service = services.first
        else {
            stop()
            bluetoothText = "Firmware diagnostic identity requis"
            return
        }
        peripheral.discoverCharacteristics(
            [
                CBUUID(string: HostIdentityProtocol.characteristic),
                CBUUID(string: KeyboardRuntime.characteristic),
            ],
            for: service)
    }

    // Le mode avec réponse est obligatoire pour savoir si l’annonce est acceptée.
    func characteristics(_ peripheral: CBPeripheral, service: CBService, error: Error?) {
        let values =
            service.characteristics?.filter { $0.uuid == CBUUID(string: HostIdentityProtocol.characteristic) }
            ?? []
        guard error == nil, values.count == 1, let value = values.first,
            value.properties.contains([.read, .write]),
            peripheral.maximumWriteValueLength(for: .withResponse) >= HostIdentityProtocol.size
        else {
            stop()
            bluetoothText = "Service d’identité incompatible"
            return
        }
        characteristic = value
        let runtimeValues =
            service.characteristics?.filter {
                $0.uuid == CBUUID(string: KeyboardRuntime.characteristic) && $0.properties.contains(.read)
            } ?? []
        runtimeCharacteristic = runtimeValues.count == 1 ? runtimeValues.first : nil
        peripheral.readValue(for: value)
    }

    // Valide le challenge, annonce l’identité puis relit les masques publics de cette même session.
    func received(_ peripheral: CBPeripheral, value: CBCharacteristic, error: Error?) {
        if value === runtimeCharacteristic {
            receivedRuntime(value.value, error: error)
            return
        }
        guard value === characteristic else { return }
        if let error {
            stop()
            bluetoothText = "Lecture Bluetooth refusée (\(Self.errorLabel(error)))"
            return
        }
        do {
            guard error == nil, let data = value.value, let identity else {
                throw HostIdentityProtocol.Failure.malformed
            }
            let report = try HostIdentityProtocol.Report(data, transport: .bluetooth)
            if waitingForStatus {
                guard let challenge, report.boot == challenge.boot, report.nonce == challenge.nonce,
                    report.claimed & (UInt8(1) << report.slot) != 0
                else { throw HostIdentityProtocol.Failure.malformed }
                ble = .init(report: report, received: Date())
                bluetoothText = "Identité annoncée"
                pendingSince = nil
                waitingForStatus = false
                probeUSB(boot: report.boot, identity: identity)
                if let runtimeCharacteristic {
                    pendingSince = Date()
                    peripheral.readValue(for: runtimeCharacteristic)
                }
            } else {
                challenge = report
                pendingSince = Date()
                peripheral.writeValue(
                    try report.announcement(identity: identity), for: value, type: .withResponse)
            }
        } catch {
            stop()
            bluetoothText = "Réponse Bluetooth refusée ou non valide"
        }
    }

    // Une lecture optionnelle invalide n’empêche pas le diagnostic des anciennes versions du firmware.
    func receivedRuntime(_ data: Data?, error: Error?, at date: Date = Date()) {
        pendingSince = nil
        guard error == nil, let data, let report = try? KeyboardRuntime(data, received: date),
            report.isCurrent(boot: ble?.report.boot, at: date)
        else {
            runtime = nil
            return
        }
        runtime = report
        now = date
    }

    // Les états copiables ne contiennent aucune identité, aucun jeton ni contenu de saisie.
    var diagnosticText: String {
        "OS actif : \(currentRuntime?.os.title ?? "Non communiqué")\nSaisie : \(currentRuntime?.transport.title ?? "Non communiquée")\nUSB : \(usbText)\nBluetooth : \(bluetoothText)\nRésultat : \(verdict)\nLa sortie de saisie est choisie par le clavier. Aucun identifiant exporté."
    }

    // Une écriture confirmée est suivie d’une lecture, sans supposer que le firmware l’a mémorisée.
    func written(_ peripheral: CBPeripheral, value: CBCharacteristic, error: Error?) {
        guard value === characteristic else { return }
        guard error == nil else {
            stop()
            bluetoothText = "Annonce Bluetooth refusée (\(Self.errorLabel(error!)))"
            return
        }
        waitingForStatus = true
        peripheral.readValue(for: value)
    }

    // Seuls domaine normalisé et code sont copiables, jamais les userInfo ou identités du système.
    static func errorLabel(_ error: Error) -> String {
        let native = error as NSError
        return "\(native.domain == CBATTErrorDomain ? "ATT" : "BLE") \(native.code)"
    }

    // Le travail USB ne bloque pas l’interface et sa réponse porte la génération qui l’a demandé.
    private func probeUSB(boot: Data, identity: UUID) {
        guard !usbBusy else { return }
        usbBusy = true
        usbText = "Lecture USB en cours"
        let ticket = generation
        usbProbe.exchange(boot: boot, identity: identity) { [weak self] report, text in
            Task { @MainActor in
                guard let self else { return }
                self.usbBusy = false
                guard self.running, self.generation == ticket else { return }
                self.usb = report.map { .init(report: $0, received: Date()) }
                self.usbText = text
                self.now = Date()
            }
        }
    }
}

@MainActor
private final class IdentityPeripheralDelegate: NSObject, @preconcurrency CBPeripheralDelegate {
    private weak var owner: HostIdentityMonitor?
    private let generation: UInt64
    // Le proxy ne survit pas comme propriétaire du lecteur et ne change jamais de génération.
    init(owner: HostIdentityMonitor, generation: UInt64) {
        self.owner = owner
        self.generation = generation
    }
    // Tous les callbacks passent la même garde avant de modifier le diagnostic.
    private func monitor(_ p: CBPeripheral) -> HostIdentityMonitor? {
        guard let owner, owner.accepts(p, generation: generation) else { return nil }
        return owner
    }
    // Transmet la découverte sécurisée de service de cette session.
    func peripheral(_ p: CBPeripheral, didDiscoverServices error: Error?) {
        monitor(p)?.services(p, error: error)
    }
    // Transmet les caractéristiques du service dédié.
    func peripheral(_ p: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        monitor(p)?.characteristics(p, service: service, error: error)
    }
    // Transmet la lecture du challenge ou de son résultat.
    func peripheral(_ p: CBPeripheral, didUpdateValueFor value: CBCharacteristic, error: Error?) {
        monitor(p)?.received(p, value: value, error: error)
    }
    // Transmet uniquement la confirmation d’écriture de la génération courante.
    func peripheral(_ p: CBPeripheral, didWriteValueFor value: CBCharacteristic, error: Error?) {
        monitor(p)?.written(p, value: value, error: error)
    }
    // Un changement de services invalide les observations, sans conserver une fausse correspondance.
    func peripheral(_ p: CBPeripheral, didModifyServices invalidatedServices: [CBService]) {
        monitor(p)?.stop()
    }
}
