<#
.SYNOPSIS
    Flash firmware à¸¥à¸‡ ESP32 à¹à¸¥à¹‰à¸§à¹à¸ªà¸”à¸‡ AP password
.PARAMETER Port
    COM port à¹€à¸Šà¹ˆà¸™ COM5 à¸–à¹‰à¸²à¹„à¸¡à¹ˆà¸£à¸°à¸šà¸¸à¸ˆà¸° auto-detect à¸«à¸£à¸·à¸­à¸–à¸²à¸¡
#>
param(
    [string]$Port = ""
)

$ErrorActionPreference = "Stop"
chcp 65001 > $null

# Script à¸­à¸¢à¸¹à¹ˆà¹ƒà¸™ flash-package/tools/ â†’ firmware à¸­à¸¢à¸¹à¹ˆà¹ƒà¸™ flash-package/firmware/
$ScriptDir   = $PSScriptRoot
$PackageRoot = Split-Path $ScriptDir -Parent
$FirmwareDir = Join-Path $PackageRoot "firmware"

$Bins = [ordered]@{
    "0x1000"  = Join-Path $FirmwareDir "bootloader.bin"
    "0x10000" = Join-Path $FirmwareDir "partition-table.bin"
    "0x17000" = Join-Path $FirmwareDir "ota_data_initial.bin"
    "0x20000" = Join-Path $FirmwareDir "fish_pump_relay_timer_control.bin"
}

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  Fish Pump Relay Timer â€” Flash Installer  " -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# --- à¸•à¸£à¸§à¸ˆà¸ªà¸­à¸š Python ---
try {
    $pyVer = python --version 2>&1
    Write-Host "[OK] Python: $pyVer" -ForegroundColor Green
} catch {
    Write-Host "ERROR: Python à¹„à¸¡à¹ˆà¸žà¸š à¸à¸£à¸¸à¸“à¸²à¸•à¸´à¸”à¸•à¸±à¹‰à¸‡à¸ˆà¸²à¸ https://python.org" -ForegroundColor Red
    Read-Host "à¸à¸” Enter à¹€à¸žà¸·à¹ˆà¸­à¸›à¸´à¸”"
    exit 1
}

# --- à¸•à¸´à¸”à¸•à¸±à¹‰à¸‡ esptool à¸–à¹‰à¸²à¸¢à¸±à¸‡à¹„à¸¡à¹ˆà¸¡à¸µ ---
$esptoolVer = python -m esptool version 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "[...] à¸•à¸´à¸”à¸•à¸±à¹‰à¸‡ esptool..." -ForegroundColor Yellow
    python -m pip install esptool --quiet
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: à¸•à¸´à¸”à¸•à¸±à¹‰à¸‡ esptool à¹„à¸¡à¹ˆà¸ªà¸³à¹€à¸£à¹‡à¸ˆ" -ForegroundColor Red
        Read-Host "à¸à¸” Enter à¹€à¸žà¸·à¹ˆà¸­à¸›à¸´à¸”"; exit 1
    }
    Write-Host "[OK] esptool installed" -ForegroundColor Green
} else {
    Write-Host "[OK] esptool ready" -ForegroundColor Green
}

# --- à¸•à¸£à¸§à¸ˆà¸ªà¸­à¸š binary à¸„à¸£à¸š ---
foreach ($entry in $Bins.GetEnumerator()) {
    if (-not (Test-Path $entry.Value)) {
        Write-Host "ERROR: à¹„à¸¡à¹ˆà¸žà¸š $($entry.Value)" -ForegroundColor Red
        Read-Host "à¸à¸” Enter à¹€à¸žà¸·à¹ˆà¸­à¸›à¸´à¸”"; exit 1
    }
}

# --- Auto-detect / à¸–à¸²à¸¡ COM port ---
if (-not $Port) {
    $ports = [System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object
    if ($ports.Count -eq 1) {
        $Port = $ports[0]
        Write-Host "[AUTO] à¸•à¸£à¸§à¸ˆà¸žà¸š COM port: $Port" -ForegroundColor Green
    } elseif ($ports.Count -gt 1) {
        Write-Host "à¸žà¸šà¸«à¸¥à¸²à¸¢ COM port:" -ForegroundColor Yellow
        $ports | ForEach-Object { Write-Host "  - $_" }
        $Port = Read-Host "à¸à¸£à¸­à¸ COM port (à¹€à¸Šà¹ˆà¸™ COM5)"
    } else {
        Write-Host "à¹„à¸¡à¹ˆà¸žà¸š COM port à¸à¸£à¸¸à¸“à¸²à¹€à¸ªà¸µà¸¢à¸š ESP32 à¹à¸¥à¹‰à¸§à¸à¸” Enter"
        Read-Host "..."
        $ports = [System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object
        $Port  = $ports | Select-Object -First 1
        if (-not $Port) {
            Write-Host "ERROR: à¸¢à¸±à¸‡à¹„à¸¡à¹ˆà¸žà¸š COM port" -ForegroundColor Red
            Read-Host "à¸à¸” Enter à¹€à¸žà¸·à¹ˆà¸­à¸›à¸´à¸”"; exit 1
        }
    }
}

Write-Host ""
Write-Host "Flash à¹„à¸›à¸—à¸µà¹ˆ $Port ..." -ForegroundColor Cyan
Write-Host ""

# --- Flash ---
$flashArgs = @(
    "--chip", "esp32",
    "--port", $Port,
    "--baud", "230400",
    "write_flash",
    "--flash-mode", "dio",
    "--flash-freq", "40m",
    "--flash-size", "4MB",
    "0x1000",  $Bins["0x1000"],
    "0x10000", $Bins["0x10000"],
    "0x17000", $Bins["0x17000"],
    "0x20000", $Bins["0x20000"]
)

python -m esptool @flashArgs
if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "ERROR: Flash à¹„à¸¡à¹ˆà¸ªà¸³à¹€à¸£à¹‡à¸ˆ à¸•à¸£à¸§à¸ˆà¸ªà¸­à¸š:" -ForegroundColor Red
    Write-Host "  - à¹€à¸ªà¸µà¸¢à¸š USB à¸–à¸¹à¸à¸•à¹‰à¸­à¸‡?" -ForegroundColor Yellow
    Write-Host "  - COM port à¸–à¸¹à¸à¸•à¹‰à¸­à¸‡? ($Port)" -ForegroundColor Yellow
    Write-Host "  - à¸¥à¸­à¸‡à¸–à¸­à¸”-à¹€à¸ªà¸µà¸¢à¸šà¸ªà¸²à¸¢ USB à¹à¸¥à¹‰à¸§à¸£à¸±à¸™ FLASH.bat à¹ƒà¸«à¸¡à¹ˆ" -ForegroundColor Yellow
    Read-Host "à¸à¸” Enter à¹€à¸žà¸·à¹ˆà¸­à¸›à¸´à¸”"; exit 1
}

Write-Host ""
Write-Host "[OK] Flash à¸ªà¸³à¹€à¸£à¹‡à¸ˆ!" -ForegroundColor Green
Write-Host ""

# --- à¸­à¹ˆà¸²à¸™ MAC à¹à¸¥à¸°à¸„à¸³à¸™à¸§à¸“ AP Password ---
Write-Host "à¸à¸³à¸¥à¸±à¸‡à¸­à¹ˆà¸²à¸™ MAC address à¸‚à¸­à¸‡à¸šà¸­à¸£à¹Œà¸”..." -ForegroundColor Cyan
Start-Sleep -Milliseconds 1500

$macOutput = python -m esptool --chip esp32 --baud 230400 --port $Port read_mac 2>&1
$macLine   = $macOutput | Select-String -Pattern "MAC:\s*([0-9a-fA-F:]{17})" | Select-Object -First 1

if (-not $macLine) {
    Write-Host "WARNING: à¸­à¹ˆà¸²à¸™ MAC à¹„à¸¡à¹ˆà¹„à¸”à¹‰" -ForegroundColor Yellow
    Write-Host "à¸£à¸±à¸™ README_password.bat à¸—à¸µà¸«à¸¥à¸±à¸‡à¹€à¸žà¸·à¹ˆà¸­à¸”à¸¹ password" -ForegroundColor Gray
} else {
    $macStr   = $macLine.Matches.Groups[1].Value.Trim()
    $macBytes = $macStr -split ":" | ForEach-Object { [Convert]::ToInt32($_.Trim(), 16) }
    $salt     = 0x5A
    $password = "{0:X2}{1:X2}{2:X2}{3:X2}" -f `
        ($macBytes[2] -bxor $salt), ($macBytes[3] -bxor $salt),
        ($macBytes[4] -bxor $salt), ($macBytes[5] -bxor $salt)

    Write-Host ""
    Write-Host "============================================" -ForegroundColor Green
    Write-Host "  Flash à¹€à¸ªà¸£à¹‡à¸ˆà¹à¸¥à¹‰à¸§!                         " -ForegroundColor Green
    Write-Host "============================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "  Wi-Fi SSID    : FishPump-Setup           " -ForegroundColor White
    Write-Host "  Wi-Fi Password: $password                " -ForegroundColor Yellow
    Write-Host "  Web Dashboard : http://192.168.4.1       " -ForegroundColor White
    Write-Host "  Login         : admin / admin123         " -ForegroundColor White
    Write-Host ""
    Write-Host "  >>> à¸šà¸±à¸™à¸—à¸¶à¸ Password à¹„à¸§à¹‰à¸”à¹‰à¸§à¸¢! <<<        " -ForegroundColor Red
    Write-Host "============================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Password à¸‚à¸¶à¹‰à¸™à¸à¸±à¸šà¸šà¸­à¸£à¹Œà¸”à¸™à¸µà¹‰à¹€à¸—à¹ˆà¸²à¸™à¸±à¹‰à¸™" -ForegroundColor Gray
    Write-Host "à¸–à¹‰à¸²à¸¥à¸·à¸¡: à¸£à¸±à¸™ README_password.bat (à¸•à¹‰à¸­à¸‡à¹€à¸ªà¸µà¸¢à¸š USB)" -ForegroundColor Gray
}

Write-Host ""
Read-Host "à¸à¸” Enter à¹€à¸žà¸·à¹ˆà¸­à¸›à¸´à¸”"
