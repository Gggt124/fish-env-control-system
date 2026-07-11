<#
.SYNOPSIS
    Read MAC address from a Fish Pump board and display the AP Wi-Fi password.
.PARAMETER Port
    COM port e.g. 5. If not specified, auto-detects or prompts.
.PARAMETER Board
    Board profile: 'esp32' or 'esp32s3'. If not specified, shows a selection menu.
#>
param(
    [string]$Port  = "",
    [ValidateSet("", "esp32", "esp32s3")]
    [string]$Board = ""
)

$ErrorActionPreference = "Stop"
chcp 65001 > $null

$ScriptDir = $PSScriptRoot

$BoardProfiles = @{
    esp32   = @{ Chip="esp32";   Baud="230400"; DisplayName="ESP32 Classic DevKit V1 (4 MB)" }
    esp32s3 = @{ Chip="esp32s3"; Baud="460800"; DisplayName="ESP32-S3 DevKitC-1 WROOM-1-N16R8 (16 MB)" }
}

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

$Profile = $BoardProfiles[$Board]

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  Recover Wi-Fi Password (Fish Pump)        " -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

if ($Board -eq "esp32s3") {
    Write-Host "*** ATTENTION: ESP32-S3 (Native USB) ***" -ForegroundColor Yellow
    Write-Host "1. Plug the USB cable into the port labeled 'USB' (NOT the 'UART' port)!" -ForegroundColor Red
    Write-Host "2. To prevent USB disconnection errors, you should put the board into" -ForegroundColor Yellow
    Write-Host "   DOWNLOAD MODE right now before selecting the COM port:" -ForegroundColor Yellow
    Write-Host "   - Hold BOOT button" -ForegroundColor White
    Write-Host "   - Press RST (or EN) button" -ForegroundColor White
    Write-Host "   - Release BOOT button" -ForegroundColor White
    Write-Host "****************************************" -ForegroundColor Yellow
    Write-Host ""
}

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
        $Port = Read-Host "Enter COM port number (e.g. 5)"
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
$oldErrPref = $ErrorActionPreference
$ErrorActionPreference = "Continue"
$macOutput = & $esptoolExe --chip $Profile.Chip --baud $Profile.Baud --port $Port read_mac 2>&1
$ErrorActionPreference = $oldErrPref

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Could not read MAC. Please check connection." -ForegroundColor Red
    Write-Host ($macOutput -join "`n") -ForegroundColor DarkGray
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
