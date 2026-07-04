<#
.SYNOPSIS
    Flash firmware ?? ESP32 ???????? AP password
.PARAMETER Port
    COM port ???? COM5 ???????????? auto-detect ???????
#>
param(
    [string]$Port = ""
)

$ErrorActionPreference = "Continue"
chcp 65001 > $null

# Script ?????? flash-package/tools/ ? firmware ?????? flash-package/firmware/
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
Write-Host "  Fish Pump Relay Timer � Flash Installer  " -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# --- ??????? esptool.exe ---
$esptoolExe = Join-Path $ScriptDir "esptool.exe"
if (-not (Test-Path $esptoolExe)) {
    Write-Host "ERROR: ????? esptool.exe ?????????? tools/" -ForegroundColor Red
    Read-Host "?? Enter ????????"; exit 1
}
Write-Host "[OK] esptool ready" -ForegroundColor Green

# --- ??????? binary ??? ---
foreach ($entry in $Bins.GetEnumerator()) {
    if (-not (Test-Path $entry.Value)) {
        Write-Host "ERROR: ????? $($entry.Value)" -ForegroundColor Red
        Read-Host "?? Enter ????????"; exit 1
    }
}

# --- Auto-detect / ??? COM port ---
if (-not $Port) {
    $ports = [System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object
    if ($ports.Count -eq 1) {
        $Port = $ports[0]
        Write-Host "[AUTO] ?????? COM port: $Port" -ForegroundColor Green
    } elseif ($ports.Count -gt 1) {
        Write-Host "?????? COM port:" -ForegroundColor Yellow
        $ports | ForEach-Object { Write-Host "  - $_" }
        $Port = Read-Host "???? COM port (???? COM5)"
    } else {
        Write-Host "????? COM port ?????????? ESP32 ?????? Enter"
        Read-Host "..."
        $ports = [System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object
        $Port  = $ports | Select-Object -First 1
        if (-not $Port) {
            Write-Host "ERROR: ???????? COM port" -ForegroundColor Red
            Read-Host "?? Enter ????????"; exit 1
        }
    }
}

Write-Host ""
Write-Host "Flash ????? $Port ..." -ForegroundColor Cyan
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

& $esptoolExe @flashArgs
if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "ERROR: Flash ????????? ???????:" -ForegroundColor Red
    Write-Host "  - ????? USB ????????" -ForegroundColor Yellow
    Write-Host "  - COM port ???????? ($Port)" -ForegroundColor Yellow
    Write-Host "  - ??????-???????? USB ??????? FLASH.bat ????" -ForegroundColor Yellow
    Read-Host "?? Enter ????????"; exit 1
}

Write-Host ""
Write-Host "[OK] Flash ??????!" -ForegroundColor Green
Write-Host ""

# --- ???? MAC ???????? AP Password ---
Write-Host "????????? MAC address ????????..." -ForegroundColor Cyan
Start-Sleep -Milliseconds 1500

$macOutput = & $esptoolExe --chip esp32 --baud 230400 --port $Port read_mac 2>&1
$macLine   = $macOutput | Select-String -Pattern "MAC:\s*([0-9a-fA-F:]{17})" | Select-Object -First 1

if (-not $macLine) {
    Write-Host "WARNING: ???? MAC ??????" -ForegroundColor Yellow
    Write-Host "??? SHOW_PASSWORD.bat ????????????? password" -ForegroundColor Gray
} else {
    $macStr   = $macLine.Matches.Groups[1].Value.Trim()
    $macBytes = $macStr -split ":" | ForEach-Object { [Convert]::ToInt32($_.Trim(), 16) }
    $salt     = 0x5A
    $password = "{0:X2}{1:X2}{2:X2}{3:X2}" -f `
        ($macBytes[2] -bxor $salt), ($macBytes[3] -bxor $salt),
        ($macBytes[4] -bxor $salt), ($macBytes[5] -bxor $salt)

    Write-Host ""
    Write-Host "============================================" -ForegroundColor Green
    Write-Host "  Flash ?????????!                         " -ForegroundColor Green
    Write-Host "============================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "  Wi-Fi SSID    : FishPump-Setup           " -ForegroundColor White
    Write-Host "  Wi-Fi Password: $password                " -ForegroundColor Yellow
    Write-Host "  Web Dashboard : http://192.168.4.1       " -ForegroundColor White
    Write-Host "  Login         : admin / admin123         " -ForegroundColor White
    Write-Host ""
    Write-Host "  >>> ?????? Password ???????! <<<        " -ForegroundColor Red
    Write-Host "============================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Password ???????????????????????" -ForegroundColor Gray
    Write-Host "??????: ??? SHOW_PASSWORD.bat (????????? USB)" -ForegroundColor Gray
}

Write-Host ""
Read-Host "?? Enter ????????"
