---
generated: 2026-05-18
focus: tech
---

# Technology Stack

## Summary

This repository is classic ESP32 firmware built with ESP-IDF. It provides a reusable local Wi-Fi setup and web dashboard foundation plus a hardware-safe pump-control core for a fish pump relay timer controller.

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
- GPIO driver: `esp_driver_gpio`, used by `components/pump_control/pump_control.c` for float input and relay output.
- Timers and uptime: `esp_timer`, used by `components/session/session.c`, `components/wifi_manager/wifi_manager.c`, `components/pump_control/pump_control.c`, and `main/web_server.c`.
- FreeRTOS tasks and semaphores: used by `components/session/session.c`, `components/wifi_manager/wifi_manager.c`, `components/pump_control/pump_control.c`, `main/dns_server.c`, and `main/app_main.c`.
- Task watchdog: `esp_task_wdt`, initialized manually in `main/app_main.c`.
- LwIP sockets and IPv4 parsing: `main/dns_server.c` and `components/wifi_manager/wifi_manager.c`.
- Heap and chip status APIs: `esp_heap_caps`, `esp_chip_info`, `esp_mac`, `esp_idf_version`, used in `main/web_server.c`.

## Managed Dependencies

Declared in `main/idf_component.yml`:

- `espressif/cjson` at `^1.7.19`, used for JSON request and response handling in `main/web_server.c`.
- `espressif/mdns` at `^1.7.0`, used for `fish-pump.local` service discovery from `main/app_main.c`.

The resolved dependency lock is stored in `dependencies.lock`.

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

- Product names, AP SSID, mDNS host, credentials, limits, AP auto-stop settings, HTTP handler capacity, watchdog timeout, and pump-control source defaults live in `components/app_config/app_config.h`.
- Wi-Fi and optional static IP runtime settings persist in NVS through `components/nvs_store/nvs_store.c`.
