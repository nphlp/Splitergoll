// Choisit le clavier suivi par identité système, sans toucher à la destination des frappes.
namespace Splitergoll.BatteryCore;

public sealed record KeyboardIdentity(string Id, string Name);

public static class PreferenceCodec
{
    private sealed record Document(int Version, KeyboardIdentity? Keyboard);

    // Refuse une préférence inconnue, vide ou malformée sans inventer d’identité de remplacement.
    public static KeyboardIdentity? Decode(string json)
    {
        try
        {
            var data = System.Text.Json.JsonSerializer.Deserialize<Document>(json);
            return data is { Version: 1, Keyboard: { } value }
                && !string.IsNullOrWhiteSpace(value.Id) && !string.IsNullOrWhiteSpace(value.Name)
                ? value : null;
        }
        catch (System.Text.Json.JsonException) { return null; }
    }

    // Versionne uniquement le choix de clavier, sans niveau batterie ou identifiant d’ordinateur.
    public static string Encode(KeyboardIdentity? keyboard)
        => System.Text.Json.JsonSerializer.Serialize(new Document(1, keyboard));
}

public sealed class KeyboardSelection
{
    public KeyboardIdentity? Preferred { get; private set; }
    public KeyboardIdentity? Selected { get; private set; }
    public IReadOnlyList<KeyboardIdentity> Available { get; private set; } = [];
    public bool NeedsChoice => Preferred is null && Selected is null && Available.Count > 1;
    public bool SelectedIsAvailable => Selected is not null && Available.Any(item => item.Id == Selected.Id);

    // Restaure seulement une préférence possédant une identité et un libellé valides.
    public KeyboardSelection(KeyboardIdentity? preferred = null)
    {
        if (preferred is not null && !string.IsNullOrWhiteSpace(preferred.Id) && !string.IsNullOrWhiteSpace(preferred.Name))
            Preferred = Selected = preferred;
    }

    // Accepte le nom configuré et ses suffixes personnalisés, sans faux préfixe.
    public static bool Matches(string name, string target) => !string.IsNullOrWhiteSpace(target)
        && (name == target || name.StartsWith(target + " ", StringComparison.Ordinal)
            && !string.IsNullOrWhiteSpace(name[(target.Length + 1)..]));

    // Une préférence absente reste sélectionnée, sans repli silencieux vers un autre clavier.
    public void Update(IEnumerable<KeyboardIdentity> candidates, string target)
    {
        Available = candidates.Where(item => Matches(item.Name, target) || item.Id == Preferred?.Id)
            .DistinctBy(item => item.Id).OrderBy(item => item.Name, StringComparer.Ordinal).ToArray();
        if (Preferred is not null)
        {
            Preferred = Available.FirstOrDefault(item => item.Id == Preferred.Id) ?? Preferred;
            Selected = Preferred;
        }
        else if (Selected is not null && Available.Any(item => item.Id == Selected.Id))
            Selected = Available.First(item => item.Id == Selected.Id);
        else Selected = Available.Count == 1 ? Available[0] : null;
    }

    // Mémorise uniquement une cible réellement présente dans l’inventaire courant.
    public bool Select(string id)
    {
        var choice = Available.FirstOrDefault(item => item.Id == id);
        if (choice is null) return false;
        Preferred = Selected = choice;
        return true;
    }
}

public static class TrayText
{
    // Garder le symbole pour toute la plage impose de mesurer la largeur maximale affichable.
    public static bool CanUsePercent(float availableWidth, float measuredMaximum, float availableHeight, float measuredHeight)
        => availableWidth > 0 && availableHeight > 0 && measuredMaximum > 0 && measuredHeight > 0
            && measuredMaximum <= availableWidth && measuredHeight <= availableHeight;

    // Le même choix typographique s’applique aux deux lignes, sans variation selon le niveau.
    public static string Format(Indication indication, bool withPercent) => indication.Kind == IndicationKind.Level
        ? indication.Level + (withPercent ? "%" : "") : indication.Text;
}
