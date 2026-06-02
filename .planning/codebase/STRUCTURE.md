---
generated: 2026-05-18
refreshed: 2026-06-02
last_mapped_commit: 3ef2ac063ebead27920e5e340508c371f689cdca
focus: arch
scope: .gitignore,.graphify_detect.json,.planning,PRODUCT.md
---

# Structure

## Repository Layout

- `CMakeLists.txt` is the root ESP-IDF project file and adds `components/` as extra component directory.
- `sdkconfig.defaults` holds portable default ESP-IDF configuration.
- `partitions.csv` defines the custom flash partition table.
- `dependencies.lock` records resolved ESP-IDF component manager dependencies.
- `README.md` documents build, manual test, template boundaries, and security notes.
- `AGENTS.md` documents local agent instructions and project-specific gotchas.
- `REFERENCE.md` tracks external repository references and provenance.
- `ROADMAP.md`, `DESIGN.md`, and `TEMPLATE.md` are planning/design artifacts.
- `docs/` contains project documentation.
- `scripts/` contains local automation.
- `components/` contains reusable ESP-IDF components.
- `main/` contains app-specific firmware, routes, captive DNS, and embedded frontend.

## Root Build Files

- `CMakeLists.txt` includes ESP-IDF project CMake and declares project name `fish_pump_relay_timer_control`.
- `sdkconfig.defaults` sets target, partition table, flash size, development security defaults, and task watchdog behavior.
- `partitions.csv` defines `nvs`, `phy_init`, and `factory`.

## Components

- `components/app_config/`
  - `app_config.h`: template constants for product name, firmware version, AP, mDNS, credentials, limits, AP auto-stop, HTTP handler capacity, watchdog timeout.
  - `CMakeLists.txt`: header-only component registration.
- `components/nvs_store/`
  - `nvs_store.h`: public persistence API for Wi-Fi credentials, AP config, static IP config, pump settings, active/pending hardware maps, and cooling settings.
  - `nvs_store.c`: NVS namespace/key implementation.
  - `CMakeLists.txt`: depends on `app_config`, `hardware_map`, and `nvs_flash`.
- `components/hardware_map/`
  - `hardware_map.h`: public hardware role, GPIO option, relay polarity, timer start phase, cooling mode, map, and validation API.
  - `hardware_map.c`: ESP32 DevKit V1 safe GPIO option lists, defaults, name helpers, and validation logic.
  - `CMakeLists.txt`: depends on `app_config` and `esp_driver_gpio`.
- `components/session/`
  - `session.h`: RAM session API and token length constants.
  - `session.c`: token creation, validation, expiry, and destruction.
  - `CMakeLists.txt`: depends on `app_config`, `esp_system`, `esp_timer`, and `freertos`.
- `components/wifi_manager/`
  - `wifi_manager.h`: AP/STA/scan/status API.
  - `wifi_manager.c`: Wi-Fi initialization, events, AP fallback, STA connect/disconnect, static IP, scan, status getters.
  - `CMakeLists.txt`: depends on `app_config`, `nvs_store`, `esp_wifi`, `esp_event`, `esp_netif`, `esp_timer`, and `freertos`.
- `components/pump_control/`
  - `pump_control.h`: public pump config/status types and default/init/start/stop/status API.
  - `pump_control.c`: GPIO validation, relay inactive initialization, float debounce, timer selection, ON/OFF phase transitions, and status snapshots.
  - `CMakeLists.txt`: depends on `app_config`, `esp_driver_gpio`, `esp_timer`, and `freertos`.

## Main Application

- `main/app_main.c`: boot sequence and main loop.
- `main/web_server.c`: HTTP server routes, auth middleware, cJSON API handlers, static file serving, status aggregation.
- `main/web_server.h`: `web_server_start()` declaration.
- `main/dns_server.c`: UDP DNS fallback server.
- `main/dns_server.h`: DNS lifecycle declarations.
- `main/CMakeLists.txt`: registers app sources, embeds static files, and declares dependencies.
- `main/idf_component.yml`: managed component dependencies for cJSON and mDNS.

## Static UI

- `main/static/login.html`: Thai login page.
- `main/static/dashboard.html`: authenticated pump-control dashboard with runtime panel, timer settings, Start/Stop controls, and compact system summary.
- `main/static/status.html`: full device status page.
- `main/static/wifi.html`: Wi-Fi scan/connect page with optional static IP fields.
- `main/static/style.css`: plain CSS visual system.
- `main/static/app.js`: vanilla JS API helpers, auth redirect logic, pump dashboard config/status/actions, system polling, Wi-Fi scan/connect UI, and HTML escaping utilities.

## Documentation

- `docs/components.md` summarizes component responsibilities and NVS keys.
- `docs/development-notes.md` captures ESP-IDF, Windows, frontend, and Wi-Fi manager development lessons.
- `README.md` is the primary operator/developer guide.
- `AGENTS.md` is the active agent instruction set for this workspace.

## Naming Conventions

- Public component APIs use prefixes matching their component: `nvs_store_*`, `session_*`, `wifi_manager_*`, `dns_server_*`, `web_server_*`.
- Static file symbols use `_binary_<filename>_<start|end>` because ESP-IDF strips the directory prefix for `EMBED_FILES`.
- Static C globals use `s_` prefixes, such as `s_sta_connected`, `s_sessions`, and `s_dns_task`.
- ESP-IDF logs use per-file `TAG` constants.

## Places To Extend

- Template constants: `components/app_config/app_config.h`.
- Product-specific startup and safe hardware initialization: `main/app_main.c`.
- Product-specific routes and pages: `main/web_server.c` and `main/static/`.
- Hardware/install UI and APIs should consume role options from `components/hardware_map/` and persistence helpers from `components/nvs_store/`.
- Pump runtime persistence, APIs, and UI controls build on `components/pump_control/` without moving GPIO drive logic into `main/`.

## Incremental Refresh: Planning Structure

This section was refreshed from `.gitignore`, `.graphify_detect.json`,
`.planning/`, and `PRODUCT.md` only. Source-tree descriptions above are
retained from the prior full-map scan.

### Scoped Root Files

- `.gitignore`: local build, SDK, dependency, log, and machine-specific
  exclusions.
- `.graphify_detect.json`: generated graphify corpus inventory; do not treat as
  hand-maintained source architecture.
- `PRODUCT.md`: product register for calm, trustworthy, offline owner and
  installer workflows.

### Planning Layout

- `.planning/PROJECT.md`: evolving product and technical context.
- `.planning/STATE.md`: current milestone, workflow, progress, deferred items,
  and operator next step.
- `.planning/ROADMAP.md`: shipped milestones plus v1.2 Phases 11-14.
- `.planning/REQUIREMENTS.md`: v1.2 requirements and phase traceability.
- `.planning/MILESTONES.md`: milestone archive summary.
- `.planning/codebase/`: seven mapped-reference documents.
- `.planning/milestones/`: archived roadmap, requirement, and audit artifacts.
- `.planning/phases/`: per-phase plans, summaries, verification, research, and
  evidence.
- `.planning/quick/`: retained quick-task plans and summaries.
- `.planning/debug/`: resolved investigation artifacts, including
  `wifi-ap-sta-longrun-stuck.md`.
- `.planning/notes/`: product exploration notes.
- `.planning/research/`: milestone research artifacts.
- `.planning/todos/`: tracked and completed follow-up items.
