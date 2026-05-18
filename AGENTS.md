# AGENTS.md

## Project

Fish Pump Relay Timer Control. ESP-IDF firmware with local web server, login/session, Wi-Fi config, SoftAP fallback.

- **Target**: ESP32 DevKit V1 (classic ESP32), ESP-IDF framework only (not Arduino/PlatformIO)
- **Scope**: Wi-Fi setup/web server foundation for a downstream pump controller. Do not add relay GPIO/timer behavior until the exact ESP32 variant, board, relay pin map, and timing requirements are explicit.

## Environment

ESP-IDF must be installed and exported before every `idf.py` command. Prefer
setting `IDF_PATH` to the local ESP-IDF install path:

```powershell
$env:IDF_PATH = "C:\esp-idf"  # example; skip this if IDF_PATH is already set
& "$env:IDF_PATH\export.ps1"
```

**Unicode workaround**: `idf.py` warns about Unicode charset on Windows. Run this first in each PowerShell session:

```powershell
chcp 65001 > $null
```

If Python dependencies are missing, run:

```powershell
idf.py --version
python -m pip install -r "$env:IDF_PATH\tools\requirements\requirements.core.txt"
```

## Build / Flash / Monitor

```powershell
# One-time target setup
idf.py set-target esp32

# Build
idf.py build

# Flash + monitor (replace COMx)
idf.py -p COMx flash monitor

# Clean build
idf.py fullclean
idf.py build
```

Run sequentially: `chcp → export → idf.py`. Build output is in `build/` (gitignored).

If a board was previously flashed with flash encryption enabled, disable it once in development mode by building/flashing plaintext firmware, then burning `FLASH_CRYPT_CNT` so the value becomes even. After that, regular `idf.py flash` works again.

## Project Structure

```
fish_pump_relay_timer_control/
├── CMakeLists.txt              # Root ESP-IDF build, EXTRA_COMPONENT_DIRS → components/
├── sdkconfig.defaults           # CONFIG_IDF_TARGET="esp32", custom partition table, dev flash workflow
├── components/
│   ├── app_config/             # Template constants: names, AP SSID, mDNS, credentials
│   ├── nvs_store/              # Reusable: NVS Wi-Fi credential I/O
│   │   ├── CMakeLists.txt       # REQUIRES nvs_flash
│   │   ├── nvs_store.h
│   │   └── nvs_store.c
│   ├── session/                # Reusable: in-memory login tokens (esp_random, 4 slots)
│   │   ├── CMakeLists.txt       # REQUIRES esp_system
│   │   ├── session.h
│   │   └── session.c
│   └── wifi_manager/           # Reusable: AP+STA, scan, connect, event handling
│       ├── CMakeLists.txt       # REQUIRES nvs_store esp_wifi esp_event esp_netif esp_timer
│       ├── wifi_manager.h
│       └── wifi_manager.c
├── main/
│   ├── CMakeLists.txt           # REQUIRES nvs_store session wifi_manager esp_http_server esp_wifi
│   ├── app_main.c               # Boot: NVS → session → Wi-Fi → HTTP server
│   ├── web_server.c/h           # 11 HTTP routes, auth middleware, static serving
│   └── static/
│       ├── login.html           # Login page (Thai UI, form → /api/login)
│       ├── dashboard.html       # Example dashboard with 4 status cards
│       ├── status.html          # Full status page
│       ├── wifi.html            # Wi-Fi scan/connect page
│       ├── style.css            # Plain CSS design system (~500 lines, no CDN)
│       └── app.js               # Vanilla JS API calls, session, UI logic
└── README.md
```

### Component dependency graph

```
main (app_main + web_server + static)
├── app_config
├── nvs_store ────── app_config, nvs_flash
├── session ──────── app_config, esp_system
├── wifi_manager ─── app_config, nvs_store, esp_wifi, esp_event, esp_netif, esp_timer
├── esp_http_server
└── esp_wifi
```

### Component layout principles

- `components/` holds reusable, low-level modules with clean boundaries
- `main/` holds project-specific code: app entrypoint, web server, and frontend assets
- `web_server` stays in `main/` because it references EMBED_FILES symbols (generated in main)
- Each component declares its ESP-IDF deps via `REQUIRES`

## Critical Gotchas

### EMBED_FILES symbol naming

`EMBED_FILES` in `main/CMakeLists.txt` generates assembly symbols. **The directory prefix is stripped** — use the filename only:

```c
// CORRECT (file: main/static/login.html)
extern const uint8_t _binary_login_html_start[] asm("_binary_login_html_start");
extern const uint8_t _binary_login_html_end[]   asm("_binary_login_html_end");

// WRONG — will cause linker errors
extern const uint8_t _binary_static_login_html_start[] ...
```

To verify correct symbol names after build:
```powershell
Get-Content build\login.html.S -First 10
```

### Default credentials

- Username: `admin`, Password: `change-me`
- WARNING: Must be changed before any production use
- Stored in `components/app_config/app_config.h` as `APP_TEMPLATE_DEFAULT_USERNAME` / `APP_TEMPLATE_DEFAULT_PASSWORD`

### Session cookies

Session cookie is **non-HttpOnly** by design — JavaScript needs to read `document.cookie.session` for the login page redirect logic. This is a local-prototype decision. Server-side auth middleware protects all `/api/*` (except `/api/login`) and `/dashboard`, `/wifi` routes.

### Wi-Fi behavior

- Boots in APSTA mode: AP (`FishPump-Setup`, open, 192.168.4.1) + optional STA
- STA credentials auto-loaded from NVS on boot
- If saved STA fails (wrong password etc.), reconnection retries automatically
- AP stays active always as fallback
- DHCP server on AP interface: clients get 192.168.4.x

### No external dependencies

Frontend uses plain CSS/HTML/JS — **no CDN links** (no Tailwind, no Google Fonts, no Material Symbols). ESP32 in AP mode has no internet. Icons use Unicode characters. Fonts use system stack.

## API Routes

| Method | Path | Auth | Notes |
|--------|------|------|-------|
| GET | `/` | No | Redirect to /dashboard or /login |
| GET | `/login` | No | Login page |
| POST | `/api/login` | No | Body: `{"username":"...","password":"..."}` |
| POST | `/api/logout` | No | Clears cookie |
| GET | `/dashboard` | Yes | Dashboard page |
| GET | `/status` | Yes | Status page |
| GET | `/wifi` | Yes | Wi-Fi config page |
| GET | `/style.css` | No | Static CSS |
| GET | `/app.js` | No | Static JS |
| GET | `/api/wifi/scan` | Yes | Returns `{"ok":true,"networks":[...]}` |
| POST | `/api/wifi/connect` | Yes | Body: `{"ssid":"...","password":"..."}` |
| POST | `/api/wifi/disconnect` | Yes | Disconnect STA |
| GET | `/api/status` | Yes | Returns device status JSON |

## NVS Keys

Namespace `wifi_cfg`: `sta_ssid` (string), `sta_pass` (string)

## Do Not Add (Phase 1)

- OTA update logic, relay/GPIO control, sensor telemetry, charts, MQTT/cloud, SPIFFS/LittleFS, WebSocket, multi-user, HTTPS, CSRF protection
- No hardware GPIO assignments without explicit pin map
- Keep `sdkconfig.defaults` minimal — target, partition table, flash size, and required development boot defaults only

## Reference Tracking

**Rule**: If an agent learns patterns, APIs, or architecture from an external repository, **record that repo URL in `REFERENCE.md`** under the relevant section. Include a one-line note on what was adopted. This keeps the project's provenance traceable.

## Validation

```powershell
idf.py build
```

If build succeeds and `fish_pump_relay_timer_control.bin` is generated, the project is valid. No unit tests in this phase. Manual testing:
1. Connect to Wi-Fi `FishPump-Setup`
2. Open `http://192.168.4.1`
3. Login admin/change-me
4. Verify dashboard loads, Wi-Fi scan works

## Local Agent Notes

Machine-specific agent workflows, local knowledge bases, private paths, and
custom skill requirements belong in `AGENTS.local.md`. That file is ignored by
git so this template stays usable on other machines.
