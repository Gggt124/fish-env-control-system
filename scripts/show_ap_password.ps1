<#
.SYNOPSIS
    อ่าน MAC address จากบอร์ด ESP32 แล้วแสดง SoftAP password ที่ firmware จะใช้

.DESCRIPTION
    รัน esptool.py read_mac เพื่ออ่าน MAC จากบอร์ดโดยตรง
    แล้วคำนวณ password ด้วยสูตรเดียวกับ firmware:
      bytes 2-5 (0-indexed) → uppercase hex 8 ตัว ไม่มี dash
    ตัวอย่าง: MAC aa:bb:cc:dd:ee:ff → password = CCDDEEFF

    Password ไม่ปรากฏใน serial log หรือ HTTP API ใด
    ใช้ script นี้เฉพาะตอนประกอบเครื่องเพื่อเขียนติดป้ายเท่านั้น

.PARAMETER Port
    Serial port ของบอร์ด เช่น COM5

.EXAMPLE
    .\scripts\show_ap_password.ps1 -Port COM5
#>
param(
    [Parameter(Mandatory=$true)]
    [string]$Port,
    [ValidateSet("esp32", "esp32s3")]
    [string]$Target = "esp32"
)

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "=== SoftAP Password (Board-Specific) ===" -ForegroundColor Cyan
Write-Host "Reading MAC address from board on $Port ..." -ForegroundColor Yellow

$oldErrPref = $ErrorActionPreference
$ErrorActionPreference = "Continue"
try {
    $output = python -m esptool --chip $Target --port $Port read_mac 2>&1
} catch {
    Write-Host "ERROR: Failed to run esptool. Make sure Python and esptool are installed." -ForegroundColor Red
    Write-Host "       pip install esptool" -ForegroundColor Gray
    exit 1
} finally {
    $ErrorActionPreference = $oldErrPref
}

# Parse line like: "MAC: aa:bb:cc:dd:ee:ff"
$macLine = $output | Select-String -Pattern "MAC:\s*([0-9a-fA-F:]{17})" | Select-Object -First 1
if (-not $macLine) {
    Write-Host "ERROR: Could not find MAC address in esptool output:" -ForegroundColor Red
    Write-Host ($output -join "`n")
    exit 1
}

$macStr = $macLine.Matches.Groups[1].Value.Trim()

$macBytes = $macStr -split ":" | ForEach-Object { [Convert]::ToInt32($_.Trim(), 16) }
if ($macBytes.Count -ne 6) {
    Write-Host "ERROR: Unexpected MAC format: $macStr" -ForegroundColor Red
    exit 1
}

# Same formula as firmware: (mac[2..5] ^ 0x5A), uppercase hex, no dash
$salt = 0x5A
$password = "{0:X2}{1:X2}{2:X2}{3:X2}" -f ($macBytes[2] -bxor $salt), 
                                          ($macBytes[3] -bxor $salt), 
                                          ($macBytes[4] -bxor $salt), 
                                          ($macBytes[5] -bxor $salt)

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  SoftAP SSID    : FishPump-Setup      " -ForegroundColor White
Write-Host "  SoftAP Password: $password            " -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Write the password above on a label and attach it to the device." -ForegroundColor White
Write-Host ""
