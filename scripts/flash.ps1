param(
    [string]$Port = "COM5"
)

$env:IDF_PATH = "C:\esp\v6.0.1\esp-idf"
$env:IDF_TOOLS_PATH = "C:\Espressif"
chcp 65001 > $null
$env:PYTHONUTF8 = "1"

& "$env:IDF_PATH\export.ps1"
idf.py -p $Port flash
