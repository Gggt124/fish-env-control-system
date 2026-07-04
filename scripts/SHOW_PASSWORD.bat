@echo off
:: Call script from tools/show_password.ps1
PowerShell -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\show_password.ps1"
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] PowerShell script exited with code %errorlevel%
    pause
)
