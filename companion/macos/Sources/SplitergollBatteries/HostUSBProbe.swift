// Utilise seulement la collection USB vendor du diagnostic, jamais les événements clavier.
import BatteryCore
import Foundation
import IOKit.hid

final class HostUSBProbe: @unchecked Sendable {
    // Tous les handles et appels synchrones IOKit restent sur cette seule file, hors interface.
    private let queue = DispatchQueue(label: "Splitergoll.identity.usb")

    // Lit d’abord le jeton de démarrage commun, sans annoncer l’identité aux autres claviers USB.
    func exchange(
        boot: Data, identity: UUID,
        completion: @escaping @Sendable (HostIdentityProtocol.Report?, String) -> Void
    ) {
        queue.async {
            let manager = IOHIDManagerCreate(kCFAllocatorDefault, IOOptionBits(kIOHIDOptionsTypeNone))
            IOHIDManagerSetDeviceMatching(
                manager,
                [
                    kIOHIDDeviceUsagePageKey: HostIdentityProtocol.usagePage,
                    kIOHIDDeviceUsageKey: HostIdentityProtocol.usage,
                    kIOHIDTransportKey: "USB",
                ] as CFDictionary)
            guard IOHIDManagerOpen(manager, IOOptionBits(kIOHIDOptionsTypeNone)) == kIOReturnSuccess else {
                completion(nil, "USB inaccessible")
                return
            }
            defer { IOHIDManagerClose(manager, IOOptionBits(kIOHIDOptionsTypeNone)) }
            let devices = IOHIDManagerCopyDevices(manager) as? Set<IOHIDDevice> ?? []
            var matching: [IOHIDDevice] = []
            var unavailable: String?
            for device in devices {
                guard IOHIDDeviceOpen(device, IOOptionBits(kIOHIDOptionsTypeNone)) == kIOReturnSuccess else {
                    unavailable = "Interface USB détectée, ouverture refusée"
                    continue
                }
                defer { IOHIDDeviceClose(device, IOOptionBits(kIOHIDOptionsTypeNone)) }
                do {
                    if try Self.read(device).boot == boot { matching.append(device) }
                } catch let error as NSError where error.domain == "SplitergollUSB" {
                    unavailable = "Lecture USB refusée (\(error.code))"
                } catch { unavailable = "Interface USB détectée, trame incompatible" }
            }
            guard matching.count == 1, let device = matching.first else {
                let absent =
                    unavailable
                    ?? (devices.isEmpty ? "USB du clavier absent" : "Autre clavier USB, aucune annonce")
                completion(nil, matching.isEmpty ? absent : "USB ambigu, aucune annonce")
                return
            }
            guard IOHIDDeviceOpen(device, IOOptionBits(kIOHIDOptionsTypeNone)) == kIOReturnSuccess else {
                completion(nil, "USB inaccessible")
                return
            }
            defer { IOHIDDeviceClose(device, IOOptionBits(kIOHIDOptionsTypeNone)) }
            do {
                let challenge = try Self.read(device)
                guard challenge.boot == boot else {
                    completion(nil, "Le clavier USB a redémarré")
                    return
                }
                let message = try challenge.announcement(identity: identity)
                let result = message.withUnsafeBytes {
                    IOHIDDeviceSetReport(
                        device, kIOHIDReportTypeFeature, 0, $0.bindMemory(to: UInt8.self).baseAddress!,
                        message.count)
                }
                guard result == kIOReturnSuccess else {
                    completion(nil, "Annonce USB refusée (\(result))")
                    return
                }
                let report = try Self.read(device)
                guard report.boot == challenge.boot, report.nonce == challenge.nonce else {
                    completion(nil, "Contexte USB renouvelé, nouvelle tentative nécessaire")
                    return
                }
                completion(report, report.usbClaimed ? "Identité annoncée" : "Annonce USB en attente")
            } catch { completion(nil, "Réponse USB non valide") }
        }
    }

    // La lecture ne reçoit aucun caractère tapé et refuse les réponses incomplètes.
    private static func read(_ device: IOHIDDevice) throws -> HostIdentityProtocol.Report {
        var bytes = [UInt8](repeating: 0, count: HostIdentityProtocol.size)
        var length = bytes.count
        let result = IOHIDDeviceGetReport(device, kIOHIDReportTypeFeature, 0, &bytes, &length)
        guard result == kIOReturnSuccess else {
            throw NSError(domain: "SplitergollUSB", code: Int(result))
        }
        guard length == bytes.count else {
            throw HostIdentityProtocol.Failure.malformed
        }
        return try HostIdentityProtocol.Report(Data(bytes), transport: .usb)
    }
}
