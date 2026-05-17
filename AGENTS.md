# AGENTS.md

## Project

ESP32 Control System — Phase 1 Foundation. ESP-IDF firmware with local web server, login/session, Wi-Fi config, SoftAP fallback.

- **Target**: ESP32 DevKit V1 (classic ESP32), ESP-IDF framework only (not Arduino/PlatformIO)
- **Scope**: Phase 1 only — no OTA, relay, sensors, MQTT, cloud, or multi-user yet

## Environment

ESP-IDF installed at `C:\esp-idf\`. Must export before every `idf.py` command:

```powershell
& "C:\esp-idf\export.ps1"
```

**Unicode workaround**: `idf.py` warns about Unicode charset on Windows. Run this first in each PowerShell session:

```powershell
chcp 65001 > $null
```

If Python dependencies are missing, run:

```powershell
& "C:\Espressif\python_env\idf6.1_py3.13_env\Scripts\python.exe" -m pip install -r "C:\esp-idf\tools\requirements\requirements.core.txt" -c "C:\Espressif\espidf.constraints.v6.1.txt"
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

Run sequentially: `export → chcp → idf.py`. Build output is in `build/` (gitignored).

## Project Structure

```
main_dashboard_mcu/
├── CMakeLists.txt              # Root ESP-IDF build, EXTRA_COMPONENT_DIRS → components/
├── sdkconfig.defaults           # CONFIG_IDF_TARGET="esp32", CONFIG_PARTITION_TABLE_SINGLE_APP=y
├── components/
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
│       ├── dashboard.html       # Dashboard with 4 status cards
│       ├── wifi.html            # Wi-Fi scan/connect page
│       ├── style.css            # Plain CSS design system (~500 lines, no CDN)
│       └── app.js               # Vanilla JS API calls, session, UI logic
└── README.md
```

### Component dependency graph

```
main (app_main + web_server + static)
├── nvs_store ────── nvs_flash
├── session ──────── esp_system
├── wifi_manager ─── nvs_store, esp_wifi, esp_event, esp_netif, esp_timer
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

- Username: `admin`, Password: `admin123`
- WARNING: Must be changed before any production use
- Stored in `web_server.c` as `DEFAULT_USERNAME` / `DEFAULT_PASSWORD`

### Session cookies

Session cookie is **non-HttpOnly** by design — JavaScript needs to read `document.cookie.session` for the login page redirect logic. This is a local-prototype decision. Server-side auth middleware protects all `/api/*` (except `/api/login`) and `/dashboard`, `/wifi` routes.

### Wi-Fi behavior

- Boots in APSTA mode: AP (`ESP32-Control-Setup`, open, 192.168.4.1) + optional STA
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
| GET | `/wifi` | Yes | Wi-Fi config page |
| GET | `/style.css` | No | Static CSS |
| GET | `/app.js` | No | Static JS |
| GET | `/api/wifi/scan` | Yes | Returns `{"ok":true,"networks":[...]}` |
| POST | `/api/wifi/connect` | Yes | Body: `{"ssid":"...","password":"..."}` |
| GET | `/api/status` | Yes | Returns device status JSON |

## NVS Keys

Namespace `wifi_cfg`: `sta_ssid` (string), `sta_pass` (string)

## Do Not Add (Phase 1)

- OTA update logic, relay/GPIO control, sensor telemetry, charts, MQTT/cloud, SPIFFS/LittleFS, WebSocket, multi-user, HTTPS, CSRF protection
- No hardware GPIO assignments without explicit pin map
- Keep `sdkconfig.defaults` minimal — only `CONFIG_IDF_TARGET` and `CONFIG_PARTITION_TABLE_SINGLE_APP`

## Reference Tracking

**Rule**: If an agent learns patterns, APIs, or architecture from an external repository, **record that repo URL in `REFERENCE.md`** under the relevant section. Include a one-line note on what was adopted. This keeps the project's provenance traceable.

## Validation

```powershell
idf.py build
```

If build succeeds and `main_dashboard_mcu.bin` is generated, the project is valid. No unit tests in this phase. Manual testing:
1. Connect to Wi-Fi `ESP32-Control-Setup`
2. Open `http://192.168.4.1`
3. Login admin/admin123
4. Verify dashboard loads, Wi-Fi scan works

## Knowledge Base (MCU / ESP32)

Agent **MUST** use the LLM Wiki to find required MCU and ESP32 information before every task.

### LLM Wiki

- Wiki is at `~/llm-wiki/` — use `index.md` to navigate and find relevant pages
- Relevant pages for this project:
  - `wiki/esp-idf.md` — ESP-IDF framework
  - `wiki/esp32-wifi-patterns.md` — Wi-Fi AP+STA, scan/connect
  - `wiki/esp32-http-server.md` — HTTP server, auth middleware, embedded files
  - `wiki/esp32-nvs-storage.md` — NVS key-value persistence
  - `wiki/esp32-session-auth.md` — In-memory login session
  - `wiki/esp32-component-layout.md` — Project structure, CMakeLists patterns
  - `wiki/main-dashboard-mcu.md` — Project overview

### Usage Rules

Agent MUST call `/wiki query <topic>` in the following cases:

- **Before implementing or modifying any component** — search for best practices, architecture patterns, and gotchas
- **When encountering an unfamiliar ESP-IDF API or pattern** — search the wiki before writing code
- **When learning a new pattern from an external repo** — use `/wiki ingest <source>` to record the knowledge
- **When unsure about ESP32 or MCU behavior** — use `/wiki query <question>` to ask

### Supported Commands

| Command | Description |
|---------|-------------|
| `/wiki query <question>` | Search the wiki for an answer |
| `/wiki ingest <source>` | Record new knowledge into the wiki |
| `/wiki lint` | Check wiki health and consistency |

## Agent Skills

These skills are installed on the system. Agent **should** use the `skill` tool when a task matches the skill's scope.

| Skill | When to use |
|-------|-------------|
| `esp32-firmware-engineer` | Implement/edit firmware, review embedded code, fix boot/runtime failures, optimize RAM/flash, low-power design, integrate LVGL, or work directly with ESP-IDF |
| `esp32-debugging` | Compilation error, runtime panic / Guru Meditation, memory crash, stack overflow, I2C/SPI/UART failure, or debug serial output |
| `embedded-systems` | General microcontroller work, FreeRTOS, bare-metal, peripheral config, interrupt handlers, DMA, real-time systems |
| `esp32-serial-commands` | Need to send serial port commands to emulate button press / user action for testing |
| `llm-wiki` | Search MCU/ESP32 knowledge from the local wiki (see Knowledge Base section above) |
| `agent-lessons` | When user says "save lessons", "update lessons", "what we learned" |
| `find-skills` | When user asks about capabilities that may have a matching skill |
| `general-plan-implement-workflow` | Complex tasks that require systematic planning before implementation |

**Rule**: Before implementing or debugging ESP32/MCU/firmware work, check if the task matches a skill in the table. If it does, load the skill with the `skill` tool before starting.

### Mandatory `esp32-firmware-engineer` Rule

Agent **MUST** load the `esp32-firmware-engineer` skill using the `skill` tool before starting work in the following cases:

- **Firmware implementation**: writing or editing C/C++ code in `main/` or `components/`
- **Build/config**: modifying `CMakeLists.txt`, `sdkconfig`, or `sdkconfig.defaults`
- **Debugging**: investigating boot failure, runtime panic, Guru Meditation, or serial logs
- **Peripherals / RTOS**: FreeRTOS tasks/queues, GPIO, I2C, SPI, UART, ADC, PWM, Wi-Fi, BLE
- **Optimization**: RAM/flash usage, power consumption, boot time
- **Hardware bring-up**: new board, pin mapping, peripheral integration
- **Code review**: reviewing embedded firmware code written by others

**Exceptions**: Not required for frontend-only work (HTML/CSS/JS in `main/static/`) or documentation-only changes unrelated to firmware logic.

**Violating this rule may lead to lower code quality or introduce subtle bugs.**
