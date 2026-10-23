# Charge seulement les validateurs du lecteur, sans exécuter les commandes série ou ETW.
param([Parameter(Mandatory = $true)][string]$Manifest)
$ErrorActionPreference = 'Stop'
$Tokens = $null
$ParseErrors = $null
$Source = Join-Path $PSScriptRoot '../capture.ps1'
$Ast = [System.Management.Automation.Language.Parser]::ParseFile($Source, [ref]$Tokens, [ref]$ParseErrors)
if ($ParseErrors.Count) { throw 'Script PowerShell invalide.' }
foreach ($Name in @('Test-BleProfile', 'Test-BleCapture')) {
    $Definition = $Ast.Find({ param($Node) $Node -is [System.Management.Automation.Language.FunctionDefinitionAst] -and $Node.Name -eq $Name }, $true)
    if (-not $Definition) { throw "Validateur absent : $Name" }
    Invoke-Expression $Definition.Extent.Text
}
$Count = 0
$Cases = Get-Content -Raw -LiteralPath $Manifest | ConvertFrom-Json
foreach ($Case in $Cases) {
    if ($Case.protocol -notin @('profile', 'ble')) { continue }
    $Lines = New-Object 'System.Collections.Generic.List[string]'
    $Lines.AddRange([IO.File]::ReadAllLines($Case.path))
    $Accepted = $true
    try {
        if ($Case.protocol -eq 'profile') { Test-BleProfile -Lines $Lines -Command $Case.command }
        else { Test-BleCapture -Lines $Lines }
    } catch { $Accepted = $false }
    if ($Accepted -ne $Case.accepted) { throw "Validation PowerShell inattendue : $($Case.name)" }
    $Count++
}
Write-Output "PowerShell : $Count cas de protocole passent, sans acces materiel."
