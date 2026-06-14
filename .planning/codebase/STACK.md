# Technology Stack

**Analysis Date:** 2026-06-14

## Languages

**Primary:**
- C - All application firmware and custom ESP-IDF components
- HTML5 / Vanilla JavaScript / Vanilla CSS - Embedded web control interface (no external dependencies, CDN, or frameworks)

**Secondary:**
- Python 3.x - Build helper tools, unit tests, and utility scripts
- PowerShell - Windows development build helper scripts (`scripts/build.ps1`)
- CMake - ESP-IDF component registration and build system

## Runtime

**Environment:**
- ESP32 (classic) - DevKit V1 30-pin microcontroller target
- ESP-IDF v6.0.1 - Espressif IoT Development Framework (native APIs, no Arduino or PlatformIO layer)
- FreeRTOS - Core real-time operating system for task scheduling, queues, timers, and mutexes

**Package Manager:**
- ESP-IDF Component Manager - Automatically resolves managed component dependencies via `idf_component.yml`

## Frameworks

**Core:**
- `esp_http_server` - Lightweight embedded HTTP server (handles routing, static files, and JSON API endpoints)
- `esp_wifi` - Native ESP32 Wi-Fi driver supporting concurrent SoftAP + Station mode (APSTA)

**Testing:**
- Python `unittest` - Performs static structural and styling audits on frontend static assets (`tests/test_ui_*.py`)

**Build/Dev:**
- CMake / Ninja - Underlying build system invoked via the `idf.py` CLI wrapper

## Key Dependencies

**Critical:**
- `espressif/cjson` ^1.7.19 - JSON parsing and writing for API request/response payloads
- `espressif/mdns` ^1.7.0 - local service discovery (`fish-pump.local` resolving to direct IP)
- `esp_lcd` - Unified driver wrapper used to communicate with the ST7789/ILI9341 TFT display via SPI

**Infrastructure:**
- `nvs_flash` - Persistent non-volatile storage driver for settings and Wi-Fi credentials
- `esp_timer` - High-resolution software timer APIs for rollback/confirmation timeouts

## Configuration

**Environment:**
- Header-only defaults: `components/app_config/app_config.h`
- Persistent settings: Raw NVS partitions loaded and stored via key-value APIs in the `nvs_store` component

**Build:**
- `sdkconfig.defaults` - Boot config, targets, custom partitions, watchdog overrides, and dev flags
- `partitions.csv` - Defines layout for NVS storage, PHY calibration, OTA data, and two OTA app slots
- `main/CMakeLists.txt` - Controls C compiler flags and embeds static assets in flash via `EMBED_FILES`

## Platform Requirements

**Development:**
- Windows (Powershell host)
- ESP-IDF v6.0.1 toolchain installed at `C:\esp\v6.0.1\esp-idf`
- Python dependencies (requirements.core.txt)

**Production:**
- ESP32 DevKit V1 30-pin board
- Peripherals:
  - 1x Binary float switch
  - 2x Pump relay modules
  - 1x DS18B20 temperature sensor (cooling loop)
  - 1x Cooling relay module
  - 1x ST7789 or ILI9341 LCD panel

---

*Stack analysis: 2026-06-14*
*Update after major dependency changes*
