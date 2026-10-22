// Conserve les proportions du symbole de charge dans l’espace disponible sur une ligne.
import CoreGraphics

public enum BatterySymbolLayout {
    // Ajuste les deux dimensions avec le même facteur, sans étirer ni déborder du cadre.
    public static func fittedSize(_ naturalSize: CGSize, within bounds: CGSize) -> CGSize {
        let dimensions = [naturalSize.width, naturalSize.height, bounds.width, bounds.height]
        guard dimensions.allSatisfy({ $0.isFinite && $0 > 0 }) else { return .zero }

        let scale = min(bounds.width / naturalSize.width, bounds.height / naturalSize.height)
        return CGSize(width: naturalSize.width * scale, height: naturalSize.height * scale)
    }
}
