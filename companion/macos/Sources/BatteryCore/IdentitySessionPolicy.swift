// Cycle du compagnon d’identité, indépendant du lecteur GATT et des préférences système.
import Foundation

public struct IdentitySessionPolicy: Sendable {
    public private(set) var paused = false
    public private(set) var sleeping = false
    private var retryAfter = Date.distantPast

    public init() {}

    // L’arrière-plan garde l’identité active, mais jamais sans clavier, pendant le sommeil ou après Arrêter.
    public func permitsStart(selected: UUID?, available: Bool, running: Bool, at date: Date) -> Bool {
        selected != nil && available && !running && !paused && !sleeping && date >= retryAfter
    }

    // Un échec du transport ne provoque pas une boucle de connexion rapide.
    public mutating func stopped(at date: Date) { retryAfter = date.addingTimeInterval(30) }
    // Un arrêt explicite reste respecté jusqu’à Relancer ou au choix d’un autre clavier.
    public mutating func pause() { paused = true }
    // La relance explicite ne contourne pas le sommeil de la machine.
    public mutating func resume() {
        paused = false
        retryAfter = .distantPast
    }
    // Le sommeil libère les échanges, sans assertion de réveil.
    public mutating func sleep() { sleeping = true }
    // Le retour de session autorise une nouvelle tentative dès que le lecteur voit le clavier.
    public mutating func wake() {
        sleeping = false
        retryAfter = .distantPast
    }
}
