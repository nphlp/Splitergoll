// Gère une seule icône Win32 avec identité stable, DPI du moniteur et ressources libérées.
using System.Runtime.InteropServices;

namespace Splitergoll;

internal sealed class TrayIcon : NativeWindow, IDisposable
{
    private const uint Callback = 0x8001;
    private static readonly Guid Identity = new("4da991dc-c94f-49dc-aa1c-4a10934257b6");
    private readonly uint taskbarCreated = RegisterWindowMessage("TaskbarCreated");
    private Icon? icon;
    private string tooltip = "Splitergoll";
    public event Action<Point>? Click;
    public event Action<Point>? RightClick;

    // Crée une fenêtre de messages invisible, sans fenêtre principale ou accès système privilégié.
    public TrayIcon() => CreateHandle(new CreateParams { Caption = "Splitergoll notifications" });

    // Ajoute ou remplace le dessin tout en gardant la même identité dans le Shell.
    public void Update(Icon next, string text)
    {
        bool first = icon is null;
        var previous = icon; icon = next; tooltip = text.Length <= 127 ? text : text[..127];
        var data = Data();
        if (!ShellNotifyIcon(first ? 0u : 1u, ref data))
        { if (!first) ShellNotifyIcon(0, ref data); }
        data.Version = 4; ShellNotifyIcon(4, ref data);
        previous?.Dispose();
    }

    // Retourne la zone réelle de l’icône, y compris après déplacement de la barre des tâches.
    public Rectangle Bounds
    {
        get
        {
            var id = new IconIdentifier { Size = (uint)Marshal.SizeOf<IconIdentifier>(), Window = Handle, Id = 1, Guid = Identity };
            return ShellGetRect(ref id, out var rect) == 0
                ? Rectangle.FromLTRB(rect.Left, rect.Top, rect.Right, rect.Bottom)
                : new Rectangle(Cursor.Position, new Size(16, 16));
        }
    }

    // Mesure les petites icônes au DPI du moniteur contenant la zone de notification.
    public int PixelSize
    {
        get
        {
            var bounds = Bounds;
            SetWindowPos(Handle, IntPtr.Zero, bounds.Left, bounds.Top, 1, 1, 0x4 | 0x10 | 0x200);
            uint dpi = GetDpiForWindow(Handle);
            if (dpi == 0) dpi = 96;
            return Math.Clamp(GetSystemMetricsForDpi(49, dpi), 16, 128);
        }
    }

    // Traite activation clavier/souris et restauration de l’icône après redémarrage d’Explorer.
    protected override void WndProc(ref Message message)
    {
        if ((uint)message.Msg == taskbarCreated && icon is not null)
        { var data = Data(); ShellNotifyIcon(0, ref data); data.Version = 4; ShellNotifyIcon(4, ref data); }
        if ((uint)message.Msg == Callback)
        {
            int code = (int)((long)message.LParam & 0xFFFF);
            var point = new Point((short)((long)message.WParam & 0xFFFF), (short)(((long)message.WParam >> 16) & 0xFFFF));
            if (code is 0x400 or 0x401) Click?.Invoke(point);
            else if (code == 0x7B) RightClick?.Invoke(point);
        }
        base.WndProc(ref message);
    }

    // Construit le message Unicode en conservant le GUID entre les versions portables.
    private IconData Data() => new()
    {
        Size = (uint)Marshal.SizeOf<IconData>(),
        Window = Handle,
        Id = 1,
        Flags = 1 | 2 | 4 | 0x20 | 0x80,
        CallbackMessage = Callback,
        Icon = icon?.Handle ?? IntPtr.Zero,
        Tip = tooltip,
        Info = "",
        InfoTitle = "",
        Guid = Identity
    };

    // Supprime seulement l’icône de cette application et libère son handle graphique.
    public void Dispose()
    {
        if (Handle == IntPtr.Zero) return;
        var data = Data(); ShellNotifyIcon(2, ref data); icon?.Dispose(); icon = null; DestroyHandle();
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    private struct IconData
    {
        public uint Size; public IntPtr Window; public uint Id, Flags, CallbackMessage; public IntPtr Icon;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)] public string Tip;
        public uint State, StateMask;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)] public string Info;
        public uint Version;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)] public string InfoTitle;
        public uint InfoFlags; public Guid Guid; public IntPtr BalloonIcon;
    }
    [StructLayout(LayoutKind.Sequential)] private struct IconIdentifier { public uint Size; public IntPtr Window; public uint Id; public Guid Guid; }
    [StructLayout(LayoutKind.Sequential)] private struct NativeRect { public int Left, Top, Right, Bottom; }
    [DllImport("shell32.dll", EntryPoint = "Shell_NotifyIconW", CharSet = CharSet.Unicode)]
    [return: MarshalAs(UnmanagedType.Bool)] private static extern bool ShellNotifyIcon(uint message, ref IconData data);
    [DllImport("shell32.dll", EntryPoint = "Shell_NotifyIconGetRect")] private static extern int ShellGetRect(ref IconIdentifier identifier, out NativeRect rect);
    [DllImport("user32.dll", CharSet = CharSet.Unicode)] private static extern uint RegisterWindowMessage(string text);
    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)] private static extern bool SetWindowPos(IntPtr window, IntPtr after, int x, int y, int width, int height, uint flags);
    [DllImport("user32.dll")] private static extern uint GetDpiForWindow(IntPtr window);
    [DllImport("user32.dll")] private static extern int GetSystemMetricsForDpi(int index, uint dpi);
}
