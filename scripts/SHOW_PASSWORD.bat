@echo off
:: Launches show_password.ps1 from the 'tools\' subfolder next to this file.
:: (In a customer package, 'tools\' is placed here automatically by scripts\package.ps1.)
PowerShell -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\show_password.ps1"
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] PowerShell script exited with code %errorlevel%
    pause
)
