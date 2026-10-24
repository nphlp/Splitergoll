// Lit les batteries des claviers déjà appairés et connectés, sans scan ni écriture GATT.
using System.Runtime.InteropServices;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Storage.Streams;
using Splitergoll.BatteryCore;

namespace Splitergoll;

internal sealed class BatteryMonitor : IDisposable
{
    private static readonly Guid ServiceId = new("0000180f-0000-1000-8000-00805f9b34fb");
    private static readonly Guid LevelId = new("00002a19-0000-1000-8000-00805f9b34fb");
    private static readonly Guid PresentationId = new("00002904-0000-1000-8000-00805f9b34fb");
    private readonly string target;
    private readonly bool demo;
    private readonly List<GattDeviceService> services = [];
    private readonly List<(int Id, GattCharacteristic Level)> channels = [];
    private BluetoothLEDevice? device;
    private CancellationTokenSource? operation;
    private DateTimeOffset retryAfter;
    private bool refreshing;
    private bool disposed;
    private long session;
    private bool invalidated;
    public BatteryState State { get; } = new();
    public KeyboardSelection Selection { get; }
    public bool Suspended { get; private set; }
    public string? Error { get; private set; }

    // Le mode démo ne consulte ni préférences ni Bluetooth et ne réalise aucun enregistrement.
    public BatteryMonitor(string target, bool demo)
    {
        this.target = target;
        this.demo = demo;
        Selection = new KeyboardSelection(demo ? null : Preferences.Load());
        if (demo)
        {
            Selection.Update([new("demo", target + " V2 (démo)")], target);
            session = State.BeginSession(); State.Identify(session, 0, [4, 0, 0xAD, 0x27, 1, 6, 1]);
            State.Identify(session, 1, [4, 0, 0xAD, 0x27, 1, 8, 1]);
        }
    }

    // Change seulement le clavier lu par l’application, jamais le profil Bluetooth de saisie.
    public void Select(string id)
    {
        if (demo) return;
        if (!Selection.Select(id)) return;
        operation?.Cancel();
        Release(); retryAfter = DateTimeOffset.MinValue;
        Preferences.Save(Selection.Preferred);
    }

    // La suspension invalide les opérations en vol sans maintenir le PC ou le clavier éveillé.
    public void SetSuspended(bool value)
    {
        Suspended = value;
        operation?.Cancel();
        Release(); retryAfter = DateTimeOffset.MinValue;
    }

    // Le battement refuse tout chevauchement et borne chaque opération à quinze secondes.
    public async Task RefreshAsync()
    {
        if (disposed || refreshing || Suspended) return;
        if (demo)
        {
            State.Receive(session, 0, [90], DateTimeOffset.UtcNow);
            State.Receive(session, 1, [10], DateTimeOffset.UtcNow);
            return;
        }
        if (DateTimeOffset.UtcNow < retryAfter) return;
        refreshing = true;
        using var timeout = new CancellationTokenSource(TimeSpan.FromSeconds(15));
        operation = timeout;
        try
        {
            var selector = BluetoothLEDevice.GetDeviceSelectorFromPairingState(true);
            var inventory = await DeviceInformation.FindAllAsync(selector,
                ["System.Devices.Aep.IsConnected"], DeviceInformationKind.AssociationEndpoint).AsTask(timeout.Token);
            timeout.Token.ThrowIfCancellationRequested();
            var previous = Selection.Selected?.Id;
            Selection.Update(inventory.Where(item => item.Properties.TryGetValue("System.Devices.Aep.IsConnected", out var connected) && connected is true)
                .Select(item => new KeyboardIdentity(item.Id, item.Name)), target);
            if (previous != Selection.Selected?.Id || Volatile.Read(ref invalidated)) Release();
            if (!Selection.SelectedIsAvailable || Selection.Selected is null)
            {
                Release(); Error = null; return;
            }
            if (device is null)
            {
                var chosen = Selection.Selected.Id;
                var opened = await BluetoothLEDevice.FromIdAsync(chosen).AsTask(timeout.Token);
                if (timeout.IsCancellationRequested || disposed || Selection.Selected?.Id != chosen)
                { opened?.Dispose(); return; }
                device = opened;
                if (device is null || device.ConnectionStatus != BluetoothConnectionStatus.Connected)
                    throw new IOException("Le clavier n’est pas connecté au système.");
                device.GattServicesChanged += ServicesChanged;
                session = State.BeginSession();
                await DiscoverAsync(timeout.Token);
            }
            if (device.ConnectionStatus != BluetoothConnectionStatus.Connected)
                throw new IOException("Le clavier est déconnecté.");
            var current = session;
            bool hasResponse = false;
            foreach (var channel in channels)
            {
                var result = await channel.Level.ReadValueAsync(BluetoothCacheMode.Uncached).AsTask(timeout.Token);
                timeout.Token.ThrowIfCancellationRequested();
                if (result.Status != GattCommunicationStatus.Success) continue;
                hasResponse |= State.Receive(current, channel.Id, Bytes(result.Value), DateTimeOffset.UtcNow);
            }
            if (!hasResponse) throw new IOException("Les batteries ne répondent pas.");
            Error = null;
        }
        catch (OperationCanceledException) { Release(); }
        catch (Exception error) when (error is IOException or FormatException or COMException or UnauthorizedAccessException or InvalidOperationException)
        {
            Error = error.Message;
            Release(); retryAfter = DateTimeOffset.UtcNow.AddSeconds(30);
        }
        finally { if (ReferenceEquals(operation, timeout)) operation = null; refreshing = false; }
    }

    // Ne découvre que BAS et la présentation des niveaux, sans activer de notifications.
    private async Task DiscoverAsync(CancellationToken cancellation)
    {
        var found = await device!.GetGattServicesForUuidAsync(ServiceId, BluetoothCacheMode.Uncached).AsTask(cancellation);
        services.AddRange(found.Services);
        cancellation.ThrowIfCancellationRequested();
        if (found.Status != GattCommunicationStatus.Success || found.Services.Count is < 1 or > 2)
        {
            throw new IOException("Les services batterie sont indisponibles.");
        }
        foreach (var service in services)
        {
            var values = await service.GetCharacteristicsForUuidAsync(LevelId, BluetoothCacheMode.Uncached).AsTask(cancellation);
            cancellation.ThrowIfCancellationRequested();
            if (values.Status != GattCommunicationStatus.Success || values.Characteristics.Count != 1
                || !values.Characteristics[0].CharacteristicProperties.HasFlag(GattCharacteristicProperties.Read))
                throw new FormatException("Niveau batterie non lisible.");
            var characteristic = values.Characteristics[0];
            var descriptions = await characteristic.GetDescriptorsForUuidAsync(PresentationId, BluetoothCacheMode.Uncached).AsTask(cancellation);
            cancellation.ThrowIfCancellationRequested();
            if (descriptions.Status != GattCommunicationStatus.Success || descriptions.Descriptors.Count != 1)
                throw new FormatException("Côté de la batterie non identifié.");
            var presentation = await descriptions.Descriptors[0].ReadValueAsync(BluetoothCacheMode.Uncached).AsTask(cancellation);
            cancellation.ThrowIfCancellationRequested();
            if (presentation.Status != GattCommunicationStatus.Success) throw new IOException("Présentation batterie non lisible.");
            int id = channels.Count;
            State.Identify(session, id, Bytes(presentation.Value));
            channels.Add((id, characteristic));
        }
    }

    // Signale l’invalidation depuis un callback système, sans libérer des objets sur sa file.
    private void ServicesChanged(BluetoothLEDevice sender, object args) => Volatile.Write(ref invalidated, true);

    // Convertit le tampon natif en données bornées, strictement validées par le noyau.
    private static byte[] Bytes(IBuffer buffer)
    {
        if (buffer.Length > 32) throw new FormatException("Réponse batterie trop longue.");
        using var reader = DataReader.FromBuffer(buffer);
        var bytes = new byte[buffer.Length]; reader.ReadBytes(bytes); return bytes;
    }

    // Ferme uniquement les références clientes, sans déconnexion HID ni suppression des bonds.
    private void Release()
    {
        session = State.BeginSession();
        channels.Clear();
        foreach (var service in services) service.Dispose();
        services.Clear();
        if (device is not null) { device.GattServicesChanged -= ServicesChanged; device.Dispose(); device = null; }
        Volatile.Write(ref invalidated, false);
    }

    // Arrête le lecteur et invalide toute réponse tardive lors de la fermeture.
    public void Dispose()
    {
        if (disposed) return;
        disposed = true; operation?.Cancel(); Release();
    }
}
