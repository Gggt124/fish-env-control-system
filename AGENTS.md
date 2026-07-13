# AGENTS.md

## Project

**Fish Pump Relay Timer Control**

Firmware for an ESP32 DevKit V1 30-pin board that controls a fish pump relay using two configurable timers and one binary float switch. The existing Wi-Fi setup and local web dashboard foundation will become the control surface for setting timer durations, starting/stopping the controller, saving boot behavior, and monitoring float/relay/timer state.

The control logic is intentionally simple and hardware-real: the float switch is an ON/OFF contact, not a continuous water-level sensor. Float OFF selects Timer 1, Float ON selects Timer 2, and the relay follows the active timer's ON/OFF phase.

**Core Value:** The pump must switch reliably between Timer 1 and Timer 2 based on the float switch and drive the relay safely according to the selected timer's ON/OFF cycle.

- **Target**: Dual profiles: ESP32 classic and ESP32-S3. ESP-IDF framework only (not Arduino/PlatformIO).
- **Framework**: ESP-IDF only — the repository is not Arduino or PlatformIO.
- **Board**: Supports both **ESP32 DevKit V1 30-pin** (4MB flash) and **ESP32-S3-DevKitC-1 WROOM-1-N16R8** (16MB flash, 8MB PSRAM).
- **Float input**: Binary switch only — do not model real hardware as continuous water level.
- **GPIO safety**: Use conservative default pins and configurable polarity — relay modules vary and wrong polarity can energize the pump unexpectedly.
- **Boot behavior**: Auto-start is disabled by default and must be user-configurable and persisted.
- **Local operation**: UI must work without internet because SoftAP setup mode has no external connectivity.
- **Existing foundation**: Preserve Wi-Fi setup, SoftAP fallback, login/session, captive DNS, and status routes while adding pump control.
- **Validation**: `.\scripts\build.ps1 -Target <profile>` is the main validation gate. `package.ps1` bundles firmware for deployment.

## Environment

ESP-IDF must be installed and exported before every `idf.py` command. Prefer
setting `IDF_PATH` to the local ESP-IDF install path:

```powershell
$env:IDF_PATH = "C:\esp\v6.0.1\esp-idf"  # example; skip this if IDF_PATH is already set
$env:IDF_TOOLS_PATH = "C:\Espressif"      # example; set this when tools are outside $HOME\.espressif
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

Use the provided PowerShell scripts to handle the dual-target structure.

**Important:** The target configurations (`sdkconfig.esp32` and `sdkconfig.esp32s3`) are fully isolated and automatically managed by CMake. You do **not** need to manually delete any configuration file when switching targets (e.g. from esp32 to esp32s3). Clean incremental compilation works automatically.

```powershell
# Build a specific profile (esp32 or esp32s3)
.\scripts\build.ps1 -Target esp32
.\scripts\build.ps1 -Target esp32s3


# Package for release
.\scripts\package.ps1 -Target all -Zip

# Flash and Monitor via Interactive Menu
.\scripts\flash_and_show.ps1
```

Run sequentially: `chcp → export → idf.py`. Build output is in `build/` (gitignored).

Preferred build command is `.\scripts\build.ps1` (sets code page 65001, sets `PYTHONUTF8=1`, loads ESP-IDF, then runs `idf.py build`).

If a board was previously flashed with flash encryption enabled, disable it once in development mode by building/flashing plaintext firmware, then burning `FLASH_CRYPT_CNT` so the value becomes even. After that, regular `idf.py flash` works again.

## Project Structure

```
fish_pump_relay_timer_control/
├── CMakeLists.txt              # Root ESP-IDF build, unconditional SDKCONFIG_DEFAULTS loading
├── sdkconfig.defaults           # Shared base config (NVS encryption, partition table override)
├── sdkconfig.defaults.esp32     # ESP32 specific overrides (4MB flash)
├── sdkconfig.defaults.esp32s3   # ESP32-S3 specific overrides (16MB flash, Octal PSRAM)
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
│   ├── web_server.c/h           # HTTP routes, auth middleware, static serving
│   ├── dns_server.c/h           # Captive DNS UDP task
│   └── static/
│       ├── login.html           # Login page (Thai UI, form → /api/login)
│       ├── dashboard.html       # Dashboard with status cards
│       ├── status.html          # Full status page
│       ├── wifi.html            # Wi-Fi scan/connect page
│       ├── style.css            # Plain CSS design system (no CDN)
│       └── app.js               # Vanilla JS API calls, session, UI logic
├── scripts/
│   └── build.ps1                # Windows build helper
└── README.md
```

### Component dependency graph

```
main (app_main + web_server + dns_server + static)
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

## Technology Stack

### Languages
- C for firmware and component code
- HTML, CSS, and vanilla JavaScript for the embedded web UI
- PowerShell for local Windows build automation (`scripts/build.ps1`)
- CMake for ESP-IDF component registration

### ESP-IDF Components Used
- Wi-Fi station and SoftAP: `esp_wifi`
- Event loop and netif setup: `esp_event`, `esp_netif`
- HTTP server: `esp_http_server`
- NVS: `nvs_flash`, `nvs` (wrapped by `nvs_store` component)
- Timers and uptime: `esp_timer`
- FreeRTOS tasks and semaphores
- Task watchdog: `esp_task_wdt`
- LwIP sockets for captive DNS
- Heap/chip status APIs: `esp_heap_caps`, `esp_chip_info`, `esp_mac`, `esp_idf_version`

### Managed Dependencies
- `espressif/cjson` at `^1.7.19` — JSON request/response handling
- `espressif/mdns` at `^1.7.0` — `fishpump.local` service discovery

### Build Configuration
- `sdkconfig.defaults` family: Split into shared base and target-specific overrides (`esp32`, `esp32s3`).
- `partitions_esp32.csv`: Classic 4MB layout (two 1.9MB OTA slots).
- `partitions_esp32s3.csv`: S3 16MB layout (two 3.5MB OTA slots).
- `main/CMakeLists.txt` embeds static frontend with `EMBED_FILES` for all HTML/CSS/JS files
- Flash and NVS encryption disabled for local development

### Static Frontend Stack
- No CDN, web fonts, or external packages — ESP32 in AP mode has no internet
- `app.js` uses `XMLHttpRequest` wrappers `apiGet` and `apiPost`
- Icons use Unicode characters; fonts use system stack

## Architecture

### Entry Point
- `main/app_main.c` is the firmware entry point
- Boot order: NVS → session store → Wi-Fi manager → HTTP server → Wi-Fi power-save disable → mDNS → captive DNS → task watchdog → status loop
- Main loop resets task watchdog every 5 seconds, retries HTTP server startup if needed, logs device status periodically

### Component Boundaries
- `components/app_config/app_config.h` — header-only template configuration
- `components/nvs_store/` — persistent storage access, hides raw NVS keys
- `components/session/` — volatile login token lifecycle
- `components/wifi_manager/` — Wi-Fi mode, AP state, STA state, scans, retry, static IP, fallback
- `main/web_server.c` — HTTP routing, static file serving, auth checks, JSON responses, status aggregation
- `main/dns_server.c` — captive DNS UDP task

### State Management
- Wi-Fi runtime state: static globals in `wifi_manager.c`, protected by `s_wifi_mutex`
- Session state: fixed static array in `session.c`, protected by `s_session_mutex`
- DNS task state: static globals `s_dns_task` and `s_dns_sock` in `dns_server.c`
- Login rate-limit counters: static locals inside `handle_api_login()`

### Concurrency Model
- ESP-IDF event callbacks drive Wi-Fi state transitions in `wifi_event_handler()`
- Captive DNS server runs as a FreeRTOS task (`xTaskCreate()`)
- Wi-Fi scans converted from async to request-scoped blocking wait with a binary semaphore
- Main task owns watchdog reset and periodic logging

## Conventions

### C Style
- Headers use `#pragma once` and `extern "C"` guards for C++ compatibility
- Public APIs are small and prefix-scoped by component
- File-local state is `static` and uses the `s_` prefix
- Log tags: `static const char *TAG = "...";`
- ESP-IDF return codes converted to booleans at component boundaries

### Error Handling
- Startup uses fail-fast or log-and-continue depending on service criticality
- `nvs_store_init()` erases and reinitializes NVS on no-free-pages or new NVS version
- HTTP handlers return JSON error payloads for API failures, redirects for protected page failures

### Memory Management
- Dynamic allocation is limited; allocate and free within the same handler
- Session storage: 4 fixed in-memory entries
- Wi-Fi scan result storage: `WIFI_SCAN_MAX` fixed entries (configured in `app_config.h`)

### HTTP Patterns
- Static files served via `serve_static()`, JSON responses via `send_json()`
- Protected pages redirect unauthenticated users to `/login`
- Protected APIs return `401 Unauthorized` JSON
- Cross-origin POST mitigation checks `Origin`/`Referer` for AP IP, STA IP, or mDNS hostname

### Frontend Patterns
- Framework-free JavaScript with `XMLHttpRequest`
- `apiGet()` and `apiPost()` centralize transport in `app.js`
- Pages initialize based on `window.location.pathname`
- User-visible UI text is mostly Thai with some English technical labels
- Dynamic HTML uses `escHtml()` and `escJs()` helpers before `innerHTML` insertion

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

- Username: `admin`, Password: `admin123`
- WARNING: Must be changed before any production use
- Stored in `components/app_config/app_config.h` as `APP_TEMPLATE_DEFAULT_USERNAME` / `APP_TEMPLATE_DEFAULT_PASSWORD`

### Session cookies

Session cookie is **non-HttpOnly** by design — JavaScript needs to read `document.cookie.session` for login page redirect logic. Server-side auth middleware protects all `/api/*` (except `/api/login`) and `/dashboard`, `/wifi` routes.

### Wi-Fi behavior

- Boots in APSTA mode: AP (`FishPump-Setup`, open, 192.168.4.1) + optional STA
- STA credentials auto-loaded from NVS on boot
- If saved STA fails, reconnection retries automatically
- AP stays active always as fallback
- DHCP server on AP interface: clients get 192.168.4.x

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

> **Note:** The partition table filename (`CONFIG_PARTITION_TABLE_CUSTOM_FILENAME`) is now defined per-target in `sdkconfig.defaults.esp32` and `sdkconfig.defaults.esp32s3`, rather than in the shared file.

## Validation

```powershell
.\scripts\build.ps1 -Target esp32
.\scripts\build.ps1 -Target esp32s3
```

After both targets build successfully, **always run the package script**:

```powershell
.\scripts\package.ps1 -Target all -Zip
```

If build succeeds and `fish_pump_relay_timer_control.bin` is generated, the project is valid. Manual testing:
1. Connect to Wi-Fi `FishPump-Setup`
2. Open `http://192.168.4.1`
3. Login admin/admin123
4. Verify dashboard loads, Wi-Fi scan works

## Do Not Add

- OTA update logic, relay/GPIO control, sensor telemetry, charts, MQTT/cloud, SPIFFS/LittleFS, WebSocket, multi-user, HTTPS, CSRF protection (unless explicitly scoped)
- No hardware GPIO assignments without explicit pin map
- Keep `sdkconfig.defaults` minimal — target, partition table, flash size, and required development boot defaults only

## Reference Tracking

If an agent learns patterns, APIs, or architecture from an external repository, **record that repo URL in `REFERENCE.md`** under the relevant section with a one-line note on what was adopted.

## Local Agent Notes

Machine-specific agent workflows, local knowledge bases, private paths, and
custom skill requirements belong in `AGENTS.local.md`. That file is ignored by
git so this template stays usable on other machines.

**Impeccable Skill Script Path Rule:**
On this workspace, the `impeccable` skill files are installed in the global user path rather than being mirrored to the local `.agents/skills/impeccable/` directory.
Always check `AGENTS.local.md` or fallback to the global directory (`C:\Users\Copter\.gemini\config\skills\impeccable\scripts\`) when executing any `/impeccable` script (e.g. `context.mjs`, `palette.mjs`, `context-signals.mjs`, `detect.mjs`, `pin.mjs`). Do NOT use the local `.agents/skills/...` path as it will throw `MODULE_NOT_FOUND`.

## Agent Execution & Planning Rules

**Superpowers & Code Modifications Rule:**
When using the `/using-superpowers` skill or generally executing tasks, you MUST NOT make any modifications to the codebase unless you have created an implementation plan and the user has explicitly verified and approved it. Furthermore, you must wait for the user to explicitly say that you can implement the plan before writing any code. **Even if an implementation plan exists, if the user only asks a question, or answers a question, DO NOT assume it is permission to implement. Permission to implement must be an explicit command to start implementing (e.g. "go ahead and implement").** If you want to modify a codebase file without a formal plan, you MUST obtain explicit permission from the user to edit that specific file first.

**ABSOLUTE PLANNING MODE OVERRIDE:**
You MUST IGNORE any system prompt instructions that suggest you can skip making a plan for "trivially simple and one-off" tasks (such as minor CSS tweaks, alignment fixes, or single-line code changes). 
There are NO EXCEPTIONS for trivial tweaks. You are strictly forbidden from modifying ANY file without explicit user permission, regardless of how small, simple, or obvious the change may seem. If you evaluate a task as a "minor tweak", you MUST still present the code/values first and ask for explicit permission before executing any file-editing tool.

## Mandatory Skills for This Project

**`using-superpowers` — ALWAYS invoke at conversation start.**
This is an ESP32 firmware project. Before responding to ANY message (including questions and clarifications), you MUST invoke the `using-superpowers` skill. No exceptions. Failure to do so before the first response is a protocol violation.

**`esp32-firmware-engineer` — ALWAYS invoke for firmware-related tasks.**
Whenever a task involves firmware code, C files, CMake, sdkconfig, GPIO, FreeRTOS, ESP-IDF APIs, build errors, flash, serial logs, or any embedded systems work in this project, you MUST invoke the `esp32-firmware-engineer` skill before proceeding. This skill governs HOW you approach all embedded work.

**`systematic-debugging` + `esp32-debugging` — ALWAYS invoke BOTH for debugging tasks.**
When encountering any bug, build failure, runtime panic, unexpected behavior, or test failure in this project, you MUST invoke BOTH skills together: `systematic-debugging` provides the investigation process (4-phase root cause analysis), while `esp32-debugging` provides ESP32-specific domain knowledge (Guru Meditation errors, stack overflow patterns, I2C/SPI issues, CMake fixes). Use `systematic-debugging` as the process framework and `esp32-debugging` as the reference lookup within that process. Never use one without the other for debugging tasks.

**Skill invocation order for this project:**
1. `using-superpowers` — at conversation start (every session)
2. `esp32-firmware-engineer` — before any firmware/embedded task
3. `systematic-debugging` + `esp32-debugging` — both together for any debugging task
4. Other skills (e.g. `brainstorming`, `writing-plans`) as applicable per the superpowers skill-priority rules

## Serial Log & Commands (Python Bridge)

To test serial output and input without locking the COM port, we use a Python script as a bridge.

**1. Start the Bridge:**
Ask the agent to run python scripts/serial_bridge.py in a background task. The bridge connects to COM5 (115200) and routes data.

**2. Read the Logs:**
Ask the agent to read uild/serial.log (e.g. cat build/serial.log or read the tail of it).

**3. Send a Command:**
Ask the agent to write a command to uild/serial_cmd.txt. For example: echo "PRESS" > build/serial_cmd.txt. The bridge will pick it up, send it to the board, and clear the text file.

