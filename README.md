<!-- generated-by: gsd-doc-writer -->
# Fish Pump Relay Timer Control

Firmware for the ESP32 (classic) and ESP32-S3 boards that controls a fish pump relay using two configurable timers and one binary float switch. It also controls a digital temperature-based cooling relay and features a local web configuration dashboard, captive portal, security session control, dual-OTA updates, and a TFT status dashboard.

---

## Table of Contents
1. [Core Features](#core-features)
2. [Hardware Profiles & GPIO Pin Mappings](#hardware-profiles--gpio-pin-mappings)
3. [Wiring & Electrical Safety](#wiring--electrical-safety)
4. [Operational Logic](#operational-logic)
   - [Pump Timer Control](#pump-timer-control)
   - [Cooling Control](#cooling-control)
5. [TFT Dashboard & Backlight Dimming](#tft-dashboard--backlight-dimming)
6. [Local Web Dashboard & Captive Portal](#local-web-dashboard--captive-portal)
7. [REST API Endpoint Reference](#rest-api-endpoint-reference)
8. [Failure Recovery & Watchdog Diagnostics](#failure-recovery--watchdog-diagnostics)
9. [Developer Environment & Sourcing](#developer-environment--sourcing)
10. [Build, Flash, and Package Commands](#build-flash-and-package-commands)

---

## Core Features

- **Dual Hardware Support**: Seamlessly targets both classic ESP32 and ESP32-S3 boards.
- **Float-Based Dual Timer**: Switches pump control between Timer 1 (Float OFF) and Timer 2 (Float ON).
- **Temperature Cooling Control**: Automates cooling relays using a DS18B20 digital sensor with compressor protection lockouts.
- **Dynamic Hardware Mapping**: Configure and save GPIO roles at runtime via the web installer page.
- **TFT Status Display**: Real-time status screens using ST7789 or ILI9341 over SPI.
- **Local Web Server**: Captive DNS fallback, secure session control, and HTML/CSS/JS frontend served offline.
- **OTA Updates**: Pre-encrypted firmware uploads over HTTP using a 256-bit AES key.
- **Diagnostics & Watchdog**: System health monitoring, task watchdog, and OOM strike auto-reboots.

---

## Hardware Profiles & GPIO Pin Mappings

The project natively supports two hardware profiles. The active profile is determined at compile time.

| Role | ESP32 Classic Default GPIO | ESP32-S3 Default GPIO | Notes |
|:---|:---:|:---:|:---|
| **Float Input** | GPIO32 | GPIO19 | Binary float switch input. Active-low with internal pull-up. |
| **Pump Relay 1** | GPIO26 | GPIO20 | Relay control for Pump channel 1. |
| **Pump Relay 2** | GPIO27 | GPIO21 | Relay control for Pump channel 2. |
| **DS18B20 Data** | GPIO33 | GPIO41 | One-wire temperature sensor communication bus. |
| **Cooling Relay** | GPIO25 | GPIO42 | Relay control for cooling compressor/fan. |
| **TFT CS** | GPIO5 | GPIO10 | SPI chip select line. |
| **TFT RESET** | GPIO22 | GPIO14 | LCD reset control line. |
| **TFT DC** | GPIO21 | GPIO13 | Data/Command selector line. |
| **TFT MOSI** | GPIO23 | GPIO11 | SPI MOSI data line. |
| **TFT SCK** | GPIO18 | GPIO12 | SPI SCK clock line. |
| **TFT LED** | GPIO4 | GPIO9 | Backlight LED PWM control line. |
| **Status LED** | GPIO2 | GPIO2 | Board status LED (strapping pin warning). |
| **External Button** | GPIO14 | GPIO38 | Manual hardware control button. |
| **External LED** | GPIO13 | GPIO39 | External panel indicator LED. |
| **Boot/Flash Button**| GPIO0 | GPIO0 | Standard ESP Boot button. |

---

## Wiring & Electrical Safety

### 1. Float Switch Pull-Up
ESP32/S3 internal pull-ups (~45 kOhm) can fail to reject electromagnetic interference (EMI) from the AC pump motor's magnetic field.
- **Recommendation**: Wire an external **4.7 kOhm pull-up resistor** between the configured Float GPIO and 3.3V.

### 2. DS18B20 Wiring (Powered Mode)
- Connect DS18B20 `VDD` to `3.3V`, `GND` to `GND`, and `DQ` to the configured Data GPIO.
- **Required**: Connect an external **4.7 kOhm pull-up resistor** between `DQ` and `3.3V`. Do not run in parasitic power mode.

### 3. Strapping Pin Hazards
- **GPIO0**: Acts as the boot-mode selection strapping pin. Avoid wiring pull-downs or components that pull this pin low on startup.
- **GPIO2**: Strapping pin. Avoid external pull-ups.

### 4. Relay Polarity
Relay modules vary (active-high vs. active-low). The firmware defaults to active-low but supports runtime software inversion. Always verify polarity configurations before connecting high-voltage pump or cooling compressor loads.

---

## Operational Logic

### Pump Timer Control
The system evaluates the debounced state of the float switch to select the active timer profile:
- **Float OFF (Normal)**: Selects **Timer 1** (Default: 20s ON / 60s OFF). Drives **Pump Relay 1**.
- **Float ON (High level)**: Selects **Timer 2** (Default: 10s ON / 180s OFF). Drives **Pump Relay 2**.

When a timer is selected, it executes its custom ON/OFF cycle. The relay follows the active phase. Auto-start on boot can be toggled by the user (disabled by default).

### Cooling Control
Governs temperature regulation using the DS18B20 digital sensor:
1. **Auto Mode**:
   - Turns cooling relay **ON** when temperature exceeds `threshold_c` (Default: 30.0 C).
   - Turns cooling relay **OFF** when temperature drops below `threshold_c - hysteresis_c` (Default: 29.0 C).
2. **Force OFF**: Immediately disables the cooling relay.
3. **Test ON**: Runs cooling temporarily for a limited duration (Default: 10 seconds).
4. **Compressor Lockout (Safety)**:
   - To protect compressor valves from backpressure, a minimum off-time (Default: 180 seconds) is enforced.
   - When the relay turns off, it enters a lockout period. Auto/Test modes cannot turn the relay ON until the lockout timer expires.
   - Sensor connection fault is declared after 3 consecutive read failures, immediately shutting off cooling.

---

## TFT Dashboard & Backlight Dimming

The device features a 320x240 landscape TFT status display (ILI9341 or ST7789) driven by the ESP-IDF `esp_lcd` component. It renders a real-time system monitoring dashboard.

- **Inactivity Backlight Control**:
  - Automatically dims the backlight to `70%` brightness after 5 minutes of inactivity.
  - Inactivity is tracked across both hardware events (switch toggles, timer transitions) and web API calls.
  - Active operations or web commands restore brightness to 100% instantly.

---

## Local Web Dashboard & Captive Portal

When the device boots, it starts a local Wi-Fi Access Point and Web Server to allow local management without internet connectivity:
- **SSID**: `FishPump-Setup` (Open network, no password)
- **Local IP Address**: `192.168.4.1`
- **mDNS Hostname**: `http://fishpump.local`
- **Default Credentials**: Username `admin`, Password `admin123` (Must be changed upon configuration).
- **Captive Portal**: DNS queries from connected devices are intercepted, redirecting all HTTP probes (e.g. NCSI, Connectivity Test) to the local login page automatically.

---

## REST API Endpoint Reference

All routes except `/api/login` and `/api/auth/nonce` require a valid HTTP Session Cookie.

| Method | Path | Description | Request/Response Payload Example |
|:---|:---|:---|:---|
| **GET** | `/api/status` | Aggregate device metrics and system stats. | `{"ok":true,"uptime_sec":123,"free_heap":45612}` |
| **POST** | `/api/login` | Authenticate username/password and set session cookie. | `{"username":"admin","password":"..."}` |
| **POST** | `/api/logout` | Clear user session token from device RAM. | `{"ok":true}` |
| **POST** | `/api/change-password` | Update current login credentials. | `{"old_password":"...","new_password":"..."}` |
| **GET** | `/api/wifi/scan` | Trigger active channel scan and return local networks. | `{"ok":true,"networks":[{"ssid":"Home","rssi":-62}]}` |
| **POST** | `/api/wifi/connect` | Save STA credentials and attempt connection. | `{"ssid":"Home","password":"..."}` |
| **POST** | `/api/wifi/disconnect` | Disconnect from active Station and forget credentials. | `{"ok":true}` |
| **GET** | `/api/wifi/profiles` | List saved Wi-Fi profiles and auto-connect settings. | `{"ok":true,"profiles":[...],"auto_connect":true}` |
| **GET** | `/api/pump/status` | Current pump running, float state, and countdown timer. | `{"running":true,"float_state":"OFF","countdown":12}` |
| **GET** | `/api/pump/config` | Retrieve configurable pump relay and timer variables. | `{"timer1":{"on_sec":20,"off_sec":60},"auto_start":false}` |
| **POST** | `/api/pump/config` | Update pump durations and active polarities. | `{"timer1":{"on_sec":10,"off_sec":30}}` |
| **POST** | `/api/pump/start` | Start the pump execution cycle manually. | `{"ok":true}` |
| **POST** | `/api/pump/stop` | Stop the pump execution cycle manually. | `{"ok":true}` |
| **GET** | `/api/cooling/status` | Read DS18B20 temperature, relay status, and lockouts. | `{"temperature":28.5,"relay_active":false,"lockout":0}` |
| **GET** | `/api/cooling/config` | Read cooling threshold, hysteresis, and protection limits. | `{"threshold_x10":300,"hysteresis_x10":10}` |
| **POST** | `/api/cooling/config` | Save persistent cooling threshold, lockout, and modes. | `{"threshold_x10":280,"hysteresis_x10":15}` |
| **POST** | `/api/cooling/mode` | Switch cooling between `auto`, `force_off`, or temporary `test_on`.| `{"mode":"auto"}` |
| **GET** | `/api/hardware/map` | Read active/pending GPIO mappings and safe options. | `{"active":{"float":32,"r1":26},"pending_reboot":false}` |
| **POST** | `/api/hardware/map` | Stage new GPIO pins (Requires reboot confirmation). | `{"float":19,"r1":20,"confirm_reboot_required":true}` |
| **GET** | `/api/display/config` | Retrieve LCD backlight percent and dim timing settings. | `{"brightness":100,"dim_percent":70,"dim_timeout":300}` |
| **POST** | `/api/display/config` | Update default LCD backlight settings. | `{"dim_percent":50}` |
| **POST** | `/api/display/wake` | Reset screen saver backlight sleep timer. | `{"ok":true}` |
| **POST** | `/api/ota` | Upload firmware image byte chunks (Pre-Encrypted OTA). | Multipart binary payload |

---

## Failure Recovery & Watchdog Diagnostics

To ensure high availability, the device monitors runtime stability:
- **Task Watchdog (WDT)**: Main loops must feed the watchdog timer every 15 seconds. If a task freezes, the hardware watchdog triggers an automatic system reboot.
- **OOM Recovery Strikes**:
  - Below `10KB` free heap or `5KB` contiguous free block, a strike is logged.
  - If strikes accumulate consecutively over 15 seconds (3 main loops), the firmware triggers a restart to release memory leaks.
- **HTTP Hung Recovery**: In case of socket exhaustion, TCP keep-alives and LRU socket purges reclaim resources from unresponsive clients.

---

## Developer Environment & Sourcing

Development requires the **ESP-IDF v6.0.1** (or compatible) framework.

### 1. PowerShell Setup & Unicode Workaround
`idf.py` may experience Unicode character page errors on Windows command prompts. Fix this by setting UTF-8 encoding in PowerShell:
```powershell
# Sourcing environment
$env:IDF_PATH = "C:\esp\v6.0.1\esp-idf"  # Update to your local path
$env:IDF_TOOLS_PATH = "C:\Espressif"
& "$env:IDF_PATH\export.ps1"

# Enable Unicode terminal workaround
chcp 65001 > $null
```

### 2. Python Dependencies
If python dependencies are missing in your ESP-IDF environment, run:
```powershell
python -m pip install -r "$env:IDF_PATH\tools\requirements\requirements.core.txt"
```

---

## Build, Flash, and Package Commands

Use the custom automation scripts located in the `scripts/` folder to manage compile targets:

### 1. Target Profile Cleaning
When switching builds between classic `esp32` and `esp32s3` profiles, you **MUST delete the existing configuration file** first:
```powershell
# Delete target cache configuration
Remove-Item sdkconfig -ErrorAction SilentlyContinue
```

### 2. Compile Targets
```powershell
# Build for ESP32 DevKit V1
.\scripts\build.ps1 -Target esp32

# Build for ESP32-S3 WROOM-1
.\scripts\build.ps1 -Target esp32s3
```

### 3. Flash and Monitor
To flash the compiled binary and monitor logs, use the interactive PowerShell menu helper:
```powershell
.\scripts\flash_and_show.ps1
```

### 4. Release Packaging
To build and bundle a flash deployment zip package containing offline binaries and batch installer scripts:
```powershell
.\scripts\package.ps1 -Target all -Zip
```
