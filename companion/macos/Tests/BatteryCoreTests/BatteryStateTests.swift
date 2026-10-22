import CoreGraphics
import Foundation
import XCTest

@testable import BatteryCore

final class BatteryStateTests: XCTestCase {
    private let left = Data([0x04, 0, 0xAD, 0x27, 1, 0x06, 0x01])
    private let right = Data([0x04, 0, 0xAD, 0x27, 1, 0x08, 0x01])
    private let now = Date(timeIntervalSince1970: 1_000)

    // Les deux sources restent correctes même si droite et niveau arrivent avant la présentation gauche.
    func testOutOfOrderDiscoveryAndReadings() throws {
        var state = BatteryState()
        state.beginSession()
        try state.receive(channel: 0, data: Data([67]), at: now)
        try state.identify(channel: 0, presentation: right)
        try state.identify(channel: 1, presentation: left)
        try state.receive(channel: 1, data: Data([63]), at: now)
        state.phase = .connected

        XCTAssertEqual(state.readings[.left]?.level, 63)
        XCTAssertEqual(state.readings[.right]?.level, 67)
        XCTAssertTrue(state.isCurrent(.left, at: now))
        XCTAssertTrue(state.isCurrent(.right, at: now))
    }

    // Deux services prétendant être la gauche ne doivent pas écraser la première identification.
    func testDuplicateAndChangedIdentityAreRejected() throws {
        var state = BatteryState()
        try state.identify(channel: 0, presentation: left)
        XCTAssertThrowsError(try state.identify(channel: 1, presentation: left)) {
            XCTAssertEqual($0 as? BatteryDataError, .duplicateSide)
        }
        XCTAssertThrowsError(try state.identify(channel: 0, presentation: right)) {
            XCTAssertEqual($0 as? BatteryDataError, .duplicateSide)
        }
    }

    // Format, unité, namespace et source inconnue sont contrôlés, y compris les données tronquées.
    func testMalformedPresentationsAreRejected() {
        for size in 0..<7 {
            XCTAssertThrowsError(try BatteryProtocol.side(from: left.prefix(size)))
        }
        XCTAssertThrowsError(try BatteryProtocol.side(from: left + Data([0])))
        for index in 0..<5 {
            var bytes = left
            bytes[index] ^= 1
            XCTAssertThrowsError(try BatteryProtocol.side(from: bytes))
        }
        XCTAssertThrowsError(try BatteryProtocol.side(from: Data([4, 0, 0xAD, 0x27, 1, 8, 2]))) {
            XCTAssertEqual($0 as? BatteryDataError, .unknownSource)
        }
    }

    // Les bornes zéro et cent sont valides. Aucun niveau invalide ne détruit la dernière lecture.
    func testBatteryLevelBoundsAndInvalidUpdates() throws {
        var state = BatteryState()
        try state.identify(channel: 0, presentation: left)
        for level in [0, 100] {
            try state.receive(channel: 0, data: Data([UInt8(level)]), at: now)
            XCTAssertEqual(state.readings[.left]?.level, level)
        }
        for invalid in [Data(), Data([101]), Data([255]), Data([50, 0])] {
            XCTAssertThrowsError(try state.receive(channel: 0, data: invalid, at: now))
            XCTAssertEqual(state.readings[.left]?.level, 100)
            XCTAssertEqual(state.readings[.left]?.receivedAt, now)
        }
    }

    // La lecture devient ancienne exactement à l’échéance. Une horloge revenue en arrière ne la rajeunit pas.
    func testFreshnessBoundariesAndClockChange() throws {
        var state = try connectedState()
        XCTAssertTrue(state.isCurrent(.left, at: now.addingTimeInterval(89.999)))
        XCTAssertFalse(state.isCurrent(.left, at: now.addingTimeInterval(90)))
        XCTAssertFalse(state.isCurrent(.left, at: now.addingTimeInterval(-1)))
        try state.receive(channel: 0, data: Data([63]), at: now.addingTimeInterval(100))
        XCTAssertTrue(state.isCurrent(.left, at: now.addingTimeInterval(100)))
    }

    // Perte de connexion, sommeil et permissions ne doivent jamais laisser un niveau actif dans la barre.
    func testUnavailableStatesRetainHistoryWithoutShowingLiveLevels() throws {
        var state = try connectedState()
        let phases: [MonitorPhase] = [
            .waiting, .loading, .disconnected, .bluetoothOff, .permissionDenied,
            .unavailable, .ambiguous, .sleeping, .failed("lecture refusée"),
        ]
        for phase in phases {
            state.phase = phase
            XCTAssertEqual(state.connectionTitle, "Déconnecté", "\(phase)")
            XCTAssertFalse(state.isCurrent(.left, at: now), "\(phase)")
            XCTAssertFalse(state.menuTitle(at: now).contains("63"), "\(phase)")
            XCTAssertEqual(state.readings[.left]?.level, 63)
        }
        state.phase = .connected
        XCTAssertEqual(state.connectionTitle, "Connecté")
    }

    // Une reconnexion repart sans anciennes associations, même si les mêmes numéros de canaux sont réutilisés.
    func testNewSessionDoesNotReuseOldChannelIdentities() throws {
        var state = try connectedState()
        state.beginSession()
        XCTAssertFalse(state.hasCurrentReading)
        try state.receive(channel: 0, data: Data([40]), at: now.addingTimeInterval(1))
        state.phase = .connected
        XCTAssertFalse(state.isCurrent(.left, at: now.addingTimeInterval(1)))
        try state.identify(channel: 0, presentation: right)
        XCTAssertEqual(state.readings[.right]?.level, 40)
        XCTAssertEqual(state.readings[.left]?.level, 63)
        XCTAssertFalse(state.isCurrent(.left, at: now.addingTimeInterval(1)))
    }

    // Une moitié absente reste indisponible, sans zéro inventé et sans bloquer celle qui répond.
    func testPartialReadingKeepsMissingSideUnknown() throws {
        let state = try connectedState()
        XCTAssertTrue(state.hasCurrentReading)
        XCTAssertTrue(state.isCurrent(.left, at: now))
        XCTAssertNil(state.readings[.right])
        XCTAssertFalse(state.isCurrent(.right, at: now))
        XCTAssertEqual(state.menuLines(at: now), ["G 63 %", "D -"])
    }

    // Une valeur relue identique donne seulement une nouvelle date de réception, sans historique illimité.
    func testRepeatedValueUpdatesReceiptTime() throws {
        var state = try connectedState()
        try state.receive(channel: 0, data: Data([63]), at: now.addingTimeInterval(30))
        XCTAssertEqual(state.readings.count, 1)
        XCTAssertEqual(state.readings[.left]?.level, 63)
        XCTAssertEqual(state.readings[.left]?.receivedAt, now.addingTimeInterval(30))
    }

    // Une lecture non identifiée de l’ancienne session ne doit pas réapparaître après reconnexion.
    func testUnidentifiedPendingReadingIsDiscardedOnReconnect() throws {
        var state = BatteryState()
        state.beginSession()
        try state.receive(channel: 0, data: Data([80]), at: now)
        state.beginSession()
        try state.identify(channel: 0, presentation: left)
        XCTAssertNil(state.readings[.left])
        XCTAssertFalse(state.hasCurrentReading)
    }

    // Zéro reste une lecture valide, mais le tiret le masque sans annoncer de déconnexion.
    func testZeroLevelIsHiddenWithoutChangingTheReadingOrConnection() throws {
        for side in BatterySide.allCases {
            var state = try connectedState()
            try state.identify(channel: 1, presentation: right)
            try state.receive(channel: 1, data: Data([67]), at: now)
            let channel = side == .left ? 0 : 1
            let otherSide: BatterySide = side == .left ? .right : .left
            let otherIndication = state.indication(for: otherSide, at: now)

            try state.receive(channel: channel, data: Data([0]), at: now)
            XCTAssertEqual(state.readings[side]?.level, 0)
            XCTAssertTrue(state.isCurrent(side, at: now))
            XCTAssertEqual(state.phase, .connected)
            XCTAssertEqual(state.indication(for: side, at: now), .unavailable)
            XCTAssertEqual(state.indication(for: otherSide, at: now), otherIndication)
            XCTAssertTrue(state.menuLines(at: now).contains("\(side.shortName) -"))

            try state.receive(channel: channel, data: Data([1]), at: now)
            XCTAssertEqual(state.indication(for: side, at: now), .level(1))
        }
    }

    // Les symboles larges et hauts restent entiers, avec leurs proportions d’origine.
    func testChargeSymbolPreservesItsAspectRatioWithinOneRow() {
        let bounds = CGSize(width: 17, height: 9)
        for naturalSize in [
            CGSize(width: 30, height: 20), CGSize(width: 8, height: 20),
            CGSize(width: 40, height: 10),
        ] {
            let fitted = BatterySymbolLayout.fittedSize(naturalSize, within: bounds)
            XCTAssertGreaterThan(fitted.width, 0)
            XCTAssertGreaterThan(fitted.height, 0)
            XCTAssertLessThanOrEqual(fitted.width, bounds.width)
            XCTAssertLessThanOrEqual(fitted.height, bounds.height)
            XCTAssertEqual(
                fitted.width / fitted.height, naturalSize.width / naturalSize.height, accuracy: 0.000_001)
            XCTAssertTrue(fitted.width == bounds.width || fitted.height == bounds.height)
        }
    }

    // Une dimension invalide ne doit pas introduire de taille NaN ou infinie dans l’image du menu.
    func testChargeSymbolRejectsInvalidDimensions() {
        let valid = CGSize(width: 17, height: 9)
        for invalid in [
            CGSize.zero, CGSize(width: -1, height: 9),
            CGSize(width: CGFloat.infinity, height: 9), CGSize(width: 17, height: CGFloat.nan),
        ] {
            XCTAssertEqual(BatterySymbolLayout.fittedSize(invalid, within: valid), .zero)
            XCTAssertEqual(BatterySymbolLayout.fittedSize(valid, within: invalid), .zero)
        }
    }

    // Un niveau de cent pour cent devient une charge possible, sans altérer la donnée reçue.
    func testFullLevelIsOnlyAPossibleCharge() throws {
        var state = try connectedState()
        try state.receive(channel: 0, data: Data([99]), at: now)
        XCTAssertEqual(state.indication(for: .left, at: now), .level(99))
        try state.receive(channel: 0, data: Data([100]), at: now)
        XCTAssertEqual(state.indication(for: .left, at: now), .possibleCharge)
        XCTAssertEqual(state.readings[.left]?.level, 100)
        XCTAssertEqual(state.indication(for: .right, at: now), .unavailable)
        XCTAssertEqual(state.menuTitle(at: now), "G Charge possible\nD -")
    }

    // Une ancienne lecture de cent pour cent ne laisse jamais un indice de charge actif.
    func testOldFullLevelDoesNotIndicateCharge() throws {
        var state = try connectedState()
        try state.receive(channel: 0, data: Data([100]), at: now)
        XCTAssertEqual(state.indication(for: .left, at: now.addingTimeInterval(90)), .unavailable)
        XCTAssertEqual(state.menuLines(at: now.addingTimeInterval(90)), ["G -", "D -"])
        state.phase = .disconnected
        XCTAssertEqual(state.indication(for: .left, at: now), .unavailable)
    }

    // L’ordre vertical reste gauche puis droite, même quand la droite est découverte en premier.
    func testMenuLinesKeepLeftAboveRight() throws {
        var state = BatteryState()
        state.beginSession()
        try state.identify(channel: 0, presentation: right)
        try state.identify(channel: 1, presentation: left)
        try state.receive(channel: 0, data: Data([79]), at: now)
        try state.receive(channel: 1, data: Data([76]), at: now)
        state.phase = .connected
        XCTAssertEqual(state.menuLines(at: now), ["G 76 %", "D 79 %"])
        state.phase = .disconnected
        XCTAssertEqual(state.menuLines(at: now), ["G -", "D -"])
    }

    // Le nom de base et ses libellés personnalisés retrouvent le clavier sans indice de position fixe.
    func testConnectedKeyboardSelection() {
        XCTAssertEqual(ConnectedKeyboardChoice.select(names: [nil, "Autre"], targetName: "Clavier"), .none)
        XCTAssertEqual(
            ConnectedKeyboardChoice.select(names: ["Autre", "Clavier"], targetName: "Clavier"), .single(1))
        XCTAssertEqual(
            ConnectedKeyboardChoice.select(names: ["Clavier", "Clavier"], targetName: "Clavier"), .ambiguous)
        XCTAssertEqual(ConnectedKeyboardChoice.select(names: ["Clavier_Left"], targetName: "Clavier"), .none)
        for name in ["Clavier V1", "Clavier V2", "Clavier bureau"] {
            XCTAssertEqual(
                ConnectedKeyboardChoice.select(names: [nil, "Autre", name], targetName: "Clavier"), .single(2)
            )
        }
    }

    // Un préfixe ressemblant, un nom absent ou un suffixe vide ne reconnaît pas un autre périphérique.
    func testConnectedKeyboardRejectsUnrelatedOrEmptyNames() {
        XCTAssertEqual(
            ConnectedKeyboardChoice.select(
                names: [
                    nil, "ClavierV2", "Clavier_Left", "Autre Clavier V2", "clavier V2", "Clavier ",
                    "Clavier \t",
                ],
                targetName: "Clavier"), .none)
        XCTAssertEqual(ConnectedKeyboardChoice.select(names: ["", " V2"], targetName: ""), .none)
    }

    // Deux versions connectées restent ambiguës, même si l’une garde le nom exact de la configuration.
    func testConnectedKeyboardAliasesRemainAmbiguous() {
        for names in [["Clavier V1", "Clavier V2"], ["Clavier", "Clavier V2"], ["Clavier V2", "Clavier"]] {
            XCTAssertEqual(ConnectedKeyboardChoice.select(names: names, targetName: "Clavier"), .ambiguous)
        }
    }

    // Prépare une seule moitié identifiée pour les scénarios de disponibilité et de reconnexion.
    private func connectedState() throws -> BatteryState {
        var state = BatteryState()
        state.beginSession()
        try state.identify(channel: 0, presentation: left)
        try state.receive(channel: 0, data: Data([63]), at: now)
        state.phase = .connected
        return state
    }
}
