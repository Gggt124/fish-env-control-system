# Fish Pump Relay Timer Control

ESP-IDF firmware for a local fish pump relay timer controller. The firmware
keeps the original SoftAP fallback, STA Wi-Fi setup, login/session auth, and
embedded HTML/CSS/JS web UI while adding dual pump relays, a DS18B20 cooling
channel, and an installer-facing hardware map flow.

## Baseline

- Target: classic ESP32 / ESP32 DevKit V1
- Framework: ESP-IDF only, no Arduino or PlatformIO
- ESP-IDF path: set `IDF_PATH` to your ESP-IDF install path (for example `C:\esp\v6.0.1\esp-idf`)
- Static frontend: embedded files, no CDN, no internet dependency
- Flash layout: 4 MB flash with custom dual-OTA `partitions.csv`
- Hardware contract: safe ESP32 DevKit V1 defaults are documented in
  `docs/hardware.md`
- GPIO changes: saved as pending values and applied only after reboot

## Features

- SoftAP setup network: `FishPump-Setup`
- Captive-portal DNS fallback on AP clients
- mDNS hostname: `fish-pump.local`
- Login page with in-memory cookie sessions
- Wi-Fi scan/connect/disconnect APIs
- Optional static STA IP fields in the Wi-Fi page
- Owner dashboard for pump timers, relay state, float state, cooling status,
  cooling threshold, boot auto-enable, Force OFF, and runtime Test ON
- Hardware/Install page with wiring-first GPIO summary, safe dropdown options,
  pending map save, and reboot-required messaging
- Device status page and `/api/status`
- Reusable components for NVS, sessions, Wi-Fi, hardware map, pump runtime, and
  cooling runtime

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

Default login is `admin` / `admin123`. Change it before using the firmware
outside local development.

## Quick Start

From PowerShell:

```powershell
.\scripts\build.ps1
```

Clean old build output and rebuild:

```powershell
.\scripts\build.ps1 -FullClean
```

If ESP-IDF is installed elsewhere:

```powershell
.\scripts\build.ps1 -IdfPath "D:\esp-idf"
```

Manual equivalent:

```powershell
chcp 65001 > $null
$env:IDF_PATH = "C:\esp\v6.0.1\esp-idf"  # example; skip this if IDF_PATH is already set
& "$env:IDF_PATH\export.ps1"
idf.py build
```

Flash:

```powershell
.\scripts\flash.ps1 -Port COMx
```

Flash and keep serial monitor open:

```powershell
.\scripts\flash.ps1 -Port COMx -Monitor
```

Replace `COMx` with the ESP32 serial port.

The flash wrapper applies the same Windows encoding workaround as the build
wrapper (`chcp 65001` and `PYTHONUTF8=1`) before exporting ESP-IDF.

## Manual Test

1. Flash the firmware.
2. Connect phone or laptop to Wi-Fi `FishPump-Setup`.
3. Open `http://192.168.4.1`.
4. Login with the configured credentials.
5. Confirm `/dashboard` loads pump runtime state, cooling runtime state, and
   links to Wi-Fi, status, and Hardware/Install.
6. Open `/hardware`, confirm active GPIOs are visible, and save a pending map
   only if you are prepared to reboot and validate wiring.
7. Open Wi-Fi Settings, scan networks, and connect STA.
8. Reboot and confirm saved STA credentials load from NVS.
9. Confirm AP fallback still works if STA connection fails.

## Project Structure

```text
fish_pump_relay_timer_control/
├── CMakeLists.txt
├── sdkconfig.defaults
├── partitions.csv
├── TEMPLATE.md
├── docs/
│   ├── components.md
│   └── hardware.md
├── scripts/
│   ├── build.ps1
│   └── flash.ps1
├── components/
│   ├── app_config/       # Template-level constants to customize first
│   ├── hardware_map/     # Safe role-based ESP32 GPIO map
│   ├── nvs_store/        # NVS Wi-Fi, hardware, pump, and cooling storage
│   ├── pump_control/     # Dual timer/relay pump runtime
│   ├── cooling_control/  # DS18B20 cooling runtime and relay control
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
| GET | `/dashboard` | Yes | Owner pump and cooling dashboard |
| GET | `/status` | Yes | Full status page |
| GET | `/wifi` | Yes | Wi-Fi setup page |
| GET | `/hardware` | Yes | Hardware/Install wiring and GPIO map page |
| GET | `/api/wifi/scan` | Yes | Scan networks |
| POST | `/api/wifi/connect` | Yes | Connect STA |
| POST | `/api/wifi/disconnect` | Yes | Disconnect STA |
| GET | `/api/status` | Yes | Device status JSON |
| GET | `/api/pump/config` | Yes | Pump timers, auto-start, relay polarity, start phases, read-only active hardware fields |
| POST | `/api/pump/config` | Yes | Save pump timers, auto-start, Relay 1/Relay 2 polarity, and Timer 1/Timer 2 start phases |
| GET | `/api/pump/status` | Yes | Dual-relay pump runtime status |
| POST | `/api/pump/start` | Yes | Start pump controller |
| POST | `/api/pump/stop` | Yes | Stop pump controller and force pump relays inactive |
| GET | `/api/hardware/map` | Yes | Active GPIO map, pending GPIO map, reboot-required status, and safe GPIO options |
| POST | `/api/hardware/map` | Yes | Save pending GPIO map after `confirm_reboot_required:true`; takes effect after reboot |
| GET | `/api/cooling/status` | Yes | Cooling runtime status, sensor state, relay state, lockout, and Test ON countdown |
| GET | `/api/cooling/config` | Yes | Cooling settings, input limits, enum values, and current status |
| POST | `/api/cooling/config` | Yes | Save cooling threshold, hysteresis, auto-enable, mode, test timeout, min-off, and relay polarity |
| POST | `/api/cooling/mode` | Yes | Runtime Auto, Force OFF, or Test ON; Test ON is not persisted |

GPIO pin changes are saved as pending values and require reboot before they
become active. The Hardware/Install UI consumes `/api/hardware/map` dropdown
options directly from firmware, so the frontend does not duplicate safe GPIO
lists or allow freeform GPIO entry. Cooling Test ON is a runtime-only action via
`/api/cooling/mode`; it is not saved as a boot mode.

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

finishes successfully and `build\fish_pump_relay_timer_control.bin` is generated.

No unit tests are included in this phase.
