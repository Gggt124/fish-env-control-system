<#
.SYNOPSIS
    Flash firmware to ESP32 and show AP password
.PARAMETER Port
    COM port (e.g. COM5). If not specified, it will auto-detect or ask.
#>
param(
    [string]$Port = ""
)

$ErrorActionPreference = "Continue"
chcp 65001 > $null

# Script is in flash-package/tools/ -> firmware is in flash-package/firmware/
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
Write-Host "  Fish Pump Relay Timer - Flash Installer   " -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# --- Check esptool.exe ---
$esptoolExe = Join-Path $ScriptDir "esptool.exe"
if (-not (Test-Path $esptoolExe)) {
    Write-Host "ERROR: esptool.exe not found in tools/ folder!" -ForegroundColor Red
    Read-Host "Press Enter to exit"; exit 1
}
Write-Host "[OK] esptool ready" -ForegroundColor Green

# --- Check required binaries ---
foreach ($entry in $Bins.GetEnumerator()) {
    if (-not (Test-Path $entry.Value)) {
        Write-Host "ERROR: Missing file $($entry.Value)" -ForegroundColor Red
        Read-Host "Press Enter to exit"; exit 1
    }
}

# --- Auto-detect / Ask COM port ---
if (-not $Port) {
    $ports = [System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object
    if ($ports.Count -eq 1) {
        $Port = $ports[0]
        Write-Host "[AUTO] Detected COM port: $Port" -ForegroundColor Green
    } elseif ($ports.Count -gt 1) {
        Write-Host "Multiple COM ports found:" -ForegroundColor Yellow
        $ports | ForEach-Object { Write-Host "  - $_" }
        $Port = Read-Host "Enter COM port (e.g. COM5)"
    } else {
        Write-Host "No COM port found. Please plug in the ESP32 via USB and press Enter."
        Read-Host "..."
        $ports = [System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object
        $Port  = $ports | Select-Object -First 1
        if (-not $Port) {
            Write-Host "ERROR: Still no COM port detected." -ForegroundColor Red
            Read-Host "Press Enter to exit"; exit 1
        }
    }
}

if ($Port -match '^\d+$') {
    $Port = "COM$Port"
}

Write-Host ""
Write-Host "Flashing to $Port ..." -ForegroundColor Cyan
Write-Host ""

# --- Flash ---
$flashArgs = @(
    "--chip", "esp32",
    "--port", $Port,
    "--baud", "230400",
    "write_flash",
    "--erase-all",
    "--flash-mode", "dio",
    "--flash-freq", "40m",
    "--flash-size", "4MB",
    "0x1000",  $Bins["0x1000"],
    "0x10000", $Bins["0x10000"],
    "0x17000", $Bins["0x17000"],
    "0x20000", $Bins["0x20000"]
)

& $esptoolExe @flashArgs
if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "ERROR: Flash failed! Please check:" -ForegroundColor Red
    Write-Host "  - Is the USB cable connected properly?" -ForegroundColor Yellow
    Write-Host "  - Is the COM port correct? ($Port)" -ForegroundColor Yellow
    Write-Host "  - Try unplugging and replugging the USB, then run FLASH.bat again." -ForegroundColor Yellow
    Read-Host "Press Enter to exit"; exit 1
}

Write-Host ""
Write-Host "[OK] Flash Successful!" -ForegroundColor Green
Write-Host ""

# --- Read MAC and Calculate AP Password ---
Write-Host "Reading board MAC address..." -ForegroundColor Cyan
Start-Sleep -Milliseconds 1500

$macOutput = & $esptoolExe --chip esp32 --baud 230400 --port $Port read_mac 2>&1
$macLine   = $macOutput | Select-String -Pattern "MAC:\s*([0-9a-fA-F:]{17})" | Select-Object -First 1

if (-not $macLine) {
    Write-Host "WARNING: Could not read MAC address." -ForegroundColor Yellow
    Write-Host "Run SHOW_PASSWORD.bat later to view your Wi-Fi password." -ForegroundColor Gray
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
    Write-Host ""
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
