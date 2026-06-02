param(
    [string]$Port = "COM5",
    [int]$BaudRate = 115200,
    [string]$OutputPath = ""
)

$ErrorActionPreference = "Stop"

chcp 65001 > $null

if (-not $OutputPath) {
    $timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
    $OutputPath = Join-Path "logs" "soak-$timestamp.log"
}

if (-not [System.IO.Path]::IsPathRooted($OutputPath)) {
    $OutputPath = Join-Path (Get-Location).Path $OutputPath
}

$OutputPath = [System.IO.Path]::GetFullPath($OutputPath)
$outputDirectory = Split-Path -Parent $OutputPath
New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null

$serial = [System.IO.Ports.SerialPort]::new(
    $Port,
    $BaudRate,
    [System.IO.Ports.Parity]::None,
    8,
    [System.IO.Ports.StopBits]::One
)
$serial.ReadTimeout = 1000
$serial.DtrEnable = $false
$serial.RtsEnable = $false

$utf8NoBom = [System.Text.UTF8Encoding]::new($false)
$writer = [System.IO.StreamWriter]::new($OutputPath, $true, $utf8NoBom)
$writer.AutoFlush = $true

try {
    $serial.Open()
    $start = Get-Date -Format "yyyy-MM-ddTHH:mm:ss.fffK"
    $header = "$start [monitor-soak] port=$Port baud=$BaudRate output=$OutputPath"
    Write-Host $header
    $writer.WriteLine($header)

    while ($true) {
        try {
            $line = $serial.ReadLine().TrimEnd("`r")
            $timestamp = Get-Date -Format "yyyy-MM-ddTHH:mm:ss.fffK"
            $entry = "$timestamp $line"
            Write-Host $entry
            $writer.WriteLine($entry)
        } catch [System.TimeoutException] {
            continue
        }
    }
} finally {
    if ($serial.IsOpen) {
        $serial.Close()
    }
    $serial.Dispose()
    $writer.Dispose()
}
