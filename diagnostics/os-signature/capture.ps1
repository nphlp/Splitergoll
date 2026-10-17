# Lit les en-têtes USB mémorisés par la centrale, avec le port série natif de Windows.
# Reçoit -Port COMx et fonctionne seul, sans dépôt ni arborescence imposée.
# Écrit la capture directement dans le dossier courant de PowerShell.
# Aucun flash, reset, logiciel compagnon ou pilote supplémentaire.
param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern('^COM[1-9][0-9]*$')]
    [string]$Port
)

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
        'unknown' { Write-Host 'Keymap automatique en attente, saisie bloquée tant que l’OS reste inconnu.' }
        default { Write-Host 'La keymap reste inchangée. Ce diagnostic ne sélectionne aucun OS.' }
    }
}
catch {
    Write-Error "$_ Données conservées si disponibles : $Capture"
    exit 1
}
finally {
    if ($null -ne $Serial) { $Serial.Dispose() }
}
