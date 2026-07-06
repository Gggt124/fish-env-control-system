<#
.SYNOPSIS
    Flash firmware to ESP32 or ESP32-S3 and show AP password.
.PARAMETER Port
    COM port (e.g. 5). If omitted, auto-detects or prompts.
.PARAMETER Board
    Board profile: 'esp32' or 'esp32s3'. If omitted, shows selection menu.
#>
param(
    [string]$Port  = "",
    [ValidateSet("", "esp32", "esp32s3")]
    [string]$Board = ""
)

$ErrorActionPreference = "Continue"
chcp 65001 > $null

$ScriptDir   = $PSScriptRoot
$PackageRoot = Split-Path $ScriptDir -Parent

$BoardProfiles = @{
    esp32   = @{ Chip="esp32";   FlashMode="dio"; FlashFreq="40m"; FlashSize="4MB";  BootOffset="0x1000"; Baud="230400"; DisplayName="ESP32 Classic DevKit V1 (4 MB)" }
    esp32s3 = @{ Chip="esp32s3"; FlashMode="qio"; FlashFreq="80m"; FlashSize="16MB"; BootOffset="0x0000"; Baud="460800"; DisplayName="ESP32-S3 DevKitC-1 WROOM-1-N16R8 (16 MB)" }
}

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  Fish Pump Relay Timer - Flash Installer   " -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

$esptoolExe = Join-Path $ScriptDir "esptool.exe"
if (-not (Test-Path $esptoolExe)) {
    Write-Host "ERROR: esptool.exe not found in tools/ folder!" -ForegroundColor Red
    Read-Host "Press Enter to exit"; exit 1
}
Write-Host "[OK] esptool ready" -ForegroundColor Green
Write-Host ""

# Board selector menu
if (-not $Board) {
    Write-Host "Select target board:" -ForegroundColor Yellow
    Write-Host "  [1] $($BoardProfiles['esp32'].DisplayName)"   -ForegroundColor White
    Write-Host "  [2] $($BoardProfiles['esp32s3'].DisplayName)" -ForegroundColor White
    Write-Host ""
    $choice = Read-Host "Enter 1 or 2"
    switch ($choice.Trim()) {
        "1"     { $Board = "esp32" }
        "2"     { $Board = "esp32s3" }
        default { Write-Host "Invalid choice." -ForegroundColor Red; Read-Host "Press Enter to exit"; exit 1 }
    }
}

$Profile     = $BoardProfiles[$Board]
$FirmwareDir = Join-Path $PackageRoot "firmware\$Board"
$BinName     = "fish_pump_relay_timer_control"
Write-Host "[OK] Board: $($Profile.DisplayName)" -ForegroundColor Green

# Locate binaries -- boot offset is board-specific
$Bins = [ordered]@{
    $Profile.BootOffset = Join-Path $FirmwareDir "bootloader.bin"
    "0x10000"           = Join-Path $FirmwareDir "partition-table.bin"
    "0x17000"           = Join-Path $FirmwareDir "ota_data_initial.bin"
    "0x20000"           = Join-Path $FirmwareDir "$BinName.bin"
}
foreach ($entry in $Bins.GetEnumerator()) {
    if (-not (Test-Path $entry.Value)) {
        Write-Host "ERROR: Missing: $($entry.Value)" -ForegroundColor Red
        Write-Host "       Run package.ps1 -Target $Board first." -ForegroundColor Yellow
        Read-Host "Press Enter to exit"; exit 1
    }
}
Write-Host "[OK] All firmware files present" -ForegroundColor Green

# Auto-detect / ask COM port
if (-not $Port) {
    $ports = [System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object
    if ($ports.Count -eq 1) {
        $Port = $ports[0]; Write-Host "[AUTO] COM port: $Port" -ForegroundColor Green
    } elseif ($ports.Count -gt 1) {
        Write-Host "Multiple COM ports:" -ForegroundColor Yellow
        $ports | ForEach-Object { Write-Host "  - $_" }
        $Port = Read-Host "Enter COM port number (e.g. 5)"
    } else {
        Write-Host "No COM port found. Plug in the board via USB then press Enter."
        Read-Host "..."
        $ports = [System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object
        $Port  = $ports | Select-Object -First 1
        if (-not $Port) { Write-Host "ERROR: No COM port." -ForegroundColor Red; Read-Host "Press Enter to exit"; exit 1 }
    }
}
if ($Port -match '^\d+$') { $Port = "COM$Port" }

Write-Host ""
Write-Host "Flashing $($Profile.DisplayName) to $Port ..." -ForegroundColor Cyan
Write-Host ""

$flashArgs = @(
    "--chip", $Profile.Chip, "--port", $Port, "--baud", $Profile.Baud,
    "write_flash", "--erase-all",
    "--flash-mode", $Profile.FlashMode, "--flash-freq", $Profile.FlashFreq, "--flash-size", $Profile.FlashSize
)
foreach ($entry in $Bins.GetEnumerator()) { $flashArgs += $entry.Key; $flashArgs += $entry.Value }

& $esptoolExe @flashArgs
if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "ERROR: Flash failed!" -ForegroundColor Red
    Write-Host "  - Check USB cable and COM port ($Port)" -ForegroundColor Yellow
    Write-Host "  - Hold BOOT button, press RESET, release BOOT to enter download mode" -ForegroundColor Yellow
    Read-Host "Press Enter to exit"; exit 1
}

Write-Host ""
Write-Host "[OK] Flash Successful!" -ForegroundColor Green
Write-Host ""

# Read MAC -- chip-aware: uses $Profile.Chip, NOT hardcoded "esp32"
Write-Host "Reading board MAC address..." -ForegroundColor Cyan
Start-Sleep -Milliseconds 1500

$macOutput = & $esptoolExe --chip $Profile.Chip --baud $Profile.Baud --port $Port read_mac 2>&1
$macLine   = $macOutput | Select-String -Pattern "MAC:\s*([0-9a-fA-F:]{17})" | Select-Object -First 1

if (-not $macLine) {
    Write-Host "WARNING: Could not read MAC. Run SHOW_PASSWORD.bat later." -ForegroundColor Yellow
} else {
    $macStr   = $macLine.Matches.Groups[1].Value.Trim()
    $macBytes = $macStr -split ":" | ForEach-Object { [Convert]::ToInt32($_.Trim(), 16) }
    $salt     = 0x5A
    $password = "{0:X2}{1:X2}{2:X2}{3:X2}" -f `
        ($macBytes[2] -bxor $salt), ($macBytes[3] -bxor $salt),
        ($macBytes[4] -bxor $salt), ($macBytes[5] -bxor $salt)

    Write-Host ""
    Write-Host "============================================" -ForegroundColor Green
    Write-Host "  Flash Completed!                          " -ForegroundColor Green
    Write-Host "============================================" -ForegroundColor Green
    Write-Host "  Board         : $($Profile.DisplayName)  " -ForegroundColor White
    Write-Host "  Wi-Fi SSID    : FishPump-Setup           " -ForegroundColor White
    Write-Host "  Wi-Fi Password: $password                " -ForegroundColor Yellow
    Write-Host "  Web Dashboard : http://192.168.4.1       " -ForegroundColor White
    Write-Host "  Login         : admin / admin123         " -ForegroundColor White
    Write-Host ""
    Write-Host "  >>> Please save this Password! <<<        " -ForegroundColor Red
    Write-Host "============================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "This password is unique to this specific board." -ForegroundColor Gray
    Write-Host "If you forget it: Run SHOW_PASSWORD.bat (USB required)" -ForegroundColor Gray
}

Write-Host ""
Read-Host "Press Enter to exit"
