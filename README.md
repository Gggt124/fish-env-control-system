# ESP32 Wi-Fi Setup / Web Server Template

Reusable ESP-IDF foundation for ESP32 projects that need local setup over Wi-Fi:
SoftAP fallback, STA Wi-Fi configuration, login/session auth, status JSON, and
embedded HTML/CSS/JS pages.

This repository is intended to be cloned as a starting point for other ESP32
projects. Keep project-specific features such as relays, sensors, MQTT, OTA, and
cloud integrations in the downstream project, not in this template.

## Baseline

- Target: classic ESP32 / ESP32 DevKit V1
- Framework: ESP-IDF only, no Arduino or PlatformIO
- ESP-IDF baseline for this workspace: `C:\esp-idf\`
- Static frontend: embedded files, no CDN, no internet dependency
- Flash layout: 4 MB flash with custom `partitions.csv`

## Features

- SoftAP setup network: `ESP32-Control-Setup`
- Captive-portal DNS fallback on AP clients
- mDNS hostname: `home1.local`
- Login page with in-memory cookie sessions
- Wi-Fi scan/connect/disconnect APIs
- Optional static STA IP fields in the Wi-Fi page
- Device status page and `/api/status`
- Reusable components for NVS, sessions, and Wi-Fi management

## First File To Edit

For a new project, start with:

```text
components/app_config/app_config.h
```

Change these values before touching the Wi-Fi or web-server internals:

- `APP_TEMPLATE_NAME`
- `APP_TEMPLATE_FIRMWARE_VERSION`
- `APP_TEMPLATE_AP_SSID`
- `APP_TEMPLATE_MDNS_HOSTNAME`
- `APP_TEMPLATE_MDNS_INSTANCE_NAME`
- `APP_TEMPLATE_DEFAULT_USERNAME`
- `APP_TEMPLATE_DEFAULT_PASSWORD`
- `APP_TEMPLATE_AP_AUTO_STOP_DEFAULT`
- `APP_TEMPLATE_AP_STOP_TMO_DEFAULT_MS`

Default login is `admin` / `admin123`. Change it before using the template
outside local development.

## Quick Start

From PowerShell:

```powershell
.\scripts\build.ps1
```

Manual equivalent:

```powershell
chcp 65001 > $null
& "C:\esp-idf\export.ps1"
idf.py build
```

Flash and monitor:

```powershell
chcp 65001 > $null
& "C:\esp-idf\export.ps1"
idf.py -p COMx flash monitor
```

Replace `COMx` with the ESP32 serial port.

## Manual Test

1. Flash the firmware.
2. Connect phone or laptop to Wi-Fi `ESP32-Control-Setup`.
3. Open `http://192.168.4.1`.
4. Login with the configured credentials.
5. Open Wi-Fi Settings, scan networks, and connect STA.
6. Reboot and confirm saved STA credentials load from NVS.
7. Confirm AP fallback still works if STA connection fails.

## Project Structure

```text
main_dashboard_mcu/
├── CMakeLists.txt
├── sdkconfig.defaults
├── partitions.csv
├── TEMPLATE.md
├── docs/
│   └── components.md
├── scripts/
│   └── build.ps1
├── components/
│   ├── app_config/       # Template-level constants to customize first
│   ├── nvs_store/        # NVS Wi-Fi credential and IP config storage
│   ├── session/          # In-memory login sessions
│   └── wifi_manager/     # AP+STA, scan, connect, fallback behavior
└── main/
    ├── app_main.c        # Boot sequence and service startup
    ├── web_server.c/h    # HTTP routes, auth, static serving, APIs
    ├── dns_server.c/h    # Captive-portal DNS fallback
    └── static/           # Embedded HTML/CSS/JS
```

## Core API Routes

| Method | Path | Auth | Description |
|--------|------|------|-------------|
| GET | `/` | No | Redirect to dashboard or login |
| GET | `/login` | No | Login page |
| POST | `/api/login` | No | Login |
| POST | `/api/logout` | No | Logout and clear session |
| GET | `/dashboard` | Yes | Example dashboard |
| GET | `/status` | Yes | Full status page |
| GET | `/wifi` | Yes | Wi-Fi setup page |
| GET | `/api/wifi/scan` | Yes | Scan networks |
| POST | `/api/wifi/connect` | Yes | Connect STA |
| POST | `/api/wifi/disconnect` | Yes | Disconnect STA |
| GET | `/api/status` | Yes | Device status JSON |

## Template Boundaries

Keep these as template core:

- `components/app_config`
- `components/nvs_store`
- `components/session`
- `components/wifi_manager`
- login/session routes
- Wi-Fi scan/connect/disconnect routes
- status API

Customize these for each product:

- `main/static/dashboard.html`
- `main/static/status.html`
- branding text in `main/static/*.html`
- product-specific API routes in `main/web_server.c`
- product-specific startup code after the template services in `app_main.c`

## Security Notes

This is a local setup template, not production internet-facing security.

- HTTP only, no TLS
- Cookie session is intentionally readable by JavaScript
- Default credentials are compile-time constants
- Wi-Fi credentials are stored in NVS without encryption in development
- Flash encryption and secure boot are disabled in `sdkconfig.defaults`

For production, add a hardening pass before shipping: unique credentials,
secure boot, flash/NVS encryption, tighter session cookie policy, and a threat
model for the final network exposure.

## Validation

The template is valid when:

```powershell
.\scripts\build.ps1
```

finishes successfully and `build\main_dashboard_mcu.bin` is generated.

No unit tests are included in this phase.
