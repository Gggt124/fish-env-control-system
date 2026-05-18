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

<!-- GSD:project-start source:PROJECT.md -->
## Project

**Fish Pump Relay Timer Control**

Firmware for an ESP32 DevKit V1 30-pin board that controls a fish pump relay using two configurable timers and one binary float switch. The existing Wi-Fi setup and local web dashboard foundation will become the control surface for setting timer durations, starting/stopping the controller, saving boot behavior, and monitoring float/relay/timer state.

The control logic is intentionally simple and hardware-real: the float switch is an ON/OFF contact, not a continuous water-level sensor. Float OFF selects Timer 1, Float ON selects Timer 2, and the relay follows the active timer's ON/OFF phase.

**Core Value:** The pump must switch reliably between Timer 1 and Timer 2 based on the float switch and drive the relay safely according to the selected timer's ON/OFF cycle.

### Constraints

- **Framework**: ESP-IDF only — the repository is not Arduino or PlatformIO.
- **Board**: ESP32 DevKit V1 30-pin / classic ESP32 — pin recommendations and build target assume this board.
- **Float input**: Binary switch only — do not model real hardware as continuous water level.
- **GPIO safety**: Use conservative default pins and configurable polarity — relay modules vary and wrong polarity can energize the pump unexpectedly.
- **Boot behavior**: Auto-start is enabled by default but must be user-configurable and persisted.
- **Local operation**: UI must work without internet because SoftAP setup mode has no external connectivity.
- **Existing foundation**: Preserve Wi-Fi setup, SoftAP fallback, login/session, captive DNS, and status routes while adding pump control.
- **Validation**: `idf.py build` remains the main automated validation gate; hardware behavior needs manual flash/device testing.
<!-- GSD:project-end -->

<!-- GSD:stack-start source:codebase/STACK.md -->
## Technology Stack

## Summary
## Languages
- C for firmware and component code: `main/app_main.c`, `main/web_server.c`, `main/dns_server.c`, `components/*/*.c`.
- Header-only C configuration in `components/app_config/app_config.h`.
- HTML, CSS, and vanilla JavaScript for the embedded web UI: `main/static/*.html`, `main/static/style.css`, `main/static/app.js`.
- PowerShell for local Windows build automation: `scripts/build.ps1`.
- CMake for ESP-IDF component registration: `CMakeLists.txt`, `main/CMakeLists.txt`, `components/*/CMakeLists.txt`.
## Runtime And Framework
- Target platform: classic ESP32 / ESP32 DevKit V1.
- Framework: ESP-IDF only; no Arduino or PlatformIO.
- Root build includes ESP-IDF project support via `include($ENV{IDF_PATH}/tools/cmake/project.cmake)` in `CMakeLists.txt`.
- Extra local components are enabled through `EXTRA_COMPONENT_DIRS "${CMAKE_SOURCE_DIR}/components"` in `CMakeLists.txt`.
- Target is pinned with `CONFIG_IDF_TARGET="esp32"` in `sdkconfig.defaults`.
## ESP-IDF Components Used
- Wi-Fi station and SoftAP: `esp_wifi`, used by `components/wifi_manager/wifi_manager.c` and `main/web_server.c`.
- Event loop and netif setup: `esp_event`, `esp_netif`, used by `components/wifi_manager/wifi_manager.c`.
- HTTP server: `esp_http_server`, used by `main/web_server.c`.
- NVS: `nvs_flash`, `nvs`, wrapped by `components/nvs_store/nvs_store.c`.
- Timers and uptime: `esp_timer`, used by `components/session/session.c`, `components/wifi_manager/wifi_manager.c`, and `main/web_server.c`.
- FreeRTOS tasks and semaphores: used by `components/session/session.c`, `components/wifi_manager/wifi_manager.c`, `main/dns_server.c`, and `main/app_main.c`.
- Task watchdog: `esp_task_wdt`, initialized manually in `main/app_main.c`.
- LwIP sockets and IPv4 parsing: `main/dns_server.c` and `components/wifi_manager/wifi_manager.c`.
- Heap and chip status APIs: `esp_heap_caps`, `esp_chip_info`, `esp_mac`, `esp_idf_version`, used in `main/web_server.c`.
## Managed Dependencies
- `espressif/cjson` at `^1.7.19`, used for JSON request and response handling in `main/web_server.c`.
- `espressif/mdns` at `^1.7.0`, used for `fish-pump.local` service discovery from `main/app_main.c`.
## Build Configuration
- `sdkconfig.defaults` sets a custom partition table, development flash workflow, 4 MB flash, and manual task watchdog initialization.
- Flash and NVS encryption are disabled in `sdkconfig.defaults` for local development.
- `partitions.csv` defines `nvs`, `phy_init`, and one `factory` app partition sized at 1 MB.
- `main/CMakeLists.txt` embeds the static frontend with `EMBED_FILES` for `login.html`, `dashboard.html`, `status.html`, `wifi.html`, `style.css`, and `app.js`.
- Embedded file symbols are filename-based in `main/web_server.c`, for example `_binary_login_html_start`.
## Local Build Workflow
- Preferred build command is `.\scripts\build.ps1`.
- `scripts/build.ps1` sets code page 65001, sets `PYTHONUTF8=1`, loads ESP-IDF through `export.ps1`, then runs `idf.py --version` and `idf.py build`.
- Manual workflow is documented in `README.md` and `AGENTS.md`: run `chcp 65001`, export ESP-IDF, then run `idf.py build`.
## Static Frontend Stack
- No CDN, web fonts, or external frontend packages.
- `main/static/app.js` uses `XMLHttpRequest` wrappers `apiGet` and `apiPost`.
- UI pages are server-rendered static files and call JSON APIs after load.
- CSS design system is plain CSS in `main/static/style.css`.
## Configuration Surface
- Product names, AP SSID, mDNS host, credentials, limits, AP auto-stop settings, HTTP handler capacity, and watchdog timeout live in `components/app_config/app_config.h`.
- Wi-Fi and optional static IP runtime settings persist in NVS through `components/nvs_store/nvs_store.c`.
<!-- GSD:stack-end -->

<!-- GSD:conventions-start source:CONVENTIONS.md -->
## Conventions

## C Style
- Code uses C with ESP-IDF APIs and explicit component boundaries.
- Headers use `#pragma once` and `extern "C"` guards for C++ compatibility.
- Public APIs are small and prefix-scoped by component.
- File-local state is `static` and often uses the `s_` prefix.
- Log tags are `static const char *TAG = "...";`.
- ESP-IDF return codes are converted to booleans at component boundaries where the public API is simple.
## Component API Pattern
- Components expose one header and one implementation file, except `app_config`, which is header-only.
- Component dependencies are declared in each component `CMakeLists.txt`.
- Higher-level code should call wrapper APIs rather than raw storage or Wi-Fi internals. Example: `main/web_server.c` calls `wifi_manager_connect_sta()` instead of configuring ESP Wi-Fi directly for STA connect.
## Error Handling
- Startup uses fail-fast or log-and-continue depending on service criticality in `main/app_main.c`.
- `nvs_store_init()` erases and reinitializes NVS when ESP-IDF reports no free pages or a new NVS version.
- Most NVS wrappers return `false` on open/set/get failure and reset output buffers when a load misses.
- `wifi_manager_init()` uses `ESP_ERROR_CHECK()` for core ESP-IDF initialization steps.
- HTTP handlers return JSON error payloads for API failures and redirects for protected page failures.
## Memory Management
- Dynamic allocation is limited.
- `main/web_server.c` allocates cookie buffers and cJSON strings, then frees them in the same handler.
- `components/wifi_manager/wifi_manager.c` allocates scan results with `calloc()` during `WIFI_EVENT_SCAN_DONE` and frees them after copying into fixed-size scan result storage.
- Session storage is fixed-size: 4 in-memory entries in `components/session/session.c`.
- Wi-Fi scan result storage is fixed-size: `WIFI_SCAN_MAX` entries from `components/app_config/app_config.h`.
## Concurrency
- Shared session state is guarded by `s_session_mutex` in `components/session/session.c`.
- Wi-Fi state is guarded by `s_wifi_mutex` in `components/wifi_manager/wifi_manager.c`.
- Request-scoped Wi-Fi scan waiting uses a FreeRTOS binary semaphore in `main/web_server.c`.
- Captive DNS runs in its own FreeRTOS task in `main/dns_server.c`.
## HTTP Patterns
- Static files are served with `serve_static()` in `main/web_server.c`.
- JSON responses generally go through `send_json()`.
- Protected pages redirect unauthenticated users to `/login`.
- Protected APIs return `401 Unauthorized` JSON.
- Cross-origin POST mitigation checks `Origin` or `Referer` for AP IP, STA IP, or mDNS hostname.
## Frontend Patterns
- JavaScript is framework-free and uses `XMLHttpRequest`.
- `apiGet()` and `apiPost()` centralize transport behavior in `main/static/app.js`.
- Pages initialize based on `window.location.pathname`.
- User-visible UI text is mostly Thai with some English technical labels.
- Dynamic network list HTML uses `escHtml()` and `escJs()` helpers before insertion into `innerHTML`.
## Build And Encoding Practices
- Windows PowerShell sessions should use `chcp 65001` before ESP-IDF commands.
- `scripts/build.ps1` sets `PYTHONUTF8=1` before exporting ESP-IDF.
- Development notes warn against PowerShell regex rewrites on Unicode-heavy JS files.
## Documentation Pattern
- `README.md` is operator-facing and includes build, flash, manual test, project structure, and security notes.
- `docs/components.md` is component-facing and concise.
- `docs/development-notes.md` records concrete lessons from prior implementation/debugging work.
- `REFERENCE.md` should record any external repositories used as architecture or API references.
<!-- GSD:conventions-end -->

<!-- GSD:architecture-start source:ARCHITECTURE.md -->
## Architecture

## Summary
## Entry Point
- `main/app_main.c` is the firmware entry point.
- Boot order is NVS, session store, Wi-Fi manager, HTTP server, Wi-Fi power-save disable, mDNS, captive DNS, task watchdog, then a status loop.
- The main loop resets the task watchdog every 5 seconds, retries HTTP server startup if needed, and logs device status periodically.
## Component Boundaries
- `components/app_config/app_config.h` is a header-only template configuration boundary.
- `components/nvs_store/` owns persistent storage access and hides raw NVS keys from the rest of the app.
- `components/session/` owns volatile login token lifecycle.
- `components/wifi_manager/` owns ESP-IDF Wi-Fi mode, AP state, STA state, scans, retry behavior, static IP application, and fallback behavior.
- `main/web_server.c` owns HTTP routing, static file serving, auth checks, request parsing, JSON responses, and status aggregation.
- `main/dns_server.c` owns the captive DNS UDP task.
## Data Flow: Boot
## Data Flow: Login
## Data Flow: Wi-Fi Connect
## Data Flow: Status
- `/api/status` is handled in `handle_api_status()` in `main/web_server.c`.
- Status combines chip metadata, IDF version, MAC addresses, heap metrics, uptime, Wi-Fi mode, AP client count, STA details, and captive DNS state.
- Frontend pages update dashboard cards and status tables by polling `/api/status`.
## State Management
- Wi-Fi runtime state is stored in static globals in `components/wifi_manager/wifi_manager.c`, protected by `s_wifi_mutex` in most state mutations.
- Session state is stored in a fixed static array in `components/session/session.c`, protected by `s_session_mutex`.
- DNS task state is stored in static globals `s_dns_task` and `s_dns_sock` in `main/dns_server.c`.
- Login rate-limit counters are static locals inside `handle_api_login()` in `main/web_server.c`.
## Concurrency Model
- ESP-IDF event callbacks drive Wi-Fi state transitions in `wifi_event_handler()`.
- The captive DNS server runs as a FreeRTOS task created by `xTaskCreate()` in `main/dns_server.c`.
- Wi-Fi scans are asynchronous at the ESP-IDF layer, then converted into a request-scoped blocking wait with a binary semaphore in `main/web_server.c`.
- The main task runs indefinitely and owns watchdog reset plus periodic logging.
## Design Boundary
<!-- GSD:architecture-end -->

<!-- GSD:skills-start source:skills/ -->
## Project Skills

No project skills found. Add skills to any of: `.claude/skills/`, `.agents/skills/`, `.cursor/skills/`, `.github/skills/`, or `.codex/skills/` with a `SKILL.md` index file.
<!-- GSD:skills-end -->

<!-- GSD:workflow-start source:GSD defaults -->
## GSD Workflow Enforcement

Before using Edit, Write, or other file-changing tools, start work through a GSD command so planning artifacts and execution context stay in sync.

Use these entry points:
- `/gsd-quick` for small fixes, doc updates, and ad-hoc tasks
- `/gsd-debug` for investigation and bug fixing
- `/gsd-execute-phase` for planned phase work

Do not make direct repo edits outside a GSD workflow unless the user explicitly asks to bypass it.
<!-- GSD:workflow-end -->

<!-- GSD:profile-start -->
## Developer Profile

> Profile not yet configured. Run `/gsd-profile-user` to generate your developer profile.
> This section is managed by `generate-claude-profile` -- do not edit manually.
<!-- GSD:profile-end -->
