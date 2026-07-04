<#
.SYNOPSIS
    Flash firmware ลง ESP32 แล้วแสดง AP password
.PARAMETER Port
    COM port เช่น COM5 ถ้าไม่ระบุจะ auto-detect หรือถาม
#>
param(
    [string]$Port = ""
)

$ErrorActionPreference = "Stop"
chcp 65001 > $null

# Script อยู่ใน flash-package/tools/ → firmware อยู่ใน flash-package/firmware/
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
Write-Host "  Fish Pump Relay Timer — Flash Installer  " -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# --- ตรวจสอบ Python ---
try {
    $pyVer = python --version 2>&1
    Write-Host "[OK] Python: $pyVer" -ForegroundColor Green
} catch {
    Write-Host "ERROR: Python ไม่พบ กรุณาติดตั้งจาก https://python.org" -ForegroundColor Red
    Read-Host "กด Enter เพื่อปิด"
    exit 1
}

# --- ติดตั้ง esptool ถ้ายังไม่มี ---
$esptoolVer = python -m esptool version 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "[...] ติดตั้ง esptool..." -ForegroundColor Yellow
    python -m pip install esptool --quiet
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: ติดตั้ง esptool ไม่สำเร็จ" -ForegroundColor Red
        Read-Host "กด Enter เพื่อปิด"; exit 1
    }
    Write-Host "[OK] esptool installed" -ForegroundColor Green
} else {
    Write-Host "[OK] esptool ready" -ForegroundColor Green
}

# --- ตรวจสอบ binary ครบ ---
foreach ($entry in $Bins.GetEnumerator()) {
    if (-not (Test-Path $entry.Value)) {
        Write-Host "ERROR: ไม่พบ $($entry.Value)" -ForegroundColor Red
        Read-Host "กด Enter เพื่อปิด"; exit 1
    }
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
Write-Host "Flash ไปที่ $Port ..." -ForegroundColor Cyan
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
    Write-Host "ERROR: Flash ไม่สำเร็จ ตรวจสอบ:" -ForegroundColor Red
    Write-Host "  - เสียบ USB ถูกต้อง?" -ForegroundColor Yellow
    Write-Host "  - COM port ถูกต้อง? ($Port)" -ForegroundColor Yellow
    Write-Host "  - ลองถอด-เสียบสาย USB แล้วรัน FLASH.bat ใหม่" -ForegroundColor Yellow
    Read-Host "กด Enter เพื่อปิด"; exit 1
}

Write-Host ""
Write-Host "[OK] Flash สำเร็จ!" -ForegroundColor Green
Write-Host ""

# --- อ่าน MAC และคำนวณ AP Password ---
Write-Host "กำลังอ่าน MAC address ของบอร์ด..." -ForegroundColor Cyan
Start-Sleep -Milliseconds 1500

$macOutput = python -m esptool --chip esp32 --baud 230400 --port $Port read_mac 2>&1
$macLine   = $macOutput | Select-String -Pattern "MAC:\s*([0-9a-fA-F:]{17})" | Select-Object -First 1

if (-not $macLine) {
    Write-Host "WARNING: อ่าน MAC ไม่ได้" -ForegroundColor Yellow
    Write-Host "รัน SHOW_PASSWORD.bat ทีหลังเพื่อดู password" -ForegroundColor Gray
} else {
    $macStr   = $macLine.Matches.Groups[1].Value.Trim()
    $macBytes = $macStr -split ":" | ForEach-Object { [Convert]::ToInt32($_.Trim(), 16) }
    $salt     = 0x5A
    $password = "{0:X2}{1:X2}{2:X2}{3:X2}" -f `
        ($macBytes[2] -bxor $salt), ($macBytes[3] -bxor $salt),
        ($macBytes[4] -bxor $salt), ($macBytes[5] -bxor $salt)

    Write-Host ""
    Write-Host "============================================" -ForegroundColor Green
    Write-Host "  Flash เสร็จแล้ว!                         " -ForegroundColor Green
    Write-Host "============================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "  Wi-Fi SSID    : FishPump-Setup           " -ForegroundColor White
    Write-Host "  Wi-Fi Password: $password                " -ForegroundColor Yellow
    Write-Host "  Web Dashboard : http://192.168.4.1       " -ForegroundColor White
    Write-Host "  Login         : admin / admin123         " -ForegroundColor White
    Write-Host ""
    Write-Host "  >>> บันทึก Password ไว้ด้วย! <<<        " -ForegroundColor Red
    Write-Host "============================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Password ขึ้นกับบอร์ดนี้เท่านั้น" -ForegroundColor Gray
    Write-Host "ถ้าลืม: รัน SHOW_PASSWORD.bat (ต้องเสียบ USB)" -ForegroundColor Gray
}

Write-Host ""
Read-Host "กด Enter เพื่อปิด"
