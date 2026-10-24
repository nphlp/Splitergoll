// Présente uniquement le clavier suivi, sa connexion et les deux niveaux.
using System.Runtime.InteropServices;
using Splitergoll.BatteryCore;

namespace Splitergoll;

internal sealed class BatteryWindow : Form
{
    private const int InitialClientWidth = 255;
    private readonly Label heading = new();
    private readonly Label connection = new();
    private readonly Label left = new();
    private readonly Label right = new();
    private readonly Label leftCaption = new();
    private readonly Label rightCaption = new();
    private readonly TableLayoutPanel layout = new();
    private readonly PictureBox branding = new();
    private readonly Icon applicationIcon;
    private readonly Bitmap applicationImage;
    private bool ready;
    private bool fitting;
    private bool artworkDisposed;
    public bool Popup { get; }
    public event Action? OpenRequested;

    // Une fermeture cache la vue sans arrêter le lecteur ni supprimer l’icône.
    public BatteryWindow(bool popup)
    {
        Popup = popup;
        Text = "Splitergoll";
        AutoScaleMode = AutoScaleMode.Dpi;
        AutoScaleDimensions = new SizeF(96, 96);
        Font = new Font("Segoe UI", 9);
        ClientSize = new Size(InitialClientWidth, 180);
        AutoSize = false;
        FormBorderStyle = popup ? FormBorderStyle.None : FormBorderStyle.Sizable;
        MaximizeBox = false; MinimizeBox = !popup;
        ShowInTaskbar = !popup; StartPosition = FormStartPosition.Manual;
        TopMost = popup;
        Padding = new Padding(18);
        applicationIcon = LoadApplicationIcon();
        Icon = applicationIcon;
        ShowIcon = true;
        using var imageIcon = new Icon(applicationIcon, new Size(256, 256));
        applicationImage = imageIcon.ToBitmap();
        branding.Name = "branding";
        branding.Image = applicationImage;
        branding.SizeMode = PictureBoxSizeMode.Zoom;
        branding.Size = new Size(32, 32);
        branding.Anchor = AnchorStyles.Left;
        branding.Margin = new Padding(0, 0, 12, 8);
        layout.Name = "content";
        layout.Dock = DockStyle.Top;
        layout.AutoSize = false;
        layout.ColumnCount = 3;
        layout.RowCount = popup ? 5 : 4;
        layout.Margin = Padding.Empty;
        layout.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
        layout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        layout.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
        for (int row = 0; row < layout.RowCount; row++) layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        heading.Name = "heading";
        heading.Font = new Font(Font.FontFamily, 13, FontStyle.Bold);
        heading.AutoSize = true; heading.Anchor = AnchorStyles.Left | AnchorStyles.Right;
        heading.Margin = new Padding(0, 0, 0, 8);
        connection.AutoSize = true; connection.Anchor = AnchorStyles.Left;
        connection.Margin = new Padding(0, 0, 0, 12);
        layout.Controls.Add(branding, 0, 0);
        layout.Controls.Add(heading, 1, 0); layout.SetColumnSpan(heading, 2);
        layout.Controls.Add(connection, 0, 1); layout.SetColumnSpan(connection, 3);
        foreach (var item in new[] { ("Gauche", leftCaption, left, 2), ("Droite", rightCaption, right, 3) })
        {
            var caption = item.Item2; var value = item.Item3;
            caption.Text = item.Item1; caption.AutoSize = true;
            caption.Anchor = AnchorStyles.Left; caption.Margin = new Padding(0, 0, 0, 8);
            layout.Controls.Add(caption, 0, item.Item4); layout.SetColumnSpan(caption, 2);
            value.AutoSize = true; value.Anchor = AnchorStyles.Right;
            value.TextAlign = ContentAlignment.MiddleRight;
            value.Margin = new Padding(16, 0, 0, 8);
            layout.Controls.Add(value, 2, item.Item4);
        }
        if (popup)
        {
            var open = new Button
            {
                Text = "Ouvrir l’app",
                AutoSize = true,
                AutoSizeMode = AutoSizeMode.GrowAndShrink,
                FlatStyle = FlatStyle.System,
                Anchor = AnchorStyles.Left,
                Padding = new Padding(10, 6, 10, 6),
                Margin = new Padding(0, 8, 0, 0)
            };
            open.Click += (_, _) => { Hide(); OpenRequested?.Invoke(); };
            layout.Controls.Add(open, 0, 4); layout.SetColumnSpan(open, 3);
            Deactivate += (_, _) => Hide();
        }
        Controls.Add(layout);
        layout.Layout += (_, _) => FitContent();
        FormClosing += (_, args) => { if (args.CloseReason == CloseReason.UserClosing) { args.Cancel = true; Hide(); } };
        ready = true;
        FitContent();
    }

    // La même ressource sert au fichier portable, à la fenêtre et au logo de son panneau.
    private static Icon LoadApplicationIcon()
    {
        using var stream = typeof(BatteryWindow).Assembly.GetManifestResourceStream("Splitergoll.AppIcon.ico")
            ?? throw new InvalidOperationException("Icône Splitergoll absente du build.");
        using var loaded = new Icon(stream, new Size(64, 64));
        return (Icon)loaded.Clone();
    }

    // La largeur est stable au DPI courant, seule la hauteur provient des lignes de contenu.
    private void FitContent()
    {
        if (!ready || fitting || IsDisposed || Disposing) return;
        fitting = true;
        try
        {
            int captions = Math.Max(leftCaption.PreferredSize.Width, rightCaption.PreferredSize.Width);
            int values = Math.Max(left.PreferredSize.Width, right.PreferredSize.Width);
            int minimumWidth = Math.Max(LogicalToDeviceUnits(InitialClientWidth), captions + values + left.Margin.Horizontal + Padding.Horizontal);
            int width = Popup ? minimumWidth : Math.Max(minimumWidth, ClientSize.Width);
            if (ClientSize.Width != width) ClientSize = new Size(width, ClientSize.Height);
            layout.Width = Math.Max(1, width - Padding.Horizontal);
            int contentHeight = layout.GetPreferredSize(new Size(layout.Width, 0)).Height;
            layout.Height = contentHeight;
            int minimumHeight = contentHeight + Padding.Vertical;
            var minimum = SizeFromClientSize(new Size(minimumWidth, minimumHeight));
            if (MinimumSize != minimum) MinimumSize = minimum;
            var client = new Size(width, Popup ? minimumHeight : Math.Max(minimumHeight, ClientSize.Height));
            if (ClientSize != client) ClientSize = client;
            layout.PerformLayout();
        }
        finally { fitting = false; }
    }

    // Les redimensionnements manuels recalculent la hauteur minimale sans refermer la fenêtre.
    protected override void OnLayout(LayoutEventArgs args)
    {
        base.OnLayout(args);
        FitContent();
    }

    // Recalcule les dimensions logiques une fois le changement de moniteur appliqué par WinForms.
    protected override void OnDpiChanged(DpiChangedEventArgs args)
    {
        base.OnDpiChanged(args);
        FitContent();
    }

    // Applique les mêmes indications que l’icône et une infobulle honnête pour cent pour cent.
    public void UpdateState(string name, BatteryState state, bool demo, ToolTip hints)
    {
        heading.Text = name;
        connection.Text = (state.Connected ? "Connecté" : "Déconnecté") + (demo ? " · Démonstration" : "");
        var now = DateTimeOffset.UtcNow;
        var l = state.Get(Side.Left, now); var r = state.Get(Side.Right, now);
        left.Text = l.Text; right.Text = r.Text;
        hints.SetToolTip(left, l.Kind == IndicationKind.PossibleCharge ? "Charge possible. Le 100 % reçu ne prouve pas une charge effective." : null);
        hints.SetToolTip(right, r.Kind == IndicationKind.PossibleCharge ? "Charge possible. Le 100 % reçu ne prouve pas une charge effective." : null);
        bool dark = TrayRenderer.DarkTaskbar();
        BackColor = dark ? Color.FromArgb(35, 35, 35) : Color.FromArgb(250, 250, 250);
        ForeColor = dark ? Color.WhiteSmoke : Color.FromArgb(25, 25, 25);
        if (IsHandleCreated)
        {
            int enabled = dark ? 1 : 0;
            DwmSetWindowAttribute(Handle, 20, ref enabled, sizeof(int));
        }
        FitContent();
    }

    // Attend le calcul de la taille et du DPI avant de replacer la vue dans la zone utile.
    public void ShowNear(Rectangle icon)
    {
        PlaceNear(icon);
        Show(); FitContent(); PlaceNear(icon); Activate();
        if (Popup)
        {
            int rounded = 2; DwmSetWindowAttribute(Handle, 33, ref rounded, sizeof(int));
        }
    }

    // Garde le panneau au-dessus de la barre, même si son contenu agrandit sa hauteur.
    private void PlaceNear(Rectangle icon)
    {
        var work = Screen.FromRectangle(icon).WorkingArea;
        int gap = LogicalToDeviceUnits(8);
        int x = Math.Clamp(icon.Right - Width, work.Left, Math.Max(work.Left, work.Right - Width));
        int y = icon.Top - Height - gap;
        if (y < work.Top) y = icon.Bottom + gap;
        Location = new Point(x, Math.Clamp(y, work.Top, Math.Max(work.Top, work.Bottom - Height)));
    }

    // Libère les copies graphiques détenues par cette vue, sans toucher à l’icône de notification.
    protected override void Dispose(bool disposing)
    {
        if (disposing && !artworkDisposed)
        {
            artworkDisposed = true;
            branding.Image = null; Icon = null;
            applicationImage.Dispose(); applicationIcon.Dispose();
        }
        base.Dispose(disposing);
    }

    [DllImport("dwmapi.dll")] private static extern int DwmSetWindowAttribute(IntPtr window, int attribute, ref int value, int size);
}
