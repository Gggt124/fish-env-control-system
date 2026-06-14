# Codebase Structure

**Analysis Date:** 2026-06-14

## Directory Layout

```
fish-pump-RelayTimerControl/
├── CMakeLists.txt              # Root build configuration
├── partitions.csv              # Flash memory partition table
├── sdkconfig.defaults          # Default build configuration parameters
├── components/                 # Custom ESP-IDF components
│   ├── app_config/             # Template constants and limits
│   ├── cooling_control/        # Temperature reader and cooling relay controller
│   ├── hardware_map/           # Pin assignment and validation rules
│   ├── nvs_store/              # NVS flash serialization layer
│   ├── pump_control/           # Relay timers and float state machine
│   ├── session/                # In-memory authentication token tracker
│   └── wifi_manager/           # Wi-Fi mode management and scan controls
├── main/                       # Application entry point and web server
│   ├── CMakeLists.txt          # Asset embeds and source compilation list
│   ├── app_main.c              # Boot coordinator and diagnostics loop
│   ├── dns_server.c/h          # Captive portal DNS server
│   ├── tft_display.c/h         # ST7789/ILI9341 LCD rendering task
│   ├── web_server.c/h          # HTTP routing, cookie auth, and API handlers
│   └── static/                 # Static web files embedded into firmware flash
│       ├── login.html          # HTML Thai login screen
│       ├── dashboard.html      # HTML dashboard cards
│       ├── wifi.html           # HTML Wi-Fi connection utility
│       ├── status.html         # HTML debug logs and hardware status
│       ├── style.css           # Vanilla CSS styles and color tokens
│       └── app.js              # AJAX requests, session validation, and DOM updates
├── scripts/                    # Development scripts
│   └── build.ps1               # Windows PowerShell build runner
└── tests/                      # Python unit tests
    ├── test_ui_phase13.py      # Static accessibility and CSS validation tests
    └── test_ui_phase16.py      # Empty state and fade transitions validation tests
```

## Directory Purposes

**components/app_config:**
- Purpose: Core configuration presets.
- Contains: `app_config.h` (header-only macros).
- Key files: [app_config.h](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/components/app_config/app_config.h).

**components/cooling_control:**
- Purpose: Bang-bang DS18B20 cooling logic.
- Contains: `cooling_control.c` and `cooling_control.h`.
- Key files: [cooling_control.h](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/components/cooling_control/cooling_control.h).

**components/hardware_map:**
- Purpose: Run-time GPIO assignment mapping.
- Contains: `hardware_map.c` and `hardware_map.h`.
- Key files: [hardware_map.h](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/components/hardware_map/hardware_map.h).

**components/nvs_store:**
- Purpose: Storing credentials and profile variables in flash.
- Contains: `nvs_store.c` and `nvs_store.h`.
- Key files: [nvs_store.h](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/components/nvs_store/nvs_store.h).

**components/pump_control:**
- Purpose: Float switch state changes and pump relay cycles.
- Contains: `pump_control.c` and `pump_control.h`.
- Key files: [pump_control.h](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/components/pump_control/pump_control.h).

**components/session:**
- Purpose: Web session lifecycle manager.
- Contains: `session.c` and `session.h`.
- Key files: [session.h](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/components/session/session.h).

**components/wifi_manager:**
- Purpose: SoftAP + STA mode event handler and scanner.
- Contains: `wifi_manager.c` and `wifi_manager.h`.
- Key files: [wifi_manager.h](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/components/wifi_manager/wifi_manager.h).

**main/static:**
- Purpose: Embedded web dashboard.
- Contains: `login.html`, `dashboard.html`, `wifi.html`, `status.html`, `style.css`, `app.js`.
- Key files: [app.js](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/main/static/app.js), [style.css](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/main/static/style.css).

## Key File Locations

**Entry Points:**
- `main/app_main.c` - C application bootstrapper.
- `main/static/login.html` - Web entry point (if unauthenticated).

**Configuration:**
- `components/app_config/app_config.h` - Code constants.
- `sdkconfig.defaults` - IDF target configs.
- `partitions.csv` - App flash sizing.

**Core Logic:**
- `components/pump_control/pump_control.c` - Pump relay state controller.
- `components/cooling_control/cooling_control.c` - Cooling relay state controller.
- `main/web_server.c` - API route dispatcher.

**Testing:**
- `tests/test_ui_phase13.py` - CSS variables and touch targets checklist.
- `tests/test_ui_phase16.py` - Fade animations and empty card checks.

## Naming Conventions

**Files:**
- `*.c` and `*.h` - lowercase snake_case for system sources (e.g., `web_server.c`).
- `*.html` - lowercase single-word name (e.g., `wifi.html`).
- `*.py` - lowercase snake_case with phase suffix (e.g., `test_ui_phase13.py`).

**Directories:**
- `components/[name]` - lowercase snake_case for IDF component modules.
- `main/static` - lowercase singular folders for compiled web files.

## Where to Add New Code

**New Component/Hardware Interface:**
- Implementation: Create a folder `components/your_component/` containing `CMakeLists.txt`, `your_component.h`, `your_component.c`.
- Registration: Declare dependencies inside `CMakeLists.txt` using `REQUIRES`.
- Usage: Add `#include "your_component.h"` inside [app_main.c](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/main/app_main.c).

**New REST API endpoint:**
- Implementation: Write handler function in [web_server.c](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/main/web_server.c).
- Registration: Append handler info to the `s_routes` array near the bottom of [web_server.c](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/main/web_server.c#L3247).

**New Frontend Page / Logic:**
- Markup: Create `[page].html` inside `main/static/`.
- Styling: Add CSS declarations to `style.css`.
- Logic: Bind event handlers inside `app.js`.
- Registration: Embed the file in `main/CMakeLists.txt` and map the URL path in `main/web_server.c`.

## Special Directories

**build/**
- Purpose: Compiled binaries, elf symbols, map logs, and assembly output.
- Source: Auto-generated by CMake/Ninja during `idf.py build`.
- Committed: No (in `.gitignore`).

---

*Structure analysis: 2026-06-14*
*Update when directory structure changes*
