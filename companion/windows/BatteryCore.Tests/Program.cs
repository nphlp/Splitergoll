// Vérifie les règles applicatives sans API Windows, Bluetooth ou préférences réelles.
using Splitergoll.BatteryCore;

var now = DateTimeOffset.Parse("2026-01-01T00:00:00Z");
byte[] left = [4, 0, 0xAD, 0x27, 1, 6, 1];
byte[] right = [4, 0, 0xAD, 0x27, 1, 8, 1];
var cases = new List<(string Name, Action Test)>();
// Enregistre un comportement et rend l’échec indépendant des autres cas.
void Test(string name, Action test) => cases.Add((name, test));
// Refuse une condition fausse au lieu d’adapter les résultats au comportement courant.
void Check(bool condition) { if (!condition) throw new Exception("Assertion échouée."); }
// Exige le type d’erreur attendu pour les entrées malformées.
void Reject<T>(Action action) where T : Exception
{
    try { action(); } catch (T) { return; }
    throw new Exception($"Erreur {typeof(T).Name} attendue.");
}
// Crée une lecture déterministe sans horloge système ou donnée matérielle.
BatteryState WithReading(int level)
{
    var s = new BatteryState(); var token = s.BeginSession();
    s.Identify(token, 0, left); s.Receive(token, 0, [(byte)level], now);
    return s;
}

Test("identité gauche", () => Check(BatteryProtocol.DecodeSide(left) == Side.Left));
Test("identité droite", () => Check(BatteryProtocol.DecodeSide(right) == Side.Right));
Test("présentation trop courte", () => Reject<FormatException>(() => BatteryProtocol.DecodeSide(left[..6])));
Test("présentation trop longue", () => Reject<FormatException>(() => BatteryProtocol.DecodeSide([.. left, 0])));
for (int i = 0; i < 5; i++)
{
    int position = i;
    Test($"présentation corrompue {i}", () => { var bytes = left.ToArray(); bytes[position] ^= 1; Reject<FormatException>(() => BatteryProtocol.DecodeSide(bytes)); });
}
Test("source inconnue", () => Reject<FormatException>(() => BatteryProtocol.DecodeSide([4, 0, 0xAD, 0x27, 1, 7, 1])));
Test("niveau vide", () => Reject<FormatException>(() => BatteryProtocol.DecodeLevel([])));
Test("niveau multioctet", () => Reject<FormatException>(() => BatteryProtocol.DecodeLevel([1, 2])));
Test("niveau hors plage", () => Reject<FormatException>(() => BatteryProtocol.DecodeLevel([101])));
Test("zéro décodé", () => Check(BatteryProtocol.DecodeLevel([0]) == 0));
Test("cent décodé", () => Check(BatteryProtocol.DecodeLevel([100]) == 100));
Test("lecture partielle", () => { var s = WithReading(63); Check(s.Get(Side.Left, now).Level == 63 && s.Get(Side.Right, now).Kind == IndicationKind.Unavailable); });
Test("zéro masqué", () => Check(WithReading(0).Get(Side.Left, now).Kind == IndicationKind.Unavailable));
Test("cent ambigu", () => Check(WithReading(100).Get(Side.Left, now).Kind == IndicationKind.PossibleCharge));
Test("un pour cent visible", () => Check(WithReading(1).Get(Side.Left, now).Text == "1 %"));
Test("lecture fraîche", () => Check(WithReading(63).Get(Side.Left, now.AddSeconds(89)).Level == 63));
Test("expiration exacte", () => Check(WithReading(63).Get(Side.Left, now.AddSeconds(90)).Kind == IndicationKind.Unavailable));
Test("horloge reculée", () => Check(WithReading(63).Get(Side.Left, now.AddTicks(-1)).Kind == IndicationKind.Unavailable));
Test("déconnexion", () => { var s = WithReading(63); s.Disconnect(); Check(!s.Connected && s.Get(Side.Left, now).Kind == IndicationKind.Unavailable); });
Test("ancienne génération ignorée", () => { var s = new BatteryState(); var old = s.BeginSession(); s.BeginSession(); Check(!s.Identify(old, 0, left) && !s.Receive(old, 0, [55], now)); });
Test("identité en doublon", () => { var s = new BatteryState(); var token = s.BeginSession(); s.Identify(token, 0, left); Reject<FormatException>(() => s.Identify(token, 1, left)); });
Test("canal remplacé", () => { var s = new BatteryState(); var token = s.BeginSession(); s.Identify(token, 0, left); Reject<FormatException>(() => s.Identify(token, 0, right)); });
Test("lecture sans identité", () => { var s = new BatteryState(); var token = s.BeginSession(); Reject<InvalidOperationException>(() => s.Receive(token, 0, [50], now)); });
Test("lecture invalide préserve la précédente", () => { var s = new BatteryState(); var token = s.BeginSession(); s.Identify(token, 0, left); s.Receive(token, 0, [42], now); Reject<FormatException>(() => s.Receive(token, 0, [255], now)); Check(s.Get(Side.Left, now).Level == 42); });
Test("ordre des services inversé", () => { var s = new BatteryState(); var token = s.BeginSession(); s.Identify(token, 0, right); s.Identify(token, 1, left); s.Receive(token, 0, [12], now); s.Receive(token, 1, [91], now); Check(s.Get(Side.Left, now).Level == 91 && s.Get(Side.Right, now).Level == 12); });
var a = new KeyboardIdentity("a", "Splitergoll V1");
var b = new KeyboardIdentity("b", "Splitergoll V2");
Test("cible unique", () => { var s = new KeyboardSelection(); s.Update([a], "Splitergoll"); Check(s.Selected == a && s.Preferred is null); });
Test("ambiguïté initiale", () => { var s = new KeyboardSelection(); s.Update([a, b], "Splitergoll"); Check(s.NeedsChoice && s.Selected is null); });
Test("préférence conservée si absente", () => { var s = new KeyboardSelection(a); s.Update([b], "Splitergoll"); Check(s.Selected == a && !s.SelectedIsAvailable); });
Test("sélection explicite", () => { var s = new KeyboardSelection(); s.Update([a, b], "Splitergoll"); Check(s.Select("b") && s.Selected == b && s.Preferred == b); });
Test("cible inexistante refusée", () => { var s = new KeyboardSelection(); s.Update([a], "Splitergoll"); Check(!s.Select("z") && s.Selected == a); });
Test("renommage par identité", () => { var s = new KeyboardSelection(a); s.Update([new("a", "Mon clavier")], "Splitergoll"); Check(s.Preferred?.Name == "Mon clavier"); });
Test("nouveau clavier sans vol de sélection", () => { var s = new KeyboardSelection(); s.Update([a], "Splitergoll"); s.Update([a, b], "Splitergoll"); Check(s.Selected == a); });
Test("noms identiques restent distincts", () => { var s = new KeyboardSelection(); s.Update([a, new("b", a.Name)], "Splitergoll"); Check(s.Available.Count == 2 && s.NeedsChoice); });
Test("faux préfixes refusés", () => Check(!KeyboardSelection.Matches("SplitergollV2", "Splitergoll") && !KeyboardSelection.Matches("Splitergoll_Left", "Splitergoll")));
Test("suffixe vide refusé", () => Check(!KeyboardSelection.Matches("Splitergoll  ", "Splitergoll")));
Test("nom cible vide refusé", () => Check(!KeyboardSelection.Matches("Splitergoll", "")));
Test("préférence aller-retour", () => Check(PreferenceCodec.Decode(PreferenceCodec.Encode(a)) == a));
Test("préférence vide", () => Check(PreferenceCodec.Decode(PreferenceCodec.Encode(null)) is null));
Test("préférence corrompue", () => Check(PreferenceCodec.Decode("{invalid") is null));
Test("version inconnue", () => Check(PreferenceCodec.Decode("{\"Version\":2,\"Keyboard\":{\"Id\":\"a\",\"Name\":\"Splitergoll\"}}") is null));
Test("identité vide", () => Check(PreferenceCodec.Decode(PreferenceCodec.Encode(new("", "Splitergoll"))) is null));
Test("nom vide", () => Check(PreferenceCodec.Decode(PreferenceCodec.Encode(new("a", " "))) is null));
Test("préférence sans mesures", () => Check(!PreferenceCodec.Encode(a).Contains("Level", StringComparison.Ordinal)));
Test("pourcent si largeur suffisante", () => Check(TrayText.CanUsePercent(16, 15, 8, 7)));
Test("pourcent exclu sans écraser les chiffres", () => Check(!TrayText.CanUsePercent(16, 17, 8, 7)));
Test("hauteur insuffisante", () => Check(!TrayText.CanUsePercent(16, 15, 8, 9)));
Test("mesure invalide", () => Check(!TrayText.CanUsePercent(16, float.NaN, 8, 7)));
Test("format stable des niveaux", () => Check(TrayText.Format(new(IndicationKind.Level, 9), true) == "9%" && TrayText.Format(new(IndicationKind.Level, 90), false) == "90"));
Test("états spéciaux sans pourcent", () => Check(TrayText.Format(new(IndicationKind.Unavailable), true) == "-" && TrayText.Format(new(IndicationKind.PossibleCharge), true) == "ϟ"));
int failures = 0;
foreach (var test in cases)
{
    try { test.Test(); Console.WriteLine($"OK {test.Name}"); }
    catch (Exception error) { failures++; Console.Error.WriteLine($"ÉCHEC {test.Name}: {error.Message}"); }
}
Console.WriteLine($"{cases.Count - failures}/{cases.Count} tests réussis.");
return failures == 0 ? 0 : 1;
