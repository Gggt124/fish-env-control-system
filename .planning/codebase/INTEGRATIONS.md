# External Integrations

**Analysis Date:** 2026-06-14

## APIs & External Services

**APIs/Services:**
- None. The system operates strictly as a local-only appliance without external internet connectivity.

## Data Storage

**Databases / Persistent Store:**
- ESP32 NVS (Non-Volatile Storage) - Primary persistent storage for configuration profiles
  - Connection: Native ESP-IDF NVS flash APIs (wrapped by the `nvs_store` component)
  - Namespaces:
    - `"wifi_cfg"` - Staging Wi-Fi SSID/password, credentials, and static IP configuration
    - `"wifi_prof"` - Saved Wi-Fi connection profiles (multi-profile store)
    - `"pump_cfg"` - Pump timer intervals, polarity, auto-start configs
    - `"cool_cfg"` - Cooling temperature thresholds, hysteresis, auto-enable mode
    - `"hw_map"` - GPIO pin assignments for relays, float switches, sensors, and screen control
  - Life cycle: Loaded on boot; modified via web UI API endpoints; can be reset to factory defaults via the hardware BOOT/EXT buttons.

## Authentication & Identity

**Auth Provider:**
- Custom Session Manager - Simple, local credential validation and in-memory session tracking
  - Implementation: `components/session/session.c` manages up to 4 concurrent session tokens
  - Token generation: 64-character hex string generated using `esp_random()`
  - Token storage: Client-side session cookie (`session`), which is non-HttpOnly to allow frontend redirect logic
  - Authorization: Web server matches the request cookie against active session slots on protected routes

## Monitoring & Observability

**Logs:**
- UART Console (stdout) - Native ESP-IDF logging framework (`esp_log.h`)
  - Integration: Serial connection via USB (COM port monitored using `idf.py monitor`)
  - Periodic Diagnostics: Status loop in `app_main.c` logs uptime, task count, TWDT status, reset reason, internal heap memory info, pump timers, cooling sensors, and Wi-Fi manager state every 30 seconds

## CI/CD & Deployment

**Hosting:**
- Local ESP32 Flash Memory - Embedded HTTP server serves compressed assets directly from flash symbols (compiled into the binary via assembly `EMBED_FILES` in CMake)

**CI/CD Pipeline:**
- Build automation: `scripts/build.ps1` compiles the ESP-IDF project with UTF8 command-page workarounds
- Flash automation: Local command `idf.py -p COMx flash` uploads the compiled firmware binaries and partition table over USB/UART

## Environment Configuration

**Development & Production:**
- Configuration resides in `components/app_config/app_config.h` (pins, SSID, default credentials, timing defaults)
- NVS configuration can be modified dynamically at runtime by sending POST requests to `/api/wifi/connect`, `/api/pump/config`, `/api/cooling/config`, and `/api/hardware/map`

---

*Integration audit: 2026-06-14*
*Update when adding/removing external services*
