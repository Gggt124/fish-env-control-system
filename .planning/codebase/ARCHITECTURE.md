# System Architecture

**Analysis Date:** 2026-06-14

## Pattern Overview

**Overall:** Monolithic Embedded C Firmware with Modular Components and Client-Server Web UI
The application runs on the bare-metal ESP32 system using ESP-IDF and FreeRTOS. It boots into concurrent tasks running the web server, DNS captive portal, TFT status screen, cooling loop, and pump timer controller. A vanilla HTML/JS web panel acts as the main control dashboard.

**Key Characteristics:**
- **Modular Component Design:** Distinct, self-contained subfolders in `components/` handle single concerns (Wi-Fi, NVS, Session, Pump, Cooling).
- **Concurrency Model:** Multi-threaded execution via FreeRTOS tasks (DNS task, Hardware UI task, TFT update task, Pump timer tick, Cooling controller).
- **Event-Driven Wi-Fi:** State transitions driven by ESP-IDF system event callbacks.
- **Fail-Safe Rollbacks:** Configuration modifications (Wi-Fi, GPIO map) are staged and verified via hardware-managed watchdog timers before committing.

## Layers

**Orchestration / Entry Layer:**
- Purpose: Boots the hardware, reads configs, initializes NVS, and spawns background tasks.
- Location: `main/app_main.c` (main thread & `hardware_ui_task`)
- Depends on: All components, `esp_http_server`, `esp_wifi`, `esp_task_wdt`

**Interface Layer (Web & DNS):**
- Purpose: Serves static web pages, hosts REST APIs for system control, and redirects clients in SoftAP captive portal.
- Location: `main/web_server.c`, `main/dns_server.c`
- Depends on: `nvs_store`, `session`, `wifi_manager`, `pump_control`, `cooling_control`, `cjson`

**Display Layer:**
- Purpose: Updates the local physical TFT screen with system status cards.
- Location: `main/tft_display.c`
- Depends on: `esp_lcd`, `pump_control`, `cooling_control`, `wifi_manager`

**Core Business Logic Layer:**
- Purpose: Physical relay logic, float-switch input debouncing, and temperature sensors.
- Location: `components/pump_control/`, `components/cooling_control/`, `components/hardware_map/`
- Depends on: `driver/gpio`, NVS structs, `esp_timer`

**Infrastructure Layer:**
- Purpose: Reusable modules for flash serialization, session tokens, and compile-time defaults.
- Location: `components/nvs_store/`, `components/session/`, `components/app_config/`
- Depends on: `nvs_flash`, `esp_system`, `esp_random`

## Data Flow

### 1. Pump Control Lifecycle (Float-Triggered Timer Swap)
1. Debounced float switch task (`components/pump_control/pump_control.c`) detects level change.
2. If Float switch is OFF -> Selects Timer 1 intervals. If Float switch is ON -> Selects Timer 2.
3. The active timer resets its phase (swapping to initial ON or OFF phase).
4. The active relay (Relay 1 or Relay 2) is energized/de-energized according to the phase.
5. Telemetry is updated, and the countdown timer decrements every second.

### 2. Cooling Control Lifecycle (DS18B20 Bang-Bang Controller)
1. Cooling controller task reads temperature from the DS18B20 sensor.
2. If temperature is valid and exceeds `Threshold + Hysteresis`, and the compressor minimum-off lockout is not active, the cooling relay is energized.
3. Once temperature drops below `Threshold - Hysteresis`, the cooling relay is shut off and the compressor minimum-off timer starts.

### 3. API Config Update (Staging & Confirmation Flow)
1. User submits a config change via POST (e.g., `/api/wifi/connect`).
2. `web_server.c` validates the session, parses the JSON payload, and writes the staged settings to NVS via `nvs_store_stage_wifi()`.
3. The ESP32 is rebooted.
4. On boot, `app_main.c` detects staged config and starts rollback timers (`s_wifi_timer`, `s_confirm_timer`).
5. If the connection succeeds and the user clicks "Confirm" in the UI (sending POST to `/api/confirm`), `nvs_store_commit_staging()` runs, committing the changes.
6. If the timer expires or the connection fails, the staged changes are discarded by `nvs_store_rollback_staging()` and the device restarts.

## Key Abstractions

- **Config and Status Structs:** Config options are collected in structured structs (e.g., `pump_control_config_t`) passed during initialization. State is reported via read-only telemetry structs (e.g., `pump_control_status_t`).
- **NVS Settings Serialization:** `nvs_store` wraps raw NVS namespaces and namespace keys into structured functions (`nvs_store_load_pump_settings()`, etc.).
- **Hardware Map:** `hardware_map` defines the pin assignments for all active functions, allowing virtual swapping of GPIO pins at runtime without recompilation.

## Entry Points

- **Firmware Entry (`app_main`):** Located in `main/app_main.c`. Run on Core 0. Launches primary components.
- **Web Requests:** Handlers in `main/web_server.c` registered to `esp_http_server` match incoming paths.
- **Display Task (`tft_task`):** Created in `main/tft_display.c` to update the screen layout.
- **Button Watcher (`hardware_ui_task`):** Monitored in `main/app_main.c` to catch bootloader hold states.

## Error Handling

- **Fail-Safe Defaults:** If NVS read fails, default configurations from `app_config.h` are loaded.
- **Hardware Watchdog:** The Task Watchdog (TWDT) is initialized in `app_main.c` with a 10-second timeout. Core loops must reset it or the system panics and restarts.
- **Sensor Fault Mode:** If the DS18B20 sensor fails, `cooling_control` enters a fault state and immediately de-energizes the cooling relay to prevent freezing or runaways.

## Cross-Cutting Concerns

- **Thread Safety:** Semaphores and mutexes protect shared states (Wi-Fi manager lists, session buffers, pump config).
- **Authentication Guard:** API routes inspect the browser cookie session token. Unauthenticated users are redirected to `/login` or receive `401 Unauthorized` JSON responses.

---

*Architecture analysis: 2026-06-14*
*Update when major patterns change*
