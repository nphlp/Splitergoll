// Exerce les vrais contrôles WinForms sous Windows, sans lecteur Bluetooth ni préférence.
using Splitergoll;
using Splitergoll.BatteryCore;

namespace Splitergoll.UiTests;

internal static class Program
{
    // Affiche brièvement chaque vue pour appliquer le layout natif, puis libère ses ressources.
    [STAThread]
    private static int Main()
    {
        Application.SetHighDpiMode(HighDpiMode.PerMonitorV2);
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        int failures = 0, total = 0;
        foreach (bool popup in new[] { false, true })
            foreach (float textScale in new[] { 1f, 1.5f, 2f, 3f })
                foreach (bool longName in new[] { false, true })
                {
                    total++;
                    string name = $"{(popup ? "panneau" : "fenêtre")}, texte x{textScale}, nom long={longName}";
                    try { CheckWindow(popup, textScale, longName); Console.WriteLine("OK " + name); }
                    catch (Exception error) { failures++; Console.Error.WriteLine($"ÉCHEC {name}: {error.Message}"); }
                }
        Console.WriteLine($"{total - failures}/{total} tests UI réussis.");
        return failures == 0 ? 0 : 1;
    }

    // Contrôle des polices agrandies et noms longs, distinct du DPI réellement choisi par Windows.
    private static void CheckWindow(bool popup, float textScale, bool longName)
    {
        using var window = new BatteryWindow(popup);
        using var hints = new ToolTip();
        var fonts = new List<Font>();
        try
        {
            var originals = Descendants(window).Prepend(window).Select(control => (Control: control, Font: control.Font)).ToArray();
            foreach (var original in originals)
            {
                var font = new Font(original.Font.FontFamily, original.Font.Size * textScale, original.Font.Style);
                fonts.Add(font); original.Control.Font = font;
            }
            var state = new BatteryState();
            var session = state.BeginSession();
            state.Identify(session, 0, [4, 0, 0xAD, 0x27, 1, 6, 1]);
            state.Receive(session, 0, [89], DateTimeOffset.UtcNow);
            string keyboard = longName ? "Splitergoll V2, clavier personnel du bureau" : "Splitergoll";
            window.UpdateState(keyboard, state, true, hints);
            var work = Screen.PrimaryScreen!.WorkingArea;
            var icon = new Rectangle(work.Right - 40, work.Bottom, 24, 24);
            window.ShowNear(icon);
            CheckLayout(window);
            if (textScale == 1 && !longName)
                Require(window.ClientSize.Width <= (int)Math.Ceiling(270 * window.DeviceDpi / 96d), "Largeur initiale trop grande.");
            Require(work.Contains(window.Bounds), "Vue hors de la zone utile après ouverture.");
            Require(window.Icon is { Width: >= 32 }, "Icône Splitergoll de fenêtre absente.");
            var branding = window.Controls.OfType<TableLayoutPanel>().Single().Controls.OfType<PictureBox>().Single();
            Require(branding.Image is not null && branding.Width > 0 && branding.Height > 0, "Logo du panneau absent.");
            if (!popup)
            {
                Require(window.FormBorderStyle == FormBorderStyle.Sizable, "Fenêtre non redimensionnable.");
                window.ClientSize = new Size(window.ClientSize.Width + 100, window.ClientSize.Height + 100);
                window.PerformLayout(); CheckLayout(window);
            }
            window.Close();
            Require(!window.Visible && !window.IsDisposed, "Fermer doit masquer sans quitter.");
            window.ShowNear(icon); CheckLayout(window);
            window.Close();
        }
        finally { foreach (var font in fonts) font.Dispose(); }
    }

    // Refuse les hauteurs fixes, le texte rogné, les chevauchements et le contenu hors fenêtre.
    private static void CheckLayout(BatteryWindow window)
    {
        var table = window.Controls.OfType<TableLayoutPanel>().Single();
        table.PerformLayout();
        Require(!window.AutoSize, "La taille de la Form ne doit pas dépendre du pourcentage de ses colonnes.");
        Require(window.ClientSize.Width >= (int)Math.Round(240 * window.DeviceDpi / 96d), "Largeur initiale illisible.");
        Require(window.MinimumSize.Width > 0 && window.MinimumSize.Height > 0, "Dimensions minimales absentes.");
        Require(table.RowStyles.Cast<RowStyle>().All(row => row.SizeType == SizeType.AutoSize), "Hauteur de ligne imposée.");
        Require(window.ClientRectangle.Contains(table.Bounds), "Tableau hors de la fenêtre.");
        foreach (Control control in table.Controls)
        {
            if (control is Label or Button)
            {
                Size preferred = control.GetPreferredSize(new Size(control.Width, 0));
                Require(control.Height >= preferred.Height, "Texte coupé : " + control.Text);
            }
            Require(table.ClientRectangle.Contains(control.Bounds), "Contrôle hors du tableau : " + control.Text);
        }
        var controls = table.Controls.Cast<Control>().ToArray();
        for (int i = 0; i < controls.Length; i++)
            for (int j = i + 1; j < controls.Length; j++)
                Require(!controls[i].Bounds.IntersectsWith(controls[j].Bounds), "Contrôles superposés.");
    }

    // Parcourt l’arbre réel des contrôles pour appliquer l’agrandissement de texte à chaque police.
    private static IEnumerable<Control> Descendants(Control root)
    {
        foreach (Control child in root.Controls)
        {
            yield return child;
            foreach (var descendant in Descendants(child)) yield return descendant;
        }
    }

    // Une régression est un échec explicite, jamais un simple message d’information.
    private static void Require(bool condition, string message)
    {
        if (!condition) throw new InvalidOperationException(message);
    }
}
