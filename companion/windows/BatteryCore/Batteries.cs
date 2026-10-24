// Décode les données batterie et conserve une session indépendante des API Windows.
namespace Splitergoll.BatteryCore;

public enum Side { Left, Right }
public enum IndicationKind { Unavailable, Level, PossibleCharge }
public readonly record struct Indication(IndicationKind Kind, int Level = 0)
{
    public string Text => Kind switch
    {
        IndicationKind.Level => $"{Level} %",
        IndicationKind.PossibleCharge => "ϟ",
        _ => "-"
    };
}
public readonly record struct Reading(int Level, DateTimeOffset ReceivedAt);

public static class BatteryProtocol
{
    // Vérifie le format GATT et l’identité de la moitié, sans utiliser l’ordre des services.
    public static Side DecodeSide(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length != 7 || !bytes[..5].SequenceEqual<byte>([0x04, 0, 0xAD, 0x27, 1]))
            throw new FormatException("Présentation batterie invalide.");
        return (bytes[5] | bytes[6] << 8) switch
        {
            0x0106 => Side.Left,
            0x0108 => Side.Right,
            _ => throw new FormatException("Source batterie inconnue.")
        };
    }

    // Refuse les données absentes, multioctets et les pourcentages hors plage.
    public static int DecodeLevel(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length != 1 || bytes[0] > 100) throw new FormatException("Niveau batterie invalide.");
        return bytes[0];
    }
}

public sealed class BatteryState
{
    private readonly Dictionary<Side, Reading> readings = [];
    private readonly Dictionary<int, Side> channels = [];
    private long generation;
    public bool Connected { get; private set; }
    public const int StaleAfterSeconds = 90;

    // Commence une nouvelle génération et vide les mesures du précédent clavier.
    public long BeginSession()
    {
        generation++;
        readings.Clear();
        channels.Clear();
        Connected = false;
        return generation;
    }

    // Invalide immédiatement les réponses encore en vol de la session libérée.
    public void Disconnect() => BeginSession();

    // Accepte un seul canal par côté et refuse toute réponse issue d’une ancienne session.
    public bool Identify(long session, int channel, ReadOnlySpan<byte> presentation)
    {
        if (session != generation) return false;
        var side = BatteryProtocol.DecodeSide(presentation);
        if (channels.Any(pair => pair.Key != channel && pair.Value == side)
            || channels.TryGetValue(channel, out var existing) && existing != side)
            throw new FormatException("Identité batterie en doublon.");
        channels[channel] = side;
        return true;
    }

    // Ne publie que les lectures identifiées, valides et appartenant à la session courante.
    public bool Receive(long session, int channel, ReadOnlySpan<byte> value, DateTimeOffset receivedAt)
    {
        if (session != generation) return false;
        if (!channels.TryGetValue(channel, out var side)) throw new InvalidOperationException("Canal non identifié.");
        readings[side] = new Reading(BatteryProtocol.DecodeLevel(value), receivedAt);
        Connected = true;
        return true;
    }

    // Masque les valeurs périmées ou ambiguës selon les conventions de l’application Mac.
    public Indication Get(Side side, DateTimeOffset now)
    {
        if (!Connected || !readings.TryGetValue(side, out var value)) return new(IndicationKind.Unavailable);
        var age = now - value.ReceivedAt;
        if (age < TimeSpan.Zero || age >= TimeSpan.FromSeconds(StaleAfterSeconds)) return new(IndicationKind.Unavailable);
        return value.Level switch
        {
            0 => new(IndicationKind.Unavailable),
            100 => new(IndicationKind.PossibleCharge),
            _ => new(IndicationKind.Level, value.Level)
        };
    }
}
