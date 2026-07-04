<#
.SYNOPSIS
    ???? MAC address ??? ESP32 ????????????????? AP Password
.PARAMETER Port
    COM port ???????????? auto-detect ???????
#>
param(
    [string]$Port = ""
)

$ErrorActionPreference = "Stop"
chcp 65001 > $null

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  ?????? Wi-Fi Password (Fish Pump)         " -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# --- ??????? Python & esptool ---
try {
    $null = python --version 2>&1
} catch {
    Write-Host "ERROR: Python ????? ??????????????? https://python.org" -ForegroundColor Red
    Read-Host "?? Enter ????????"; exit 1
}

$esptoolVer = python -m esptool version 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "??????? esptool..." -ForegroundColor Yellow
    python -m pip install esptool --quiet
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
Write-Host "????????? MAC address ???? $Port ..." -ForegroundColor Cyan

# --- ???? MAC ---
$macOutput = python -m esptool --chip esp32 --baud 230400 --port $Port read_mac 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: ???? MAC ?????? ???????????????????????" -ForegroundColor Red
    Read-Host "?? Enter ????????"; exit 1
}

$macLine = $macOutput | Select-String -Pattern "MAC:\s*([0-9a-fA-F:]{17})" | Select-Object -First 1
if (-not $macLine) {
    Write-Host "ERROR: ????? MAC address ?? output" -ForegroundColor Red
    Read-Host "?? Enter ????????"; exit 1
}

# --- ????? Password ---
$macStr   = $macLine.Matches.Groups[1].Value.Trim()
$macBytes = $macStr -split ":" | ForEach-Object { [Convert]::ToInt32($_.Trim(), 16) }
$salt     = 0x5A
$password = "{0:X2}{1:X2}{2:X2}{3:X2}" -f `
    ($macBytes[2] -bxor $salt), ($macBytes[3] -bxor $salt),
    ($macBytes[4] -bxor $salt), ($macBytes[5] -bxor $salt)

Write-Host ""
Write-Host "============================================" -ForegroundColor Green
Write-Host "  ?????? Wi-Fi                             " -ForegroundColor Green
Write-Host "============================================" -ForegroundColor Green
Write-Host ""
Write-Host "  MAC Address   : $macStr                  " -ForegroundColor Gray
Write-Host "  Wi-Fi SSID    : FishPump-Setup           " -ForegroundColor White
Write-Host "  Wi-Fi Password: $password                " -ForegroundColor Yellow
Write-Host ""
Write-Host "============================================" -ForegroundColor Green
Write-Host ""
Read-Host "?? Enter ????????"
