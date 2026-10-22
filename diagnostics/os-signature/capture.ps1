# Lit les diagnostics USB/BLE (-Port COMx [-Ble]), contrôle les emplacements (-Profile) ou capture ETW (-UsbEvents).
# Fonctionne seul, sans dépôt ni arborescence imposée, avec les fichiers dans le dossier courant.
# Le mode UsbEvents observe le firmware normal sans port série et nécessite un terminal administrateur.
# Aucun flash, reset, effacement de bonds ou pilote supplémentaire. -Profile est une action explicite.
[CmdletBinding(DefaultParameterSetName = 'Serial')]
param(
    [Parameter(Mandatory = $true, ParameterSetName = 'Serial')]
    [Parameter(Mandatory = $true, ParameterSetName = 'Profile')]
    [ValidatePattern('^COM[1-9][0-9]*$')]
    [string]$Port,

    [Parameter(ParameterSetName = 'Serial')]
    [switch]$Ble,

    [Parameter(Mandatory = $true, ParameterSetName = 'Profile')]
    [ValidatePattern('^(status|free|[0-9])$')]
    [string]$Profile,

    [Parameter(Mandatory = $true, ParameterSetName = 'UsbEvents')]
    [switch]$UsbEvents
)

# Contrôle les sessions et la table GATT du diagnostic BLE, sans dépendance à un autre fichier.
function Test-BleCapture {
    param([System.Collections.Generic.List[string]]$Lines)

    if ($Lines.Count -lt 4 -or $Lines[1] -notmatch '^BLETRACE BEGIN version=([12]) side=left count=([0-9]+) dropped=0 rejected=0 uptime_ms=[0-9]+ session=([0-9]+) connected=([0-9]+)$') {
        throw 'Capture BLE refusée : format incorrect ou pertes signalées.'
    }
    $Version = [int]$Matches[1]
    $Expected = [int]$Matches[2]
    $ExpectedSession = [int]$Matches[3]
    $ExpectedConnected = [int]$Matches[4]
    if ($Expected -lt 1 -or $Expected -gt 256 -or $ExpectedConnected -gt 6 -or
        ($Version -eq 1 -and $ExpectedConnected -gt 1) -or $Lines.Count -lt $Expected + 4) {
        throw "Nombre d’événements BLE incorrect."
    }
    $Session = 0
    $Connected = 0
    $Live = @{}
    $Profiles = @{}
    for ($Index = 0; $Index -lt $Expected; $Index++) {
        if ($Lines[$Index + 2] -notmatch '^BLETRACE EVENT ([0-9]+) [0-9]+ ([0-9]+) (CONNECTED|DISCONNECTED|SECURITY|PARAM_REQUEST|PARAM_UPDATED|MTU|FEATURES|ATT)( [0-9a-f]{8}){5}$' -or [int]$Matches[1] -ne $Index) {
            throw 'Capture BLE refusée : événements manquants ou désordonnés.'
        }
        $EventSession = [int]$Matches[2]
        $Kind = $Matches[3]
        $Fields = $Lines[$Index + 2] -split ' '
        if ($Kind -eq 'CONNECTED') {
            if (($Version -eq 1 -and $Connected -ne 0) -or $Connected -ge 6 -or $EventSession -ne $Session + 1) { throw 'Sessions BLE incohérentes.' }
            $Live[$EventSession] = $true
            $Connected++
            $Session++
            if ($Version -eq 2) {
                $Code = [Convert]::ToUInt32($Fields[10], 16)
                if ($Code -gt 10) { throw 'Emplacement BLE invalide.' }
                $Profiles[$EventSession] = [int]$Code - 1
            }
        } elseif (-not $Live[$EventSession]) { throw 'Session BLE absente.' }
        if ($Kind -eq 'DISCONNECTED') {
            if ($Version -eq 2) {
                $Code = [Convert]::ToUInt32($Fields[7], 16)
                if ($Code -gt 10 -or ($Profiles[$EventSession] -ge 0 -and $Profiles[$EventSession] -ne [int]$Code - 1)) { throw 'Emplacement BLE incohérent.' }
                $Profiles[$EventSession] = [int]$Code - 1
            }
            $Live[$EventSession] = $false
            $Connected--
        }
    }
    $Attributes = 0
    $Seen = @{}
    for ($Index = $Expected + 2; $Index -lt $Lines.Count - 1; $Index++) {
        if ($Lines[$Index] -match '^BLETRACE (OS|HOST) ') { break }
        if ($Lines[$Index] -notmatch '^BLETRACE ATTR ([0-9a-f]{4}) ([0-9a-f]{4}|[0-9a-f]{8}|[0-9a-f]{8}(-[0-9a-f]{4}){3}-[0-9a-f]{12})$') {
            throw 'Table GATT BLE invalide.'
        }
        $Handle = $Matches[1]
        if ($Seen.ContainsKey($Handle)) { throw 'Handle GATT dupliqué.' }
        $Seen[$Handle] = $true
        $Attributes++
    }
    $Reports = 0
    for (; $Index -lt $Lines.Count - 1; $Index++) {
        if ($Version -eq 2) {
            if ($Lines[$Index] -notmatch '^BLETRACE HOST session=([0-9]+) profile=(-1|[0-9]) connected=([01])$' -or [int]$Matches[1] -ne $Reports + 1 -or [int]$Matches[1] -gt $Session) { throw 'Association de session BLE incorrecte.' }
            $HostSession = [int]$Matches[1]
            $HostProfile = [int]$Matches[2]
            $HostConnected = [int]$Matches[3]
            if ($HostConnected -ne [int]$Live[$HostSession] -or ($Profiles[$HostSession] -ge 0 -and $HostProfile -ne $Profiles[$HostSession])) { throw 'État de session BLE incohérent.' }
            $Index++
        }
        if ($Index -ge $Lines.Count - 1 -or $Lines[$Index] -notmatch '^BLETRACE OS session=([0-9]+) result=(macos|windows|unknown|ambiguous) evidence=[0-9a-f]{4}$' -or
            [int]$Matches[1] -ne $Reports + 1 -or [int]$Matches[1] -gt $Session) {
            throw 'Hypothèses BLE incorrectes ou désordonnées.'
        }
        $Reports++
    }
    if (($Version -eq 2 -or $Reports -ne 0) -and $Reports -ne $Session) {
        throw 'Hypothèses BLE manquantes pour certaines sessions.'
    }
    if ($Lines[$Lines.Count - 1] -notmatch '^BLETRACE END attrs=([0-9]+)$' -or
        [int]$Matches[1] -ne $Attributes -or $Attributes -lt 1 -or $Attributes -gt 512 -or
        $Session -ne $ExpectedSession -or $Connected -ne $ExpectedConnected) {
        throw 'Capture BLE tronquée ou sessions incohérentes.'
    }
}

# Vérifie la réponse du contrôle avant de présenter le choix ou son refus.
function Test-BleProfile {
    param([System.Collections.Generic.List[string]]$Lines, [string]$Command)

    if ($Lines.Count -lt 5 -or $Lines[1] -notmatch '^BLEPROFILE BEGIN version=([1234]) count=([0-9]+) active=([0-9]+) save_delay_ms=[0-9]+$') {
        throw 'Réponse incorrecte : firmware ble-trace avec contrôle des emplacements requis.'
    }
    $Version = [int]$Matches[1]
    $Expected = [int]$Matches[2]
    $Active = [int]$Matches[3]
    $Extra = [int]($Version -ge 3) + [int]($Version -eq 4)
    if ($Expected -lt 1 -or $Expected -gt 10 -or $Active -ge $Expected -or $Lines.Count -ne $Expected + 4 + $Extra) {
        throw 'Réponse des emplacements tronquée ou incohérente.'
    }
    for ($Index = 0; $Index -lt $Expected; $Index++) {
        if ($Lines[$Index + 2] -notmatch '^BLEPROFILE SLOT index=([0-9]+) paired=[01] connected=[01]$' -or [int]$Matches[1] -ne $Index) {
            throw 'Liste des emplacements incorrecte.'
        }
    }
    if ($Version -ge 3) {
        if ($Lines[$Expected + 2] -notmatch '^BLEPROFILE WINDOW ready=([01]) remaining_ms=([0-9]+) advertising=([01]) known=([0-9]+) connected=([0-9]+) error=-?[0-9]+$') {
            throw 'État de reconnexion absent ou incorrect.'
        }
        $Ready = [int]$Matches[1]
        $Remaining = [int]$Matches[2]
        $Advertising = [int]$Matches[3]
        $Known = [int]$Matches[4]
        $Connected = [int]$Matches[5]
        $Limit = 1 -shl $Expected
        if ($Remaining -gt 30000 -or $Known -ge $Limit -or $Connected -ge $Limit -or
            ($Connected -band (-bnot $Known)) -ne 0 -or
            ($Ready -eq 0 -and ($Remaining -ne 0 -or $Advertising -ne 0 -or $Known -ne 0 -or $Connected -ne 0))) {
            throw 'État de reconnexion incohérent.'
        }
    }
    if ($Version -eq 4) {
        if ($Lines[$Expected + 3] -notmatch '^BLEPROFILE POLICY active=([01]) without_host=([01]) loaded=([01]) saved=([01]) remembered=(-1|[0-9]+) restoring=([01]) error=-?[0-9]+$') {
            throw 'Politique de destination absente ou incorrecte.'
        }
        $Awake = [int]$Matches[1]
        $Without = [int]$Matches[2]
        $Loaded = [int]$Matches[3]
        $Saved = [int]$Matches[4]
        $Remembered = [int]$Matches[5]
        if ($Remembered -ge $Expected -or ($Remembered -ge 0 -and ($Loaded -eq 0 -or $Saved -eq 0)) -or
            ($Without -eq 1 -and ($Awake -eq 0 -or $Connected -ne 0))) {
            throw 'Politique de destination incohérente.'
        }
    }
    if ($Lines[$Expected + 2 + $Extra] -notmatch '^BLEPROFILE RESULT command=([SF0-9]) status=(ok|busy|full|invalid|failed) selected=([0-9]+) changed=[01] error=-?[0-9]+$' -or
        $Matches[1] -ne $Command -or [int]$Matches[3] -ne $Active -or $Lines[$Expected + 3 + $Extra] -ne 'BLEPROFILE END') {
        throw 'Résultat du contrôle incorrect.'
    }
}

# Exécute un outil Windows natif et rend explicite tout échec avant de poursuivre.
function Invoke-TraceCommand {
    param([string]$Command, [string[]]$Arguments)

    & $Command @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Échec de $Command, code $LASTEXITCODE."
    }
}

# Observe pendant vingt secondes la pile USB de Windows, sans changer les descripteurs du clavier.
function Capture-UsbEvents {
    param([string]$Directory)

    $Identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $Principal = New-Object Security.Principal.WindowsPrincipal $Identity
    if (-not $Principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw 'Le mode -UsbEvents nécessite PowerShell en administrateur pour lire les événements USB système.'
    }
    $Logman = Join-Path ([Environment]::SystemDirectory) 'logman.exe'
    $Tracerpt = Join-Path ([Environment]::SystemDirectory) 'tracerpt.exe'
    foreach ($Tool in @($Logman, $Tracerpt)) {
        if (-not (Test-Path -LiteralPath $Tool -PathType Leaf)) {
            throw "Outil Windows natif absent : $Tool"
        }
    }
    $Stamp = [DateTime]::UtcNow.ToString('yyyy-MM-dd-HH-mm-ss')
    $Identifier = [Guid]::NewGuid().ToString('N')
    $Session = "SplitergollUsb-$Identifier"
    $Base = Join-Path $Directory "windows-usb-$Stamp-$($Identifier.Substring(0, 8))"
    $Name = [IO.Path]::GetFileName($Base)
    $EtlPattern = '^' + [Regex]::Escape($Name) + '(_[0-9]+)?\.etl$'
    $Etl = "$Base.etl"
    $Xml = "$Base.xml"
    $Summary = "$Base-summary.txt"
    $Providers = @(
        'Microsoft-Windows-USB-USBXHCI',
        'Microsoft-Windows-USB-UCX',
        'Microsoft-Windows-USB-USBHUB3'
    )
    $Started = $false

    foreach ($File in @($Etl, $Xml, $Summary)) {
        if (Test-Path -LiteralPath $File) {
            throw "Un fichier existe déjà, capture refusée : $File"
        }
    }
    $Existing = @(Get-ChildItem -LiteralPath $Directory -Filter "$Name*.etl" -File |
        Where-Object { $_.Name -match $EtlPattern })
    if ($Existing.Count -ne 0) {
        throw 'Une trace du même nom existe déjà, aucun enregistrement lancé.'
    }
    Write-Host "Session temporaire : $Session"
    Write-Host "Arrêt de secours si le terminal est fermé brutalement : logman stop $Session -ets"
    try {
        Invoke-TraceCommand -Command $Logman -Arguments @(
            'start', $Session, '-ets', '-o', $Etl, '-f', 'bin',
            '-nb', '16', '128', '-bs', '64',
            '-p', $Providers[0], '(Default,HeadersBusTrace)'
        )
        $Started = $true
        foreach ($Provider in $Providers[1..2]) {
            Invoke-TraceCommand -Command $Logman -Arguments @(
                'update', 'trace', '-n', $Session, '-ets',
                '-p', $Provider, '(Default,HeadersBusTrace)'
            )
        }
        Write-Host ''
        Write-Host 'CAPTURE ACTIVE : branche maintenant la gauche de V1, avec son firmware normal.'
        Write-Host 'Attends vingt secondes sans taper. Aucun port COM nécessaire.'
        Start-Sleep -Seconds 20
    }
    finally {
        if ($Started) {
            Write-Host 'Arrêt de la capture USB...'
            Invoke-TraceCommand -Command $Logman -Arguments @('stop', $Session, '-ets')
        }
    }
    $Candidates = @(Get-ChildItem -LiteralPath $Directory -Filter "$Name*.etl" -File |
        Where-Object { $_.Name -match $EtlPattern })
    if ($Candidates.Count -ne 1) {
        throw 'Une seule trace ETL complète est attendue. Fichiers conservés, aucun export ambigu.'
    }
    $Etl = $Candidates[0].FullName
    Invoke-TraceCommand -Command $Tracerpt -Arguments @(
        $Etl, '-o', $Xml, '-of', 'XML', '-lr', '-summary', $Summary, '-y'
    )
    if (-not (Test-Path -LiteralPath $Xml -PathType Leaf) -or
        -not (Test-Path -LiteralPath $Summary -PathType Leaf)) {
        throw "Export incomplet. Trace brute conservée : $Etl"
    }
    Write-Host "Événements à transmettre : $Xml"
    Write-Host "Résumé à transmettre : $Summary"
    Write-Host "Trace brute conservée : $Etl"
    Write-Host 'Cette capture observe Windows, sans accès à la décision interne du classifieur.'
}

$ErrorActionPreference = 'Stop'
$Capture = $null
$Serial = $null

try {
    if ([Environment]::OSVersion.Platform -ne [PlatformID]::Win32NT) {
        throw 'Ce lecteur est réservé à Windows.'
    }
    $WorkingDirectory = Get-Location
    if ($WorkingDirectory.Provider.Name -ne 'FileSystem') {
        throw 'Place-toi dans un dossier de fichiers avant de lancer la capture.'
    }
    $TraceRoot = $WorkingDirectory.ProviderPath
    if ($UsbEvents) {
        Capture-UsbEvents -Directory $TraceRoot
        exit 0
    }
    if ([IO.Ports.SerialPort]::GetPortNames() -notcontains $Port) {
        throw "Port absent : $Port"
    }
    $Serial = New-Object IO.Ports.SerialPort $Port, 115200, 'None', 8, 'One'
    $Serial.ReadTimeout = 1000
    $Serial.WriteTimeout = 1000
    $Serial.DtrEnable = $true
    $Serial.NewLine = "`n"
    $Serial.Open()
    $Stamp = [DateTime]::UtcNow.ToString('yyyy-MM-dd-HH-mm-ss')
    $Suffix = [Guid]::NewGuid().ToString('N').Substring(0, 8)
    $CapturePrefix = if ($Profile) { 'windows-ble-profile' } elseif ($Ble) { 'windows-ble' } else { 'windows' }
    $Capture = Join-Path $TraceRoot "$CapturePrefix-$Stamp-$Suffix.txt"
    $Lines = New-Object 'System.Collections.Generic.List[string]'
    $Lines.Add("# host=windows collected_at_utc=$([DateTime]::UtcNow.ToString('o'))")
    Start-Sleep -Seconds 1
    $SerialCommand = 'D'
    if ($Profile) {
        $SerialCommand = switch ($Profile) { 'status' { 'S' } 'free' { 'F' } default { $Profile } }
        Write-Host "Action Bluetooth explicite : $Profile. Aucun appairage effacé."
    }
    $Serial.Write($SerialCommand)
    $Deadline = [DateTime]::UtcNow.AddSeconds(15)
    $Finished = $false
    while ([DateTime]::UtcNow -lt $Deadline) {
        try { $Line = $Serial.ReadLine().TrimEnd("`r") }
        catch [TimeoutException] { continue }
        $Lines.Add($Line)
        if (($Profile -and $Line -eq 'BLEPROFILE END') -or (-not $Profile -and -not $Ble -and $Line -eq 'USBTRACE END') -or ($Ble -and $Line -match '^BLETRACE END attrs=[0-9]+$')) { $Finished = $true; break }
    }
    $Utf8 = New-Object Text.UTF8Encoding $false
    [IO.File]::WriteAllLines($Capture, $Lines, $Utf8)
    if (-not $Finished -or $Lines.Count -lt 4) { throw 'Capture incomplète ou mauvais firmware/port.' }
    if ($Profile) {
        Test-BleProfile -Lines $Lines -Command $SerialCommand
        Write-Host "Réponse conservée : $Capture"
        $Lines | Select-Object -Skip 1 | ForEach-Object { Write-Host $_ }
        if ($Lines[$Lines.Count - 2] -notmatch ' status=ok ') {
            throw 'Sélection refusée : busy = saisie en cours (ancien firmware : hôte connecté), full = aucun emplacement libre.'
        }
        exit 0
    }
    if ($Ble) {
        Test-BleCapture -Lines $Lines
        Write-Host "Capture complète : $Capture"
        $SessionLabels = @{}
        foreach ($Line in $Lines) {
            if ($Line -match '^BLETRACE HOST session=([0-9]+) profile=(-1|[0-9]) connected=([01])$') {
                $HostSession = $Matches[1]
                $HostProfile = if ($Matches[2] -eq '-1') { 'inconnu' } else { $Matches[2] }
                $HostConnected = if ($Matches[3] -eq '1') { 'connecté' } else { 'déconnecté' }
                $SessionLabels[$HostSession] = ", emplacement $HostProfile, $HostConnected"
            }
        }
        $Reports = @($Lines | Where-Object { $_ -match '^BLETRACE OS ' })
        foreach ($Report in $Reports) {
            if ($Report -match '^BLETRACE OS session=([0-9]+) result=(macos|windows|unknown|ambiguous) ') {
                $Session = $Matches[1]
                $Label = switch ($Matches[2]) {
                    'macos' { 'hypothèse macOS' }
                    'windows' { 'hypothèse Windows' }
                    'ambiguous' { 'indices contradictoires, résultat ambigu' }
                    default { 'indices insuffisants, OS inconnu' }
                }
                Write-Host "Signature Bluetooth, session ${Session}$($SessionLabels[$Session]) : $Label"
            }
        }
        if ($Reports.Count -eq 0) { Write-Host 'Classification absente : ancien firmware de capture' }
        Write-Host 'Le diagnostic Bluetooth ne sélectionne aucun OS.'
        exit 0
    }
    if ($Lines[1] -notmatch '^USBTRACE BEGIN version=1 side=left count=([0-9]+) dropped=0 rejected=0 uptime_ms=[0-9]+$') {
        throw 'Capture refusée : format incorrect ou pertes signalées.'
    }
    $Expected = [int]$Matches[1]
    $HasActive = $Lines.Count -eq $Expected + 5
    $HasClassification = $Lines.Count -eq $Expected + 4 -or $HasActive
    if ($Expected -lt 1 -or $Expected -gt 256 -or
        ($Lines.Count -ne $Expected + 3 -and -not $HasClassification)) {
        throw 'Capture refusée : nombre de requêtes incorrect.'
    }
    for ($Index = 0; $Index -lt $Expected; $Index++) {
        if ($Lines[$Index + 2] -notmatch '^USBTRACE SETUP ([0-9]+) [0-9]+ [0-9a-f]{2} [0-9a-f]{2} [0-9a-f]{4} [0-9a-f]{4} [0-9a-f]{4}$' -or
            [int]$Matches[1] -ne $Index) {
            throw 'Capture refusée : requêtes manquantes ou désordonnées.'
        }
    }
    $Result = $null
    if ($HasClassification) {
        if ($Lines[$Expected + 2] -notmatch '^USBTRACE OS result=(macos|windows|unknown|ambiguous) evidence=[0-9a-f]{4}$') {
            throw 'Capture refusée : diagnostic OS incorrect.'
        }
        $Result = $Matches[1]
    }
    $Active = $null
    if ($HasActive) {
        if ($Lines[$Expected + 3] -notmatch '^USBTRACE ACTIVE result=(macos|windows|unknown)$') {
            throw 'Capture refusée : sélection automatique incorrecte.'
        }
        $Active = $Matches[1]
    }
    Write-Host "Capture complète : $Capture"
    switch ($Result) {
        'macos' { Write-Host 'Signature USB : hypothèse macOS' }
        'windows' { Write-Host 'Signature USB : hypothèse Windows' }
        'ambiguous' { Write-Host 'Signature USB : indices contradictoires, résultat ambigu' }
        'unknown' { Write-Host 'Signature USB : indices insuffisants, OS inconnu' }
        default { Write-Host 'Classification absente : ancien firmware de capture' }
    }
    switch ($Active) {
        'macos' { Write-Host 'Keymap automatique appliquée : macOS' }
        'windows' { Write-Host 'Keymap automatique appliquée : Windows + gaming' }
        'unknown' { Write-Host 'Keymap automatique en attente : OS inconnu, saisie bloquée.' }
        default { Write-Host 'La keymap reste inchangée. Ce diagnostic ne sélectionne aucun OS.' }
    }
}
catch {
    if ($UsbEvents) {
        Write-Error "$_ Les fichiers windows-usb déjà créés restent dans le dossier courant."
    } else {
        Write-Error "$_ Données conservées si disponibles : $Capture"
    }
    exit 1
}
finally {
    if ($null -ne $Serial) { $Serial.Dispose() }
}
