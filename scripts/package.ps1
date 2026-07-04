<#
.SYNOPSIS
    เก็บไฟล์ .bin จาก build/ และสคริปต์เสริม เพื่อเตรียมโฟลเดอร์ flash-package/ สำหรับแจกลูกค้า
.PARAMETER OutputDir
    Path ปลายทางของ flash-package/ (default: .\flash-package)
.PARAMETER Zip
    เปิดเพื่อ pass flag นี้เพื่อสั่ง zip flash-package/ เป็น flash-package.zip ด้วย
.EXAMPLE
    .\scripts\package.ps1
    .\scripts\package.ps1 -Zip
#>
param(
    [string]$OutputDir = ".\flash-package",
    [switch]$Zip
)

$ErrorActionPreference = "Stop"
chcp 65001 > $null

$ProjectRoot = Split-Path $PSScriptRoot -Parent
$BuildDir    = Join-Path $ProjectRoot "build"
$ScriptsDir  = $PSScriptRoot

# --- ตรวจสอบไฟล์ binary ที่จำเป็นต้องใช้ ---
$RequiredBinaries = [ordered]@{
    "bootloader/bootloader.bin"                 = "firmware\bootloader.bin"
    "partition_table/partition-table.bin"       = "firmware\partition-table.bin"
    "ota_data_initial.bin"                      = "firmware\ota_data_initial.bin"
    "fish_pump_relay_timer_control.bin"         = "firmware\fish_pump_relay_timer_control.bin"
}

foreach ($src in $RequiredBinaries.Keys) {
    $fullSrc = Join-Path $BuildDir $src
    if (-not (Test-Path $fullSrc)) {
        Write-Error "Missing binary: $fullSrc`nRun '.\scripts\build.ps1' first."
    }
}

Write-Host "[package] Checking standalone esptool.exe..." -ForegroundColor Cyan
$esptoolExe = Join-Path $ScriptsDir "esptool.exe"
if (-not (Test-Path $esptoolExe)) {
    Write-Host "  esptool.exe not found. Downloading v5.3.1 from GitHub..." -ForegroundColor Yellow
    $url = "https://github.com/espressif/esptool/releases/download/v5.3.1/esptool-v5.3.1-windows-amd64.zip"
    $zipPath = Join-Path $ScriptsDir "esptool.zip"
    $tmpDir = Join-Path $ScriptsDir "esptool_tmp"
    
    # Use SilentlyContinue to speed up download in PS 5.1
    $ProgressPreference = 'SilentlyContinue'
    Invoke-WebRequest -Uri $url -OutFile $zipPath
    Expand-Archive -Path $zipPath -DestinationPath $tmpDir -Force
    Copy-Item (Join-Path $tmpDir "esptool-windows-amd64\esptool.exe") -Destination $esptoolExe -Force
    Remove-Item $tmpDir -Recurse -Force
    Remove-Item $zipPath -Force
    Write-Host "  Downloaded esptool.exe" -ForegroundColor Green
}

# --- ??????????????? output directory ---
if (Test-Path $OutputDir) {
    Remove-Item $OutputDir -Recurse -Force
}
$FirmwareDir = Join-Path $OutputDir "firmware"
$ToolsDir    = Join-Path $OutputDir "tools"

foreach ($dir in @($OutputDir, $FirmwareDir, $ToolsDir)) {
    if (-not (Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir | Out-Null
    }
}

# --- Copy binary ---
Write-Host "[package] Copying firmware binaries..." -ForegroundColor Cyan
foreach ($entry in $RequiredBinaries.GetEnumerator()) {
    $src  = Join-Path $BuildDir $entry.Key
    $dest = Join-Path $OutputDir $entry.Value
    Copy-Item -LiteralPath $src -Destination $dest -Force
    $size = (Get-Item $dest).Length
    Write-Host "  OK  $($entry.Value)  ($size bytes)" -ForegroundColor Green
}

# --- Copy tools (ps1 scripts) ---
Write-Host "[package] Copying tools..." -ForegroundColor Cyan
foreach ($tool in @("flash_and_show.ps1", "show_password.ps1", "esptool.exe")) {
    $src  = Join-Path $ScriptsDir $tool
    $dest = Join-Path $ToolsDir $tool
    if (Test-Path $src) {
        Copy-Item -LiteralPath $src -Destination $dest -Force
        Write-Host "  OK  tools\$tool" -ForegroundColor Green
    } else {
        Write-Warning "  MISSING  scripts\$tool - skipping"
    }
}

# --- Copy batch launchers & README ---
Write-Host "[package] Copying batch launchers & README..." -ForegroundColor Cyan
$extraFiles = @("FLASH.bat", "SHOW_PASSWORD.bat", "README_customer.md")
$extraFiles += (Get-ChildItem -Path $ScriptsDir -Filter "README_*.md" -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Name | Where-Object { $_ -ne "README_customer.md" })

foreach ($file in $extraFiles) {
    if ([string]::IsNullOrWhiteSpace($file)) { continue }
    $src  = Join-Path $ScriptsDir $file
    $dest = Join-Path $OutputDir $file
    if (Test-Path $src) {
        Copy-Item -LiteralPath $src -Destination $dest -Force
        Write-Host "  OK  $file" -ForegroundColor Green
    } else {
        Write-Warning "  MISSING  scripts\$file - skipping"
    }
}

Write-Host ""
Write-Host "=== Package ready: $OutputDir ===" -ForegroundColor Green

# --- Optional zip ---
if ($Zip) {
    $ZipPath = Join-Path $ProjectRoot "flash-package.zip"
    if (Test-Path $ZipPath) { Remove-Item $ZipPath -Force }
    Compress-Archive -LiteralPath $OutputDir -DestinationPath $ZipPath
    $zipSize = [math]::Round((Get-Item $ZipPath).Length / 1MB, 2)
    Write-Host "Zipped to: $ZipPath  ($zipSize MB)" -ForegroundColor Yellow
}
