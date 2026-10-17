# Lit les en-têtes USB mémorisés par la centrale, avec le port série natif de Windows.
# Reçoit -Port COMx. Aucun flash, reset, logiciel compagnon ou pilote supplémentaire.
param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern('^COM[1-9][0-9]*$')]
    [string]$Port
)

$ErrorActionPreference = 'Stop'
$Repository = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$TraceRoot = Join-Path $Repository '.local/usb-traces'
$Capture = $null
$Serial = $null

try {
    if ([Environment]::OSVersion.Platform -ne [PlatformID]::Win32NT) {
        throw 'Ce lecteur est réservé à Windows.'
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
    New-Item -ItemType Directory -Force -Path $TraceRoot | Out-Null
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
    if ($Expected -lt 1 -or $Expected -gt 256 -or $Lines.Count -ne $Expected + 3) {
        throw 'Capture refusée : nombre de requêtes incorrect.'
    }
    for ($Index = 0; $Index -lt $Expected; $Index++) {
        if ($Lines[$Index + 2] -notmatch '^USBTRACE SETUP ([0-9]+) [0-9]+ [0-9a-f]{2} [0-9a-f]{2} [0-9a-f]{4} [0-9a-f]{4} [0-9a-f]{4}$' -or
            [int]$Matches[1] -ne $Index) {
            throw 'Capture refusée : requêtes manquantes ou désordonnées.'
        }
    }
    Write-Host "Capture complète : $Capture"
}
catch {
    Write-Error "$_ Données conservées si disponibles : $Capture"
    exit 1
}
finally {
    if ($null -ne $Serial) { $Serial.Dispose() }
}
