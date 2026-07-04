<#
.SYNOPSIS
    สร้าง flash-package/ สำหรับส่งให้ลูกค้า
.PARAMETER OutputDir
    Path ของ flash-package/ (default: .\flash-package)
.PARAMETER Zip
    ถ้า pass flag นี้ จะ zip flash-package/ เป็น flash-package.zip ด้วย
.EXAMPLE
    .\scripts\package.ps1
    .\scripts\package.ps1 -Zip
#>
param(
    [string]$OutputDir = ".\flash-package",
    [switch]$Zip
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path $PSScriptRoot -Parent
$BuildDir    = Join-Path $ProjectRoot "build"
$ScriptsDir  = $PSScriptRoot

# --- ตรวจสอบ binary ที่ต้องการ ---
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

# --- เตรียม output directory ---
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
foreach ($tool in @("flash_and_show.ps1", "show_password.ps1")) {
    $src  = Join-Path $ScriptsDir $tool
    $dest = Join-Path $ToolsDir $tool
    if (Test-Path $src) {
        Copy-Item -LiteralPath $src -Destination $dest -Force
        Write-Host "  OK  tools\$tool" -ForegroundColor Green
    } else {
        Write-Warning "  MISSING  scripts\$tool — skipping"
    }
}

# --- Copy batch launchers & README ---
foreach ($file in @("FLASH.bat", "README_password.bat", "README_ลูกค้า.md", "README_customer.md")) {
    $src  = Join-Path $ScriptsDir $file
    $dest = Join-Path $OutputDir $file
    if (Test-Path $src) {
        Copy-Item -LiteralPath $src -Destination $dest -Force
        Write-Host "  OK  $file" -ForegroundColor Green
    } else {
        Write-Warning "  MISSING  scripts\$file — skipping"
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
