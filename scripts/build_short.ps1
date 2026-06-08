param(
    [string]$IdfPath = $env:IDF_PATH,
    [string]$IdfToolsPath = $env:IDF_TOOLS_PATH
)
$ErrorActionPreference = "Stop"
chcp 65001 > $null
$env:PYTHONUTF8 = "1"
$env:IDF_PATH = "C:\esp\v6.0.1\esp-idf"
$env:IDF_TOOLS_PATH = "C:\Espressif"
& "$env:IDF_PATH\export.ps1"
idf.py -B C:\temp\fish-build build
