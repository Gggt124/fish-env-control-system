<#
.SYNOPSIS
    à¸­à¹ˆà¸²à¸™ MAC address à¸ˆà¸²à¸ ESP32 à¹€à¸žà¸·à¹ˆà¸­à¸„à¸³à¸™à¸§à¸“à¹à¸¥à¸°à¹à¸ªà¸”à¸‡ AP Password
.PARAMETER Port
    COM port à¸–à¹‰à¸²à¹„à¸¡à¹ˆà¸£à¸°à¸šà¸¸à¸ˆà¸° auto-detect à¸«à¸£à¸·à¸­à¸–à¸²à¸¡
#>
param(
    [string]$Port = ""
)

$ErrorActionPreference = "Stop"
chcp 65001 > $null

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  à¸à¸¹à¹‰à¸„à¸·à¸™ Wi-Fi Password (Fish Pump)         " -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# --- à¸•à¸£à¸§à¸ˆà¸ªà¸­à¸š Python & esptool ---
try {
    $null = python --version 2>&1
} catch {
    Write-Host "ERROR: Python à¹„à¸¡à¹ˆà¸žà¸š à¸à¸£à¸¸à¸“à¸²à¸•à¸´à¸”à¸•à¸±à¹‰à¸‡à¸ˆà¸²à¸ https://python.org" -ForegroundColor Red
    Read-Host "à¸à¸” Enter à¹€à¸žà¸·à¹ˆà¸­à¸›à¸´à¸”"; exit 1
}

$esptoolVer = python -m esptool version 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "à¸•à¸´à¸”à¸•à¸±à¹‰à¸‡ esptool..." -ForegroundColor Yellow
    python -m pip install esptool --quiet
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
Write-Host "à¸à¸³à¸¥à¸±à¸‡à¸­à¹ˆà¸²à¸™ MAC address à¸œà¹ˆà¸²à¸™ $Port ..." -ForegroundColor Cyan

# --- à¸­à¹ˆà¸²à¸™ MAC ---
$macOutput = python -m esptool --chip esp32 --baud 230400 --port $Port read_mac 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: à¸­à¹ˆà¸²à¸™ MAC à¹„à¸¡à¹ˆà¹„à¸”à¹‰ à¹‚à¸›à¸£à¸”à¸•à¸£à¸§à¸ˆà¸ªà¸­à¸šà¸à¸²à¸£à¹€à¸Šà¸·à¹ˆà¸­à¸¡à¸•à¹ˆà¸­" -ForegroundColor Red
    Read-Host "à¸à¸” Enter à¹€à¸žà¸·à¹ˆà¸­à¸›à¸´à¸”"; exit 1
}

$macLine = $macOutput | Select-String -Pattern "MAC:\s*([0-9a-fA-F:]{17})" | Select-Object -First 1
if (-not $macLine) {
    Write-Host "ERROR: à¹„à¸¡à¹ˆà¸žà¸š MAC address à¹ƒà¸™ output" -ForegroundColor Red
    Read-Host "à¸à¸” Enter à¹€à¸žà¸·à¹ˆà¸­à¸›à¸´à¸”"; exit 1
}

# --- à¸„à¸³à¸™à¸§à¸“ Password ---
$macStr   = $macLine.Matches.Groups[1].Value.Trim()
$macBytes = $macStr -split ":" | ForEach-Object { [Convert]::ToInt32($_.Trim(), 16) }
$salt     = 0x5A
$password = "{0:X2}{1:X2}{2:X2}{3:X2}" -f `
    ($macBytes[2] -bxor $salt), ($macBytes[3] -bxor $salt),
    ($macBytes[4] -bxor $salt), ($macBytes[5] -bxor $salt)

Write-Host ""
Write-Host "============================================" -ForegroundColor Green
Write-Host "  à¸‚à¹‰à¸­à¸¡à¸¹à¸¥ Wi-Fi                             " -ForegroundColor Green
Write-Host "============================================" -ForegroundColor Green
Write-Host ""
Write-Host "  MAC Address   : $macStr                  " -ForegroundColor Gray
Write-Host "  Wi-Fi SSID    : FishPump-Setup           " -ForegroundColor White
Write-Host "  Wi-Fi Password: $password                " -ForegroundColor Yellow
Write-Host ""
Write-Host "============================================" -ForegroundColor Green
Write-Host ""
Read-Host "à¸à¸” Enter à¹€à¸žà¸·à¹ˆà¸­à¸›à¸´à¸”"
