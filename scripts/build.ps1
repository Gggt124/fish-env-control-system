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
.PARAMETER ResetConfig
    Regenerate the target sdkconfig from sdkconfig.defaults files.
.EXAMPLE
    .\scripts\build.ps1
    .\scripts\build.ps1 -Target esp32
    .\scripts\build.ps1 -Target esp32s3
    .\scripts\build.ps1 -Target esp32s3 -FullClean
    .\scripts\build.ps1 -Target esp32s3 -FullClean -ResetConfig
#>
param(
    [ValidateSet("esp32", "esp32s3")]
    [string]$Target = "esp32",
    [string]$IdfPath = $env:IDF_PATH,
    [string]$IdfToolsPath = $env:IDF_TOOLS_PATH,
    [switch]$FullClean,
    [switch]$ResetConfig
)

$ErrorActionPreference = "Stop"
chcp 65001 > $null
$env:PYTHONUTF8 = "1"

$Profiles = @{
    esp32   = @{ Chip="esp32";   FlashMode="dio"; FlashFreq="40m"; FlashSize="4MB";  BootOffset="0x1000"; Baud="230400"; PartitionFile="partitions_esp32.csv";   FlashConfig="CONFIG_ESPTOOLPY_FLASHSIZE_4MB" }
    esp32s3 = @{ Chip="esp32s3"; FlashMode="dio"; FlashFreq="40m"; FlashSize="16MB"; BootOffset="0x0000"; Baud="460800"; PartitionFile="partitions_esp32s3.csv"; FlashConfig="CONFIG_ESPTOOLPY_FLASHSIZE_16MB" }
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

function Invoke-Idf {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Arguments
    )

    & idf.py @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "[build] idf.py failed (exit $LASTEXITCODE): $($Arguments -join ' ')"
    }
}

function Get-CMakeCacheValue {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [string]$Key
    )

    if (-not (Test-Path -LiteralPath $Path)) { return $null }
    $escapedKey = [regex]::Escape($Key)
    $line = Get-Content -LiteralPath $Path |
        Where-Object { $_ -match ("^{0}:[^=]+=([^#]*)" -f $escapedKey) } |
        Select-Object -First 1
    if ($line -and $line -match "^[^:]+:[^=]+=([^#]*)") {
        return $Matches[1].Trim()
    }
    return $null
}

function Get-SdkconfigValue {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [string]$Key
    )

    if (-not (Test-Path -LiteralPath $Path)) { return $null }
    $escapedKey = [regex]::Escape($Key)
    $lines = Get-Content -LiteralPath $Path
    $valueLine = $lines |
        Where-Object { $_ -match ("^{0}=(.*)$" -f $escapedKey) } |
        Select-Object -First 1
    if ($valueLine -and $valueLine -match ("^{0}=(.*)$" -f $escapedKey)) {
        return $Matches[1].Trim().Trim('"')
    }

    $unsetLine = $lines |
        Where-Object { $_ -eq "# $Key is not set" } |
        Select-Object -First 1
    if ($unsetLine) { return "n" }
    return $null
}

function Assert-SdkconfigValue {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [string]$Key,
        [Parameter(Mandatory = $true)]
        [string]$Expected,
        [Parameter(Mandatory = $true)]
        [string]$Target
    )

    $actual = Get-SdkconfigValue -Path $Path -Key $Key
    if ($null -eq $actual) {
        throw "[build] $Target config missing $Key (expected '$Expected')."
    }
    if ($actual -ne $Expected) {
        throw "[build] $Target config mismatch for $Key`: expected '$Expected', actual '$actual'."
    }
}

function Assert-TargetConfig {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [string]$Target,
        [Parameter(Mandatory = $true)]
        [hashtable]$Profile
    )

    if (-not (Test-Path -LiteralPath $Path)) {
        throw "[build] Generated sdkconfig not found: $Path"
    }

    Assert-SdkconfigValue -Path $Path -Key "CONFIG_IDF_TARGET" -Expected $Target -Target $Target
    Assert-SdkconfigValue -Path $Path -Key "CONFIG_PARTITION_TABLE_CUSTOM_FILENAME" -Expected $Profile.PartitionFile -Target $Target
    Assert-SdkconfigValue -Path $Path -Key $Profile.FlashConfig -Expected "y" -Target $Target

    if ($Target -eq "esp32s3") {
        Assert-SdkconfigValue -Path $Path -Key "CONFIG_SPIRAM" -Expected "y" -Target $Target
        Assert-SdkconfigValue -Path $Path -Key "CONFIG_SPIRAM_MODE_OCT" -Expected "y" -Target $Target
        Assert-SdkconfigValue -Path $Path -Key "CONFIG_SPIRAM_SPEED_80M" -Expected "y" -Target $Target
        Assert-SdkconfigValue -Path $Path -Key "CONFIG_SPIRAM_USE_CAPS_ALLOC" -Expected "y" -Target $Target
        Assert-SdkconfigValue -Path $Path -Key "CONFIG_SPIRAM_USE_MALLOC" -Expected "n" -Target $Target
    }
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

$ConfigPath  = Join-Path $ProjectRoot "sdkconfig.$Target"
$DefaultsPath = Join-Path $ProjectRoot "sdkconfig.defaults"
$CmakeDefines = @(
    "-DIDF_TARGET=$Target",
    "-DSDKCONFIG=$ConfigPath",
    "-DSDKCONFIG_DEFAULTS=$DefaultsPath"
)

Write-Host "    Config    : $ConfigPath" -ForegroundColor Gray
Write-Host "    Defaults  : $DefaultsPath (+ .$Target when present)" -ForegroundColor Gray

Invoke-Idf @("--version")

if ($FullClean -or $ResetConfig) {
    Write-Host "[build] Cleaning $BuildDir ..." -ForegroundColor Yellow
    if (Test-Path $BuildDir) {
        Invoke-Idf @("-B", $BuildDir, "fullclean")
    }
}

if ($ResetConfig) {
    foreach ($configFile in @($ConfigPath, "$ConfigPath.old")) {
        if (Test-Path -LiteralPath $configFile) {
            Write-Host "[build] ResetConfig: removing $configFile ..." -ForegroundColor Yellow
            Remove-Item -LiteralPath $configFile -Force
        }
    }
}

$CacheFile = Join-Path $BuildDir "CMakeCache.txt"
$CachedTarget = Get-CMakeCacheValue -Path $CacheFile -Key "IDF_TARGET"
if ($CachedTarget -and $CachedTarget -ne $Target) {
    throw "[build] Build directory target mismatch: '$BuildDir' contains '$CachedTarget', requested '$Target'. Use -FullClean."
}

if (-not (Test-Path $CacheFile)) {
    Write-Host "[build] idf.py set-target $Target ..." -ForegroundColor Cyan
    Invoke-Idf (@("-B", $BuildDir) + $CmakeDefines + @("set-target", $Target))
} elseif (-not $CachedTarget) {
    Write-Host "[build] CMake cache has no target; reinitializing $Target ..." -ForegroundColor Yellow
    Invoke-Idf @("-B", $BuildDir, "fullclean")
    Invoke-Idf (@("-B", $BuildDir) + $CmakeDefines + @("set-target", $Target))
} else {
    Write-Host "[build] Target cache verified: $Target ..." -ForegroundColor Cyan
}

Write-Host "[build] Validating generated sdkconfig ..." -ForegroundColor Cyan
Assert-TargetConfig -Path $ConfigPath -Target $Target -Profile $Profile
Write-Host "[build] sdkconfig profile verified: $Target ..." -ForegroundColor Green

Write-Host "[build] idf.py build ..." -ForegroundColor Cyan
Invoke-Idf (@("-B", $BuildDir) + $CmakeDefines + @("build"))

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
Write-Host "  [DEV]      .\scripts\show_ap_password.ps1 -Port COMx" -ForegroundColor Yellow
Write-Host "  [CUSTOMER] Run SHOW_PASSWORD.bat from the flash-package folder" -ForegroundColor Gray
Write-Host ""
