param(
    [string]$IdfPath = $env:IDF_PATH,
    [string]$IdfToolsPath = $env:IDF_TOOLS_PATH,
    [switch]$FullClean
)

$ErrorActionPreference = "Stop"

chcp 65001 > $null
$env:PYTHONUTF8 = "1"

function Resolve-IdfPath {
    param(
        [string]$Path
    )

    $candidates = @()

    if ($Path) {
        $candidates += $Path
        $candidates += (Join-Path $Path "esp-idf")
    }

    $candidates += "C:\esp\v6.0.1\esp-idf"
    $candidates += "C:\esp\esp-idf"

    if (Test-Path "C:\esp") {
        $candidates += Get-ChildItem -LiteralPath "C:\esp" -Directory -ErrorAction SilentlyContinue |
            Sort-Object Name -Descending |
            ForEach-Object { Join-Path $_.FullName "esp-idf" }
    }

    foreach ($candidate in $candidates) {
        if ($candidate -and (Test-Path (Join-Path $candidate "export.ps1"))) {
            return (Resolve-Path $candidate).Path
        }
    }

    return $null
}

function Resolve-IdfToolsPath {
    param(
        [string]$Path
    )

    $candidates = @()

    if ($Path) {
        $candidates += $Path
    }

    $candidates += "C:\Espressif"
    $candidates += (Join-Path $HOME ".espressif")

    foreach ($candidate in $candidates) {
        if ($candidate -and
            (Test-Path (Join-Path $candidate "tools")) -and
            (Test-Path (Join-Path $candidate "python_env"))) {
            return (Resolve-Path $candidate).Path
        }
    }

    return $null
}

$resolvedIdfPath = Resolve-IdfPath -Path $IdfPath
if (-not $resolvedIdfPath) {
    throw "ESP-IDF path not found. Set IDF_PATH or pass -IdfPath `"C:\esp`" or `"C:\path\to\esp-idf`"."
}

$resolvedIdfToolsPath = Resolve-IdfToolsPath -Path $IdfToolsPath
if (-not $resolvedIdfToolsPath) {
    throw "ESP-IDF tools path not found. Set IDF_TOOLS_PATH or pass -IdfToolsPath `"C:\Espressif`"."
}

$env:IDF_PATH = $resolvedIdfPath
$env:IDF_TOOLS_PATH = $resolvedIdfToolsPath
$exportScript = Join-Path $env:IDF_PATH "export.ps1"
if (-not (Test-Path $exportScript)) {
    throw "ESP-IDF export script not found: $exportScript"
}

& $exportScript

idf.py --version
if ($FullClean) {
    idf.py fullclean
}

idf.py build
