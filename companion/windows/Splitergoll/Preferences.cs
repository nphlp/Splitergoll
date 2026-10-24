// Stocke le seul clavier suivi dans le profil utilisateur, sans identifiant hôte partagé.
using System.Text.Json;
using Splitergoll.BatteryCore;

namespace Splitergoll;

internal static class Preferences
{
    private static readonly string FileName = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Splitergoll", "settings.json");

    // Une préférence absente ou malformée ne doit jamais sélectionner un autre clavier arbitrairement.
    public static KeyboardIdentity? Load()
    {
        try
        {
            if (!File.Exists(FileName)) return null;
            return PreferenceCodec.Decode(File.ReadAllText(FileName));
        }
        catch (Exception error) when (error is IOException or UnauthorizedAccessException or JsonException) { return null; }
    }

    // Remplace atomiquement le fichier après écriture, sans stocker de mesures batterie ou de clés.
    public static void Save(KeyboardIdentity? keyboard)
    {
        Directory.CreateDirectory(Path.GetDirectoryName(FileName)!);
        string temporary = FileName + "." + Guid.NewGuid().ToString("N") + ".tmp";
        try { File.WriteAllText(temporary, PreferenceCodec.Encode(keyboard)); File.Move(temporary, FileName, true); }
        finally { if (File.Exists(temporary)) File.Delete(temporary); }
    }
}
