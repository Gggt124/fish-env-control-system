# ESP32 Control System - Phase 1 Foundation

Minimal ESP32 web-based control system. Local web server with login, Wi-Fi management, and SoftAP fallback.

## Hardware

- **Board**: ESP32 DevKit V1 (30-pin) or ESP32-S3 compatible
- **Framework**: ESP-IDF (tested with v5.x)

## Features (Phase 1)

- Local web server on ESP32
- Login session system (cookie-based, in-memory)
- Wi-Fi scan / connect / configuration page
- SoftAP fallback mode (`ESP32-Control-Setup`, open network)
- Dashboard with connection status, memory, uptime
- Responsive UI (sidebar on desktop, stacked on mobile)
- Thai language UI labels

## Not Yet Implemented

- OTA firmware update
- Relay / GPIO control
- Sensor telemetry and charts
- MQTT / cloud backend
- SPIFFS / LittleFS file storage
- Real-time WebSocket updates
- Production-grade security
- Multi-user management

## Quick Start

### Prerequisites

ESP-IDF installed and available. Export the environment first:

```powershell
# On Windows (PowerShell)
& "C:\path\to\esp-idf\export.ps1"

# Verify
idf.py --version
```

### Build and Flash

```bash
# 1. Set target chip
idf.py set-target esp32

# 2. Configure (optional - review Wi-Fi/defaults if needed)
idf.py menuconfig

# 3. Build
idf.py build

# 4. Flash and monitor (replace COMx with your port)
idf.py -p COMx flash monitor
```

### How to Connect

1. On your phone/laptop, connect to Wi-Fi network: **ESP32-Control-Setup** (open, no password)
2. Open browser and go to: **http://192.168.4.1**
3. Login with default credentials:
   - **Username**: `admin`
   - **Password**: `admin123`
   > ⚠️ **WARNING**: Change these before any production use!
4. Go to **Wi-Fi Settings** → click **Scan** → select your home Wi-Fi → enter password → **Connect**
5. The ESP32 will connect to your home Wi-Fi while keeping the AP active as fallback

## Project Structure

```
main_dashboard_mcu/
├── CMakeLists.txt              # Root ESP-IDF build
├── sdkconfig.defaults           # Default config (target=esp32)
├── .gitignore
├── README.md
└── main/
    ├── CMakeLists.txt           # Component registration + embed static files
    ├── app_main.c               # Boot sequence
    ├── wifi_manager.h/c         # AP + STA, scan, connect, event handling
    ├── web_server.h/c           # HTTP routes, auth middleware, static serving
    ├── session.h/c              # In-memory login tokens
    ├── nvs_store.h/c            # NVS Wi-Fi credential storage
    └── static/
        ├── login.html           # Login page (Thai UI)
        ├── dashboard.html       # Dashboard with status cards
        ├── wifi.html            # Wi-Fi scan/connect page
        ├── style.css            # Plain CSS design system
        └── app.js               # Vanilla JS (API calls, session, UI)
```

## API Routes

| Method | Path | Auth | Description |
|--------|------|------|-------------|
| GET | `/` | No | Redirect to /dashboard or /login |
| GET | `/login` | No | Login page |
| POST | `/api/login` | No | Login (JSON body: `{username, password}`) |
| POST | `/api/logout` | No | Logout, clear session |
| GET | `/dashboard` | Yes | Dashboard page |
| GET | `/wifi` | Yes | Wi-Fi config page |
| GET | `/style.css` | No | Stylesheet |
| GET | `/app.js` | No | JavaScript |
| GET | `/api/wifi/scan` | Yes | Scan networks → JSON |
| POST | `/api/wifi/connect` | Yes | Connect to SSID (`{ssid, password}`) |
| GET | `/api/status` | Yes | Device status JSON |

## Security Notes

This is a **local-prototype** system. Security is minimal by design:

- Session tokens are random (esp_random) but transmitted as plain HTTP cookies
- Credentials are hardcoded (`admin`/`admin123`) - must be changed for production
- No HTTPS/TLS (ESP32 serves HTTP only)
- No brute-force protection
- No CSRF protection
- Passwords stored in NVS (not encrypted)

**Not suitable for internet-facing deployment without significant hardening.**

## License

MIT
