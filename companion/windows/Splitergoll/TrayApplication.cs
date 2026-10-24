// Coordonne les vues Windows et le lecteur sans maintenir une console ou empêcher la veille.
using Microsoft.Win32;
using Splitergoll.BatteryCore;

namespace Splitergoll;

internal sealed class TrayApplication : ApplicationContext
{
    private BatteryMonitor monitor;
    private bool demo;
    private readonly string target;
    private readonly TrayIcon tray = new();
    private readonly BatteryWindow popup = new(true);
    private readonly BatteryWindow window = new(false);
    private readonly ContextMenuStrip menu = new();
    private readonly ToolTip hints = new();
    private readonly System.Windows.Forms.Timer timer = new() { Interval = 5000 };
    private readonly Control dispatcher = new();
    private bool stopped;
    private string? iconKey;

    // Le premier battement démarre sur la boucle graphique, sans créer de service système.
    public TrayApplication(string target, bool demo)
    {
        this.demo = demo;
        this.target = target;
        monitor = new(target, demo);
        _ = dispatcher.Handle;
        tray.Click += _ => { if (popup.Visible) popup.Hide(); else { RefreshViews(); popup.ShowNear(tray.Bounds); } };
        tray.RightClick += point => { RefreshMenu(); menu.Show(point); };
        popup.OpenRequested += OpenWindow;
        timer.Tick += Tick;
        SystemEvents.PowerModeChanged += PowerChanged;
        SystemEvents.UserPreferenceChanged += AppearanceChanged;
        SystemEvents.DisplaySettingsChanged += AppearanceChanged;
        Application.Idle += FirstTick;
        RefreshViews(); timer.Start();
    }

    // Ouvre la même fenêtre compacte depuis le panneau ou le menu de l’icône.
    private void OpenWindow() { RefreshViews(); window.ShowNear(tray.Bounds); }

    // La première lecture ne s’exécute qu’une fois, après installation du contexte WinForms.
    private void FirstTick(object? sender, EventArgs args) { Application.Idle -= FirstTick; Tick(sender, args); }

    // Attend les lectures sans bloquer l’interface, avec erreurs présentées sans dialogue répétitif.
    private async void Tick(object? sender, EventArgs args)
    {
        if (stopped) return;
        try { await monitor.RefreshAsync(); }
        catch (Exception error)
        {
            timer.Stop();
            MessageBox.Show("Le lecteur s’est arrêté : " + error.Message, "Splitergoll", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
        if (!stopped) RefreshViews();
    }

    // Redessine seulement lorsque valeurs, thème ou définition changent, sans fuite de handles GDI.
    private void RefreshViews()
    {
        string name = monitor.Selection.Selected?.Name ?? (monitor.Selection.NeedsChoice ? "Choisir un clavier" : "Splitergoll");
        popup.UpdateState(name, monitor.State, demo, hints); window.UpdateState(name, monitor.State, demo, hints);
        var now = DateTimeOffset.UtcNow;
        var l = monitor.State.Get(Side.Left, now); var r = monitor.State.Get(Side.Right, now);
        string Value(Indication indication) => indication.Kind == IndicationKind.PossibleCharge ? "charge possible" : indication.Text;
        string tip = $"{name}\nGauche : {Value(l)}\nDroite : {Value(r)}";
        if (monitor.Error is not null) tip += "\nLecture indisponible, nouvelle tentative automatique.";
        int size = tray.PixelSize;
        string key = $"{size}:{TrayRenderer.DarkTaskbar()}:{l}:{r}:{tip}";
        if (key != iconKey) { tray.Update(TrayRenderer.Draw(size, l, r), tip); iconKey = key; }
    }

    // Reconstruit le menu sur ouverture pour ne pas le modifier pendant le choix de l’utilisateur.
    private void RefreshMenu()
    {
        foreach (var item in menu.Items.Cast<ToolStripItem>().ToArray()) item.Dispose();
        menu.Items.Clear();
        menu.Items.Add(monitor.State.Connected ? "Connecté" : "Déconnecté").Enabled = false;
        var keyboards = new ToolStripMenuItem("Clavier suivi");
        foreach (var candidate in monitor.Selection.Available)
        {
            string label = candidate.Name;
            if (monitor.Selection.Available.Count(item => item.Name == label) > 1) label += " · " + candidate.Id[^Math.Min(12, candidate.Id.Length)..];
            var item = new ToolStripMenuItem(label) { Checked = monitor.Selection.Selected?.Id == candidate.Id, Enabled = !demo };
            item.Click += (_, _) =>
            {
                try { monitor.Select(candidate.Id); RefreshViews(); Tick(null, EventArgs.Empty); }
                catch (IOException error) { MessageBox.Show(error.Message, "Préférence non sauvegardée"); }
                catch (UnauthorizedAccessException error) { MessageBox.Show(error.Message, "Préférence non sauvegardée"); }
            };
            keyboards.DropDownItems.Add(item);
        }
        if (monitor.Selection.Preferred is { } absent && !monitor.Selection.SelectedIsAvailable)
            keyboards.DropDownItems.Add(new ToolStripMenuItem(absent.Name + " (absent)") { Checked = true, Enabled = false });
        if (keyboards.DropDownItems.Count == 0) keyboards.DropDownItems.Add("Aucun clavier connecté").Enabled = false;
        menu.Items.Add(keyboards);
        var startup = new ToolStripMenuItem("Lancer avec Windows") { Checked = LaunchAtLogin.Enabled, Enabled = !demo };
        startup.Click += (_, _) =>
        {
            try { LaunchAtLogin.SetEnabled(!LaunchAtLogin.Enabled); }
            catch (Exception error) when (error is UnauthorizedAccessException or IOException or System.Security.SecurityException)
            { MessageBox.Show("Le réglage n’a pas été modifié : " + error.Message, "Splitergoll"); }
        };
        menu.Items.Add(startup);
        var demonstration = new ToolStripMenuItem("Démonstration sans Bluetooth") { Checked = demo };
        demonstration.Click += (_, _) =>
        {
            monitor.Dispose(); demo = !demo; monitor = new BatteryMonitor(target, demo);
            iconKey = null; RefreshViews(); Tick(null, EventArgs.Empty);
        };
        menu.Items.Add(demonstration);
        menu.Items.Add(new ToolStripSeparator());
        menu.Items.Add("Ouvrir l’app", null, (_, _) => OpenWindow());
        menu.Items.Add("Quitter l’app", null, (_, _) => ExitThread());
    }

    // Les événements système sont transférés vers la file graphique avant tout changement de session.
    private void PowerChanged(object sender, PowerModeChangedEventArgs args)
    {
        if (stopped || dispatcher.IsDisposed) return;
        dispatcher.BeginInvoke(() =>
        {
            if (stopped) return;
            if (args.Mode == PowerModes.Suspend) { timer.Stop(); monitor.SetSuspended(true); }
            else if (args.Mode == PowerModes.Resume) { monitor.SetSuspended(false); timer.Start(); Tick(null, EventArgs.Empty); }
            RefreshViews();
        });
    }

    // Le prochain dessin tient compte de la nouvelle définition ou du contraste de la barre.
    private void AppearanceChanged(object? sender, EventArgs args)
    {
        if (!stopped && !dispatcher.IsDisposed) dispatcher.BeginInvoke(() => { if (!stopped) { iconKey = null; RefreshViews(); } });
    }

    // Quitter détruit toutes les seules ressources de l’app, sans interrompre la saisie Bluetooth.
    protected override void ExitThreadCore()
    {
        stopped = true;
        timer.Stop(); Application.Idle -= FirstTick;
        SystemEvents.PowerModeChanged -= PowerChanged;
        SystemEvents.UserPreferenceChanged -= AppearanceChanged;
        SystemEvents.DisplaySettingsChanged -= AppearanceChanged;
        monitor.Dispose(); tray.Dispose(); popup.Dispose(); window.Dispose(); menu.Dispose(); hints.Dispose(); timer.Dispose(); dispatcher.Dispose();
        base.ExitThreadCore();
    }
}

internal static class LaunchAtLogin
{
    private const string RegistryPath = @"Software\Microsoft\Windows\CurrentVersion\Run";
    private const string ValueName = "SplitergollBatteries";
    private static string Command => "\"" + Environment.ProcessPath + "\"";

    // Consulte uniquement l’inscription de cette application, sans la créer au lancement.
    public static bool Enabled
    {
        get { using var key = Registry.CurrentUser.OpenSubKey(RegistryPath); return key?.GetValue(ValueName) as string == Command; }
    }

    // Une action explicite ajoute ou retire la seule entrée utilisateur de Splitergoll.
    public static void SetEnabled(bool enabled)
    {
        using var key = Registry.CurrentUser.CreateSubKey(RegistryPath, true);
        if (enabled) key.SetValue(ValueName, Command);
        else key.DeleteValue(ValueName, false);
    }
}
