<#
.SYNOPSIS
    Package firmware binaries into flash-package/ for customer distribution.
.PARAMETER Target
    Profile to package: 'esp32', 'esp32s3', or 'all' (both). Default: all
.PARAMETER OutputDir
    Destination. Default: .\flash-package
.PARAMETER Zip
    Also compress flash-package/ to flash-package.zip
.EXAMPLE
    .\scripts\package.ps1
    .\scripts\package.ps1 -Target esp32s3
    .\scripts\package.ps1 -Target all -Zip
#>
param(
    [ValidateSet("all", "esp32", "esp32s3")]
    [string]$Target = "all",
    [string]$OutputDir = ".\flash-package",
    [switch]$Zip
)

$ErrorActionPreference = "Stop"
chcp 65001 > $null

$ProjectRoot = Split-Path $PSScriptRoot -Parent
$ScriptsDir  = $PSScriptRoot
$Targets     = if ($Target -eq "all") { @("esp32", "esp32s3") } else { @($Target) }
$BinName     = "fish_pump_relay_timer_control"

Write-Host "[package] Checking standalone esptool.exe..." -ForegroundColor Cyan
$esptoolExe = Join-Path $ScriptsDir "esptool.exe"
if (-not (Test-Path $esptoolExe)) {
    Write-Host "  Downloading esptool v5.3.1..." -ForegroundColor Yellow
    $url = "https://github.com/espressif/esptool/releases/download/v5.3.1/esptool-v5.3.1-windows-amd64.zip"
    $zipPath = Join-Path $ScriptsDir "esptool.zip"
    $tmpDir  = Join-Path $ScriptsDir "esptool_tmp"
    $ProgressPreference = 'SilentlyContinue'
    Invoke-WebRequest -Uri $url -OutFile $zipPath
    Expand-Archive -Path $zipPath -DestinationPath $tmpDir -Force
    Copy-Item (Join-Path $tmpDir "esptool-windows-amd64\esptool.exe") -Destination $esptoolExe -Force
    Remove-Item $tmpDir -Recurse -Force; Remove-Item $zipPath -Force
    Write-Host "  Downloaded esptool.exe" -ForegroundColor Green
}

# Validate all build dirs BEFORE touching output
foreach ($t in $Targets) {
    $bd = Join-Path $ProjectRoot "build-$t"
    @("$bd\bootloader\bootloader.bin",
      "$bd\partition_table\partition-table.bin",
      "$bd\ota_data_initial.bin",
      "$bd\$BinName.bin") | ForEach-Object {
        if (-not (Test-Path $_)) {
            Write-Error "Missing binary for '$t': $_`nRun: .\scripts\build.ps1 -Target $t"
        }
    }
}
Write-Host "[package] All required binaries present." -ForegroundColor Green

if (Test-Path $OutputDir) { Remove-Item $OutputDir -Recurse -Force }
$ToolsDir = Join-Path $OutputDir "tools"
New-Item -ItemType Directory -Path $ToolsDir | Out-Null

foreach ($t in $Targets) {
    $bd  = Join-Path $ProjectRoot "build-$t"
    $fwd = Join-Path $OutputDir "firmware\$t"
    New-Item -ItemType Directory -Path $fwd | Out-Null
    Write-Host "[package] Copying firmware for target: $t ..." -ForegroundColor Cyan
    $filemap = [ordered]@{
        "$bd\bootloader\bootloader.bin"           = "$fwd\bootloader.bin"
        "$bd\partition_table\partition-table.bin" = "$fwd\partition-table.bin"
        "$bd\ota_data_initial.bin"                = "$fwd\ota_data_initial.bin"
        "$bd\$BinName.bin"                        = "$fwd\$BinName.bin"
    }
    foreach ($entry in $filemap.GetEnumerator()) {
        Copy-Item -LiteralPath $entry.Key -Destination $entry.Value -Force
        $sz = (Get-Item $entry.Value).Length
        Write-Host "  OK  firmware\$t\$(Split-Path $entry.Value -Leaf)  ($sz bytes)" -ForegroundColor Green
    }
}

Write-Host "[package] Copying tools..." -ForegroundColor Cyan
foreach ($tool in @("flash_and_show.ps1", "show_password.ps1", "esptool.exe")) {
    $src = Join-Path $ScriptsDir $tool; $dest = Join-Path $ToolsDir $tool
    if (Test-Path $src) { Copy-Item -LiteralPath $src -Destination $dest -Force; Write-Host "  OK  tools\$tool" -ForegroundColor Green }
    else { Write-Warning "  MISSING  scripts\$tool -- skipping" }
}

Write-Host "[package] Copying launchers and README..." -ForegroundColor Cyan
$extras = @("FLASH.bat", "SHOW_PASSWORD.bat", "README_customer.md")
$extras += (Get-ChildItem -Path $ScriptsDir -Filter "README_*.md" -ErrorAction SilentlyContinue |
    Select-Object -ExpandProperty Name | Where-Object { $_ -ne "README_customer.md" })
foreach ($f in $extras) {
    if ([string]::IsNullOrWhiteSpace($f)) { continue }
    $src = Join-Path $ScriptsDir $f; $dest = Join-Path $OutputDir $f
    if (Test-Path $src) { Copy-Item -LiteralPath $src -Destination $dest -Force; Write-Host "  OK  $f" -ForegroundColor Green }
    else { Write-Warning "  MISSING  $f -- skipping" }
}

Write-Host ""
Write-Host "=== Package ready: $OutputDir ===" -ForegroundColor Green

if ($Zip) {
    $ZipPath = Join-Path $ProjectRoot "flash-package.zip"
    if (Test-Path $ZipPath) { Remove-Item $ZipPath -Force }
    Compress-Archive -LiteralPath $OutputDir -DestinationPath $ZipPath
    $sz = [math]::Round((Get-Item $ZipPath).Length / 1MB, 2)
    Write-Host "Zipped: $ZipPath  ($sz MB)" -ForegroundColor Yellow
}
