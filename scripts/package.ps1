<#
.SYNOPSIS
    Ã Â¹â‚¬Ã Â¸ÂÃ Â¹â€¡Ã Â¸Å¡Ã Â¹â€žÃ Â¸Å¸Ã Â¸Â¥Ã Â¹Å’ .bin Ã Â¸Ë†Ã Â¸Â²Ã Â¸Â build/ Ã Â¹ÂÃ Â¸Â¥Ã Â¸Â°Ã Â¸ÂªÃ Â¸â€žÃ Â¸Â£Ã Â¸Â´Ã Â¸â€ºÃ Â¸â€¢Ã Â¹Å’Ã Â¹â‚¬Ã Â¸ÂªÃ Â¸Â£Ã Â¸Â´Ã Â¸Â¡ Ã Â¹â‚¬Ã Â¸Å¾Ã Â¸Â·Ã Â¹Ë†Ã Â¸Â­Ã Â¹â‚¬Ã Â¸â€¢Ã Â¸Â£Ã Â¸ÂµÃ Â¸Â¢Ã Â¸Â¡Ã Â¹â€šÃ Â¸Å¸Ã Â¸Â¥Ã Â¹â‚¬Ã Â¸â€Ã Â¸Â­Ã Â¸Â£Ã Â¹Å’ flash-package/ Ã Â¸ÂªÃ Â¸Â³Ã Â¸Â«Ã Â¸Â£Ã Â¸Â±Ã Â¸Å¡Ã Â¹ÂÃ Â¸Ë†Ã Â¸ÂÃ Â¸Â¥Ã Â¸Â¹Ã Â¸ÂÃ Â¸â€žÃ Â¹â€°Ã Â¸Â²
.PARAMETER OutputDir
    Path Ã Â¸â€ºÃ Â¸Â¥Ã Â¸Â²Ã Â¸Â¢Ã Â¸â€”Ã Â¸Â²Ã Â¸â€¡Ã Â¸â€šÃ Â¸Â­Ã Â¸â€¡ flash-package/ (default: .\flash-package)
.PARAMETER Zip
    Ã Â¹â‚¬Ã Â¸â€ºÃ Â¸Â´Ã Â¸â€Ã Â¹â‚¬Ã Â¸Å¾Ã Â¸Â·Ã Â¹Ë†Ã Â¸Â­ pass flag Ã Â¸â„¢Ã Â¸ÂµÃ Â¹â€°Ã Â¹â‚¬Ã Â¸Å¾Ã Â¸Â·Ã Â¹Ë†Ã Â¸Â­Ã Â¸ÂªÃ Â¸Â±Ã Â¹Ë†Ã Â¸â€¡ zip flash-package/ Ã Â¹â‚¬Ã Â¸â€ºÃ Â¹â€¡Ã Â¸â„¢ flash-package.zip Ã Â¸â€Ã Â¹â€°Ã Â¸Â§Ã Â¸Â¢
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

# --- Ã Â¸â€¢Ã Â¸Â£Ã Â¸Â§Ã Â¸Ë†Ã Â¸ÂªÃ Â¸Â­Ã Â¸Å¡Ã Â¹â€žÃ Â¸Å¸Ã Â¸Â¥Ã Â¹Å’ binary Ã Â¸â€”Ã Â¸ÂµÃ Â¹Ë†Ã Â¸Ë†Ã Â¸Â³Ã Â¹â‚¬Ã Â¸â€ºÃ Â¹â€¡Ã Â¸â„¢Ã Â¸â€¢Ã Â¹â€°Ã Â¸Â­Ã Â¸â€¡Ã Â¹Æ’Ã Â¸Å Ã Â¹â€° ---
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

# --- à¹€à¸•à¸£à¸µà¸¢à¸¡à¹‚à¸„à¸£à¸‡à¸ªà¸£à¹‰à¸²à¸‡ output directory ---
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
foreach ($tool in @("flash_and_show.ps1", "show_password.ps1")) {
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
