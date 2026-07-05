<#
.SYNOPSIS
    Build fish_pump_relay_timer_control firmware for a specified target board.
.PARAMETER Target
    Target chip: 'esp32' (classic DevKit V1 30-pin) or 'esp32s3' (DevKitC-1 N16R8). Default: esp32
.PARAMETER IdfPath
    Path to ESP-IDF. Falls back to IDF_PATH env var then known paths.
.PARAMETER IdfToolsPath
    Path to ESP-IDF tools. Falls back to IDF_TOOLS_PATH env var then known paths.
.PARAMETER FullClean
    Delete build-<Target>/ before building.
.EXAMPLE
    .\scripts\build.ps1
    .\scripts\build.ps1 -Target esp32
    .\scripts\build.ps1 -Target esp32s3
    .\scripts\build.ps1 -Target esp32s3 -FullClean
#>
param(
    [ValidateSet("esp32", "esp32s3")]
    [string]$Target = "esp32",
    [string]$IdfPath = $env:IDF_PATH,
    [string]$IdfToolsPath = $env:IDF_TOOLS_PATH,
    [switch]$FullClean
)

$ErrorActionPreference = "Stop"
chcp 65001 > $null
$env:PYTHONUTF8 = "1"

$Profiles = @{
    esp32   = @{ Chip="esp32";   FlashMode="dio"; FlashFreq="40m"; FlashSize="4MB";  BootOffset="0x1000"; Baud="230400" }
    esp32s3 = @{ Chip="esp32s3"; FlashMode="qio"; FlashFreq="80m"; FlashSize="16MB"; BootOffset="0x0000"; Baud="460800" }
}
$Profile     = $Profiles[$Target]
$ProjectRoot = Split-Path $PSScriptRoot -Parent
$BuildDir    = Join-Path $ProjectRoot "build-$Target"

function Resolve-IdfPath {
    param([string]$Path)
    $candidates = @()
    if ($Path) { $candidates += $Path; $candidates += (Join-Path $Path "esp-idf") }
    $candidates += "C:\esp\v6.0.1\esp-idf"; $candidates += "C:\esp\esp-idf"
    if (Test-Path "C:\esp") {
        $candidates += Get-ChildItem -LiteralPath "C:\esp" -Directory -ErrorAction SilentlyContinue |
            Sort-Object Name -Descending | ForEach-Object { Join-Path $_.FullName "esp-idf" }
    }
    foreach ($c in $candidates) {
        if ($c -and (Test-Path (Join-Path $c "export.ps1"))) { return (Resolve-Path $c).Path }
    }
    return $null
}

function Resolve-IdfToolsPath {
    param([string]$Path)
    $candidates = @()
    if ($Path) { $candidates += $Path }
    $candidates += "C:\Espressif"; $candidates += (Join-Path $HOME ".espressif")
    foreach ($c in $candidates) {
        if ($c -and (Test-Path (Join-Path $c "tools")) -and (Test-Path (Join-Path $c "python_env"))) {
            return (Resolve-Path $c).Path
        }
    }
    return $null
}

$resolvedIdfPath = Resolve-IdfPath -Path $IdfPath
if (-not $resolvedIdfPath) { throw "ESP-IDF not found. Set IDF_PATH or pass -IdfPath." }

$resolvedIdfToolsPath = Resolve-IdfToolsPath -Path $IdfToolsPath
if (-not $resolvedIdfToolsPath) { throw "ESP-IDF tools not found. Set IDF_TOOLS_PATH or pass -IdfToolsPath." }

$env:IDF_PATH       = $resolvedIdfPath
$env:IDF_TOOLS_PATH = $resolvedIdfToolsPath
$exportScript = Join-Path $env:IDF_PATH "export.ps1"
if (-not (Test-Path $exportScript)) { throw "ESP-IDF export.ps1 not found: $exportScript" }
. $exportScript

Write-Host ""
Write-Host "=== Building for target: $Target ===" -ForegroundColor Cyan
Write-Host "    Build dir : $BuildDir" -ForegroundColor Gray
Write-Host "    Chip      : $($Profile.Chip)" -ForegroundColor Gray
Write-Host ""

$SecretsFile = Join-Path $ProjectRoot "components\app_config\secrets.h"
if (-not (Test-Path $SecretsFile)) {
    Write-Host "[build] secrets.h not found -- generating new OTA key..." -ForegroundColor Yellow
    & "$PSScriptRoot\generate_secrets.ps1"
}

idf.py --version

if ($FullClean) {
    Write-Host "[build] FullClean: removing $BuildDir ..." -ForegroundColor Yellow
    if (Test-Path $BuildDir) { idf.py -B "$BuildDir" fullclean }
}

Write-Host "[build] idf.py set-target $Target ..." -ForegroundColor Cyan
idf.py -B "$BuildDir" set-target $Target
if ($LASTEXITCODE -ne 0) { throw "[build] set-target $Target failed (exit $LASTEXITCODE)" }

Write-Host "[build] idf.py build ..." -ForegroundColor Cyan
idf.py -B "$BuildDir" build
if ($LASTEXITCODE -ne 0) { throw "[build] build failed (exit $LASTEXITCODE)" }

Write-Host "[build] Build successful for $Target" -ForegroundColor Green

$BinName    = "fish_pump_relay_timer_control"
$BinPath    = "$BuildDir\$BinName.bin"
$EncPath    = "$BuildDir\${BinName}_${Target}_encrypted.bin"
$MergedPath = "$BuildDir\${BinName}_${Target}_merged.bin"

$configFile = Join-Path $ProjectRoot "components\app_config\secrets.h"
$keyLine    = Select-String -Path $configFile -Pattern 'APP_CONFIG_OTA_ENCRYPTION_KEY\s+"([0-9A-Fa-f]+)"'
if ($keyLine) {
    $keyHex = $keyLine.Matches.Groups[1].Value
    if (Test-Path $BinPath) {
        Write-Host "[build] Generating encrypted OTA binary..." -ForegroundColor Cyan
        python "$PSScriptRoot\encrypt_ota.py" $BinPath $EncPath $keyHex
    } else { Write-Warning "[build] $BinPath not found -- skipping encryption." }
} else { Write-Warning "[build] OTA key not found in secrets.h -- skipping encryption." }

Write-Host "[build] Generating merged binary..." -ForegroundColor Cyan
python -m esptool --chip $Profile.Chip merge-bin `
    -o $MergedPath `
    --flash-mode $Profile.FlashMode `
    --flash-freq $Profile.FlashFreq `
    --flash-size $Profile.FlashSize `
    $Profile.BootOffset "$BuildDir\bootloader\bootloader.bin" `
    0x10000 "$BuildDir\partition_table\partition-table.bin" `
    0x17000 "$BuildDir\ota_data_initial.bin" `
    0x20000 $BinPath

Write-Host ""
Write-Host "[build] OTA encrypted : $EncPath" -ForegroundColor Green
Write-Host "[build] Merged binary : $MergedPath" -ForegroundColor Green
Write-Host ""
Write-Host "=== Build complete: $Target ===" -ForegroundColor Green
Write-Host "After flashing, get AP password with:" -ForegroundColor White
Write-Host "  .\scripts\show_ap_password.ps1 -Port COMx" -ForegroundColor Yellow
Write-Host ""
