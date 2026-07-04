@echo off
:: Call script from tools/flash_and_show.ps1
PowerShell -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\flash_and_show.ps1"
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] PowerShell script exited with code %errorlevel%
    pause
)
