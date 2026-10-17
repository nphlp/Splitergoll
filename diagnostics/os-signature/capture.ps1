# Capture les en-têtes mémorisés du clavier (-Port COMx) ou les événements USB Windows (-UsbEvents).
# Fonctionne seul, sans dépôt ni arborescence imposée, avec les fichiers dans le dossier courant.
# Le mode UsbEvents observe le firmware normal sans port série et nécessite un terminal administrateur.
# Aucun flash, reset, logiciel compagnon ou pilote supplémentaire.
[CmdletBinding(DefaultParameterSetName = 'Serial')]
param(
    [Parameter(Mandatory = $true, ParameterSetName = 'Serial')]
    [ValidatePattern('^COM[1-9][0-9]*$')]
    [string]$Port,

    [Parameter(Mandatory = $true, ParameterSetName = 'UsbEvents')]
    [switch]$UsbEvents
)

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
    $Capture = Join-Path $TraceRoot "windows-$Stamp-$Suffix.txt"
    $Lines = New-Object 'System.Collections.Generic.List[string]'
    $Lines.Add("# host=windows collected_at_utc=$([DateTime]::UtcNow.ToString('o'))")
    Start-Sleep -Seconds 1
    $Serial.Write('D')
    $Deadline = [DateTime]::UtcNow.AddSeconds(15)
    $Finished = $false
    while ([DateTime]::UtcNow -lt $Deadline) {
        try { $Line = $Serial.ReadLine().TrimEnd("`r") }
        catch [TimeoutException] { continue }
        $Lines.Add($Line)
        if ($Line -eq 'USBTRACE END') { $Finished = $true; break }
    }
    $Utf8 = New-Object Text.UTF8Encoding $false
    [IO.File]::WriteAllLines($Capture, $Lines, $Utf8)
    if (-not $Finished -or $Lines.Count -lt 4) { throw 'Capture incomplète ou mauvais firmware/port.' }
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
