param(
    [string]$IdfPath = "C:\esp-idf"
)

$ErrorActionPreference = "Stop"

chcp 65001 > $null
$env:PYTHONUTF8 = "1"

$exportScript = Join-Path $IdfPath "export.ps1"
if (-not (Test-Path $exportScript)) {
    throw "ESP-IDF export script not found: $exportScript"
}

& $exportScript

idf.py --version
idf.py build
