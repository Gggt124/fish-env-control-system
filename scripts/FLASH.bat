@echo off
:: Launches flash_and_show.ps1 from the 'tools\' subfolder next to this file.
:: (In a customer package, 'tools\' is placed here automatically by scripts\package.ps1.)
PowerShell -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\flash_and_show.ps1"
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] PowerShell script exited with code %errorlevel%
    pause
)
