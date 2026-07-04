<#
.SYNOPSIS
    Read MAC address from ESP32 to calculate and display AP Password
.PARAMETER Port
    COM port e.g. COM5. If not specified, it will auto-detect or ask.
#>
param(
    [string]$Port = ""
)

$ErrorActionPreference = "Stop"
chcp 65001 > $null

$ScriptDir = $PSScriptRoot

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  Recover Wi-Fi Password (Fish Pump)        " -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# --- Check esptool.exe ---
$esptoolExe = Join-Path $ScriptDir "esptool.exe"
if (-not (Test-Path $esptoolExe)) {
    Write-Host "ERROR: esptool.exe not found in tools/ folder!" -ForegroundColor Red
    Read-Host "Press Enter to exit"; exit 1
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
Write-Host "Reading MAC address via $Port ..." -ForegroundColor Cyan

# --- Read MAC ---
$macOutput = & $esptoolExe --chip esp32 --baud 230400 --port $Port read_mac 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Could not read MAC. Please check connection." -ForegroundColor Red
    Read-Host "Press Enter to exit"; exit 1
}

$macLine = $macOutput | Select-String -Pattern "MAC:\s*([0-9a-fA-F:]{17})" | Select-Object -First 1
if (-not $macLine) {
    Write-Host "ERROR: MAC address not found in output." -ForegroundColor Red
    Read-Host "Press Enter to exit"; exit 1
}

# --- Calculate Password ---
$macStr   = $macLine.Matches.Groups[1].Value.Trim()
$macBytes = $macStr -split ":" | ForEach-Object { [Convert]::ToInt32($_.Trim(), 16) }
$salt     = 0x5A

$password = "{0:X2}{1:X2}{2:X2}{3:X2}" -f `
    ($macBytes[2] -bxor $salt), ($macBytes[3] -bxor $salt),
    ($macBytes[4] -bxor $salt), ($macBytes[5] -bxor $salt)

Write-Host ""
Write-Host "============================================" -ForegroundColor Green
Write-Host "  Wi-Fi Info                                " -ForegroundColor Green
Write-Host "============================================" -ForegroundColor Green
Write-Host ""
Write-Host "  Wi-Fi SSID    : FishPump-Setup           " -ForegroundColor White
Write-Host "  Wi-Fi Password: $password                " -ForegroundColor Yellow
Write-Host "  Web Dashboard : http://192.168.4.1       " -ForegroundColor White
Write-Host "  Login         : admin / admin123         " -ForegroundColor White
Write-Host ""
Write-Host "============================================" -ForegroundColor Green
Write-Host ""

Read-Host "Press Enter to exit"
