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

    if ($Lines.Count -lt 5 -or $Lines[1] -notmatch '^BLETRACE BEGIN version=1 side=left count=([0-9]+) dropped=0 rejected=0 uptime_ms=[0-9]+ session=([0-9]+) connected=([01])$') {
        throw 'Capture BLE refusée : format incorrect, aucune connexion ou pertes signalées.'
    }
    $Expected = [int]$Matches[1]
    $ExpectedSession = [int]$Matches[2]
    $ExpectedConnected = [int]$Matches[3]
    if ($Expected -lt 1 -or $Expected -gt 256 -or $Lines.Count -lt $Expected + 4) {
        throw 'Capture BLE refusée : nombre incorrect.'
    }
    $Session = 0
    $Connected = $false
    for ($Index = 0; $Index -lt $Expected; $Index++) {
        if ($Lines[$Index + 2] -notmatch '^BLETRACE EVENT ([0-9]+) [0-9]+ ([0-9]+) (CONNECTED|DISCONNECTED|SECURITY|PARAM_REQUEST|PARAM_UPDATED|MTU|FEATURES|ATT)( [0-9a-f]{8}){5}$' -or [int]$Matches[1] -ne $Index) {
            throw 'Capture BLE refusée : événements manquants ou désordonnés.'
        }
        $EventSession = [int]$Matches[2]
        $Kind = $Matches[3]
        if ($Kind -eq 'CONNECTED') {
            if ($Connected -or $EventSession -ne $Session + 1) { throw 'Sessions BLE incohérentes.' }
            $Connected = $true
            $Session++
        } elseif (-not $Connected -or $EventSession -ne $Session) { throw 'Session BLE absente.' }
        if ($Kind -eq 'DISCONNECTED') { $Connected = $false }
    }
    $Attributes = 0
    $Seen = @{}
    for ($Index = $Expected + 2; $Index -lt $Lines.Count - 1; $Index++) {
        if ($Lines[$Index] -notmatch '^BLETRACE ATTR ([0-9a-f]{4}) ([0-9a-f]{4}|[0-9a-f]{8}|[0-9a-f]{8}(-[0-9a-f]{4}){3}-[0-9a-f]{12})$') {
            throw 'Table GATT BLE invalide.'
        }
        $Handle = $Matches[1]
        if ($Seen.ContainsKey($Handle)) { throw 'Handle GATT dupliqué.' }
        $Seen[$Handle] = $true
        $Attributes++
    }
    if ($Lines[$Lines.Count - 1] -notmatch '^BLETRACE END attrs=([0-9]+)$' -or
        [int]$Matches[1] -ne $Attributes -or $Attributes -lt 1 -or $Attributes -gt 512 -or
        $Session -ne $ExpectedSession -or [int]$Connected -ne $ExpectedConnected) {
        throw 'Capture BLE tronquée ou sessions incohérentes.'
    }
}

# Vérifie la réponse du contrôle avant de présenter le choix ou son refus.
function Test-BleProfile {
    param([System.Collections.Generic.List[string]]$Lines, [string]$Command)

    if ($Lines.Count -lt 5 -or $Lines[1] -notmatch '^BLEPROFILE BEGIN version=1 count=([0-9]+) active=([0-9]+) save_delay_ms=[0-9]+$') {
        throw 'Réponse incorrecte : firmware ble-trace avec contrôle des emplacements requis.'
    }
    $Expected = [int]$Matches[1]
    $Active = [int]$Matches[2]
    if ($Expected -lt 1 -or $Expected -gt 10 -or $Active -ge $Expected -or $Lines.Count -ne $Expected + 4) {
        throw 'Réponse des emplacements tronquée ou incohérente.'
    }
    for ($Index = 0; $Index -lt $Expected; $Index++) {
        if ($Lines[$Index + 2] -notmatch '^BLEPROFILE SLOT index=([0-9]+) paired=[01] connected=[01]$' -or [int]$Matches[1] -ne $Index) {
            throw 'Liste des emplacements incorrecte.'
        }
    }
    if ($Lines[$Expected + 2] -notmatch '^BLEPROFILE RESULT command=([SF0-9]) status=(ok|busy|full|invalid|failed) selected=([0-9]+) changed=[01] error=-?[0-9]+$' -or
        $Matches[1] -ne $Command -or [int]$Matches[3] -ne $Active -or $Lines[$Expected + 3] -ne 'BLEPROFILE END') {
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
            throw 'Sélection refusée : busy = hôte connecté, full = aucun emplacement libre.'
        }
        exit 0
    }
    if ($Ble) {
        Test-BleCapture -Lines $Lines
        Write-Host "Capture complète : $Capture"
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
