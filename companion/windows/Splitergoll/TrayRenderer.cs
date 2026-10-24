// Dessine les deux valeurs directement à la définition réelle de l’icône, sans agrandir un bitmap.
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Drawing.Text;
using System.Runtime.InteropServices;
using Microsoft.Win32;
using Splitergoll.BatteryCore;

namespace Splitergoll;

internal static class TrayRenderer
{
    // Détermine le contraste de la barre Windows, avec repli sur les couleurs système.
    public static bool DarkTaskbar()
    {
        try
        {
            using var key = Registry.CurrentUser.OpenSubKey(@"Software\Microsoft\Windows\CurrentVersion\Themes\Personalize");
            if (key?.GetValue("SystemUsesLightTheme") is int value) return value == 0;
        }
        catch (Exception error) when (error is UnauthorizedAccessException or System.Security.SecurityException) { }
        return SystemColors.Window.GetBrightness() < 0.5;
    }

    // Choisit une taille lisible pour tous les niveaux avant d’autoriser le symbole de pourcentage.
    public static Icon Draw(int size, Indication left, Indication right)
    {
        using var bitmap = new Bitmap(size, size, PixelFormat.Format32bppArgb);
        using var graphics = Graphics.FromImage(bitmap);
        graphics.Clear(Color.Transparent);
        graphics.CompositingQuality = CompositingQuality.HighQuality;
        graphics.TextRenderingHint = TextRenderingHint.AntiAliasGridFit;
        using var brush = new SolidBrush(DarkTaskbar() ? Color.White : Color.FromArgb(25, 25, 25));
        float row = size / 2f;
        float fontSize = row * 1.06f;
        using var format = (StringFormat)StringFormat.GenericTypographic.Clone();
        format.FormatFlags |= StringFormatFlags.NoWrap;
        format.Alignment = StringAlignment.Center;
        format.LineAlignment = StringAlignment.Center;
        Font? selected = null;
        for (float candidate = fontSize; candidate >= row * 0.8f; candidate -= 0.25f)
        {
            var font = new Font("Segoe UI", candidate, FontStyle.Bold, GraphicsUnit.Pixel);
            var digits = graphics.MeasureString("99", font, size * 3, format);
            if (digits.Width <= size && digits.Height <= row + size * 0.1f) { selected = font; break; }
            font.Dispose();
        }
        using var finalFont = selected ?? new Font("Segoe UI", row * 0.8f, FontStyle.Bold, GraphicsUnit.Pixel);
        var sizes = Enumerable.Range(1, 99).Select(value => graphics.MeasureString(value + "%", finalFont, size * 3, format)).ToArray();
        bool percent = TrayText.CanUsePercent(size - 1, sizes.Max(value => value.Width), row + size * 0.1f, sizes.Max(value => value.Height));
        Indication[] values = [left, right];
        for (int index = 0; index < values.Length; index++)
        {
            var area = new RectangleF(0, index * row - size * 0.015f, size, row);
            if (values[index].Kind == IndicationKind.PossibleCharge) DrawBolt(graphics, brush, area);
            else graphics.DrawString(TrayText.Format(values[index], percent), finalFont, brush, area, format);
        }
        IntPtr handle = bitmap.GetHicon();
        try { using var borrowed = Icon.FromHandle(handle); return (Icon)borrowed.Clone(); }
        finally { DestroyIcon(handle); }
    }

    // L’éclair vectoriel reste lisible sans dépendre d’un glyphe disponible sur le PC.
    private static void DrawBolt(Graphics graphics, Brush brush, RectangleF area)
    {
        float x = area.Width * 0.35f, y = area.Top + area.Height * 0.1f, w = area.Width * 0.3f, h = area.Height * 0.8f;
        PointF[] points = [new(x + w * .6f, y), new(x, y + h * .6f), new(x + w * .45f, y + h * .6f), new(x + w * .3f, y + h), new(x + w, y + h * .35f), new(x + w * .55f, y + h * .35f)];
        graphics.FillPolygon(brush, points);
    }

    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)] private static extern bool DestroyIcon(IntPtr icon);
}
