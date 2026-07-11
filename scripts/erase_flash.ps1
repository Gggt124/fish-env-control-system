<#
.SYNOPSIS
    Erase flash memory of an ESP32 or ESP32-S3 board.
#>
param(
    [string]$Port  = "",
    [ValidateSet("", "esp32", "esp32s3")]
    [string]$Board = ""
)

$ErrorActionPreference = "Continue"
chcp 65001 > $null

$ScriptDir = $PSScriptRoot

$BoardProfiles = @{
    esp32   = @{ Chip="esp32";   Baud="230400"; DisplayName="ESP32 Classic DevKit V1 (4 MB)" }
    esp32s3 = @{ Chip="esp32s3"; Baud="460800"; DisplayName="ESP32-S3 DevKitC-1 WROOM-1-N16R8 (16 MB)" }
}

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  Erase Flash Memory (Fish Pump)            " -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

$esptoolExe = Join-Path $ScriptDir "esptool.exe"
if (-not (Test-Path $esptoolExe)) {
    Write-Host "ERROR: esptool.exe not found in tools/ folder!" -ForegroundColor Red
    Read-Host "Press Enter to exit"; exit 1
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
Write-Host "[OK] Board: $($Profile.DisplayName)" -ForegroundColor Green

if ($Board -eq "esp32s3") {
    Write-Host ""
    Write-Host "*** ATTENTION: ESP32-S3 ***" -ForegroundColor Yellow
    Write-Host "Plug the USB cable into the port labeled 'COM' (or 'UART') on the board." -ForegroundColor Cyan
    Write-Host "Do NOT use the port labeled 'USB'." -ForegroundColor Red
    Write-Host "***************************" -ForegroundColor Yellow
    Write-Host ""
}

# Auto-detect / ask COM port
if (-not $Port) {
    while ($true) {
        $ports = [System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object
        
        Write-Host ""
        if ($ports.Count -eq 0) {
            Write-Host "No COM port found. Plug in the board and press Enter to scan again..." -ForegroundColor Yellow
            Read-Host "..."
            continue
        }

        Write-Host "Available COM ports:" -ForegroundColor Cyan
        $ports | ForEach-Object { Write-Host "  - $_" }
        Write-Host ""
        Write-Host "Type the port number (e.g. 5), or press Enter to AUTO-SELECT if only 1 exists."
        $choice = Read-Host "Type 'R' to Rescan"

        if ([string]::IsNullOrWhiteSpace($choice)) {
            if ($ports.Count -eq 1) {
                $Port = $ports[0]
                Write-Host "[AUTO] Selected: $Port" -ForegroundColor Green
                break
            } else {
                Write-Host "Multiple ports available. Please specify one." -ForegroundColor Red
                continue
            }
        }

        if ($choice.Trim() -match '^[rR]$') {
            continue
        }

        if ($choice.Trim() -match '^\d+$') {
            $Port = "COM" + $choice.Trim()
            break
        }
        
        $Port = $choice.Trim()
        break
    }
}
if ($Port -match '^\d+$') { $Port = "COM$Port" }

Write-Host ""
Write-Host "Erasing flash on $Port ..." -ForegroundColor Cyan
Write-Host ""

& $esptoolExe --chip $Profile.Chip --port $Port --baud $Profile.Baud erase_flash

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "ERROR: Erase failed!" -ForegroundColor Red
    Write-Host "  - Check USB cable and COM port ($Port)" -ForegroundColor Yellow
    Write-Host "  - Make sure the cable is in the 'COM' (or 'UART') port, not the 'USB' port" -ForegroundColor Yellow
    Read-Host "Press Enter to exit"; exit 1
}

Write-Host ""
Write-Host "[OK] Flash Erased Successfully!" -ForegroundColor Green
Write-Host "The board is now completely empty. You will need to flash firmware again." -ForegroundColor Yellow
Write-Host ""

Read-Host "Press Enter to exit"
