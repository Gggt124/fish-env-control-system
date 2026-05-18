param(
    [string]$IdfPath = $env:IDF_PATH
)

$ErrorActionPreference = "Stop"

chcp 65001 > $null
$env:PYTHONUTF8 = "1"

if (-not $IdfPath) {
    $defaultIdfPath = "C:\esp-idf"
    if (Test-Path (Join-Path $defaultIdfPath "export.ps1")) {
        $IdfPath = $defaultIdfPath
    } else {
        throw "ESP-IDF path not set. Set IDF_PATH or pass -IdfPath `"C:\path\to\esp-idf`"."
    }
}

$exportScript = Join-Path $IdfPath "export.ps1"
if (-not (Test-Path $exportScript)) {
    throw "ESP-IDF export script not found: $exportScript"
}

& $exportScript

idf.py --version
idf.py build
