// Lance un compagnon portable à instance unique, avec démonstration explicitement sans Bluetooth.
using System.Reflection;

namespace Splitergoll;

internal static class Program
{
    // Le manifeste demande les droits utilisateur ordinaires, jamais une élévation administrateur.
    [STAThread]
    private static void Main(string[] args)
    {
        bool demo = args.Length == 1 && args[0] == "--demo";
        if (args.Length > 0 && !demo) { MessageBox.Show("Usage : Splitergoll.exe [--demo]", "Splitergoll"); return; }
        using var instance = new Mutex(true, @"Local\SplitergollBatteries", out bool created);
        if (!created) { MessageBox.Show("Splitergoll est déjà ouvert près de l’horloge.", "Splitergoll"); return; }
        ApplicationConfiguration.Initialize();
        string? target = Assembly.GetExecutingAssembly().GetCustomAttributes<AssemblyMetadataAttribute>()
            .FirstOrDefault(attribute => attribute.Key == "KeyboardName")?.Value;
        if (string.IsNullOrWhiteSpace(target)) { MessageBox.Show("Nom Bluetooth absent du build.", "Splitergoll"); return; }
        using var app = new TrayApplication(target, demo);
        Application.Run(app);
    }
}
