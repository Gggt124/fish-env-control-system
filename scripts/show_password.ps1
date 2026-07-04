<#
.SYNOPSIS
    อ่าน MAC address จาก ESP32 เพื่อคำนวณและแสดง AP Password
.PARAMETER Port
    COM port ถ้าไม่ระบุจะ auto-detect หรือถาม
#>
param(
    [string]$Port = ""
)

$ErrorActionPreference = "Stop"
chcp 65001 > $null

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  กู้คืน Wi-Fi Password (Fish Pump)         " -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# --- ตรวจสอบ Python & esptool ---
try {
    $null = python --version 2>&1
} catch {
    Write-Host "ERROR: ไม่พบ Python กรุณารัน FLASH.bat เพื่อติดตั้งเครื่องมือก่อน" -ForegroundColor Red
    Read-Host "กด Enter เพื่อปิด"; exit 1
}

$esptoolVer = python -m esptool version 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: ไม่พบ esptool กรุณารัน FLASH.bat เพื่อติดตั้งเครื่องมือก่อน" -ForegroundColor Red
    Read-Host "กด Enter เพื่อปิด"; exit 1
}

# --- Auto-detect / ถาม COM port ---
if (-not $Port) {
    $ports = [System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object
    if ($ports.Count -eq 1) {
        $Port = $ports[0]
        Write-Host "[AUTO] ตรวจพบ COM port: $Port" -ForegroundColor Green
    } elseif ($ports.Count -gt 1) {
        Write-Host "พบหลาย COM port:" -ForegroundColor Yellow
        $ports | ForEach-Object { Write-Host "  - $_" }
        $Port = Read-Host "กรอก COM port (เช่น COM5)"
    } else {
        Write-Host "ไม่พบ COM port กรุณาเสียบ ESP32 แล้วกด Enter"
        Read-Host "..."
        $ports = [System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object
        $Port  = $ports | Select-Object -First 1
        if (-not $Port) {
            Write-Host "ERROR: ยังไม่พบ COM port" -ForegroundColor Red
            Read-Host "กด Enter เพื่อปิด"; exit 1
        }
    }
}

Write-Host ""
Write-Host "กำลังอ่าน MAC address ผ่าน $Port ..." -ForegroundColor Cyan

# --- อ่าน MAC ---
$macOutput = python -m esptool --chip esp32 --baud 230400 --port $Port read_mac 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: อ่าน MAC ไม่ได้ โปรดตรวจสอบการเชื่อมต่อ" -ForegroundColor Red
    Read-Host "กด Enter เพื่อปิด"; exit 1
}

$macLine = $macOutput | Select-String -Pattern "MAC:\s*([0-9a-fA-F:]{17})" | Select-Object -First 1
if (-not $macLine) {
    Write-Host "ERROR: ไม่พบ MAC address ใน output" -ForegroundColor Red
    Read-Host "กด Enter เพื่อปิด"; exit 1
}

# --- คำนวณ Password ---
$macStr   = $macLine.Matches.Groups[1].Value.Trim()
$macBytes = $macStr -split ":" | ForEach-Object { [Convert]::ToInt32($_.Trim(), 16) }
$salt     = 0x5A
$password = "{0:X2}{1:X2}{2:X2}{3:X2}" -f `
    ($macBytes[2] -bxor $salt), ($macBytes[3] -bxor $salt),
    ($macBytes[4] -bxor $salt), ($macBytes[5] -bxor $salt)

Write-Host ""
Write-Host "============================================" -ForegroundColor Green
Write-Host "  ข้อมูล Wi-Fi                             " -ForegroundColor Green
Write-Host "============================================" -ForegroundColor Green
Write-Host ""
Write-Host "  MAC Address   : $macStr                  " -ForegroundColor Gray
Write-Host "  Wi-Fi SSID    : FishPump-Setup           " -ForegroundColor White
Write-Host "  Wi-Fi Password: $password                " -ForegroundColor Yellow
Write-Host ""
Write-Host "============================================" -ForegroundColor Green
Write-Host ""
Read-Host "กด Enter เพื่อปิด"
