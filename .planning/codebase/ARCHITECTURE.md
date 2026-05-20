---
generated: 2026-05-18
focus: arch
---

# Architecture

## Summary

The codebase is layered ESP-IDF firmware. Reusable low-level modules live in `components/`; project-specific startup, web routes, captive DNS, and embedded UI live in `main/`. The device initializes a hardware-safe pump-control core, starts local services, exposes a small authenticated web UI, persists network configuration in NVS, and keeps SoftAP available as a setup fallback.

## Entry Point

- `main/app_main.c` is the firmware entry point.
- Boot order is pump-control safe init, NVS, session store, Wi-Fi manager, HTTP server, Wi-Fi power-save disable, mDNS, captive DNS, task watchdog, then a status loop.
- The main loop resets the task watchdog every 5 seconds, retries HTTP server startup if needed, and logs device status periodically.

## Component Boundaries

- `components/app_config/app_config.h` is a header-only template configuration boundary.
- `components/nvs_store/` owns persistent storage access and hides raw NVS keys from the rest of the app.
- `components/session/` owns volatile login token lifecycle.
- `components/wifi_manager/` owns ESP-IDF Wi-Fi mode, AP state, STA state, scans, retry behavior, static IP application, and fallback behavior.
- `components/pump_control/` owns pump GPIO config validation, relay inactive setup, binary float debounce, timer selection, ON/OFF phase transitions, start/stop behavior, and status snapshots.
- `main/web_server.c` owns HTTP routing, static file serving, auth checks, request parsing, JSON responses, and status aggregation.
- `main/dns_server.c` owns the captive DNS UDP task.

## Data Flow: Boot

1. `app_main()` calls `pump_control_default_config()` and `pump_control_init()` so the relay GPIO is driven inactive while pump control remains stopped.
2. `app_main()` calls `nvs_store_init()` in `components/nvs_store/nvs_store.c`.
3. `app_main()` calls `session_init()` in `components/session/session.c`.
4. `app_main()` calls `wifi_manager_init()` in `components/wifi_manager/wifi_manager.c`.
5. `wifi_manager_init()` initializes netif, event loop, AP/STA interfaces, ESP Wi-Fi, event handlers, and AP auto-stop timer.
6. `wifi_manager_init()` loads saved STA credentials and optional static IP config through `nvs_store_load_wifi()` and `nvs_store_load_sta_ip()`.
7. `wifi_manager_init()` starts Wi-Fi and then calls `wifi_manager_start_ap()`.
8. `app_main()` starts HTTP via `web_server_start()`.
9. `app_main()` starts mDNS and captive DNS.

## Data Flow: Login

1. Browser posts JSON or form-style credentials to `/api/login`.
2. `handle_api_login()` in `main/web_server.c` validates same-origin hints, rate limits failures, parses the request body, and compares credentials to `APP_TEMPLATE_DEFAULT_USERNAME` and `APP_TEMPLATE_DEFAULT_PASSWORD`.
3. On success, `session_create()` stores a RAM token in `components/session/session.c`.
4. The HTTP response sets a `session` cookie.
5. Protected page and API handlers call `require_auth()`, which extracts the cookie and calls `session_validate()`.

## Data Flow: Wi-Fi Connect

1. Browser selects an SSID in `main/static/app.js` and posts JSON to `/api/wifi/connect`.
2. `handle_api_wifi_connect()` in `main/web_server.c` validates authentication, same-origin headers, SSID length, password length, and static IP fields.
3. `wifi_manager_connect_sta()` stores credentials through `nvs_store_save_wifi()`.
4. Optional static IP settings are applied with LwIP `ip4addr_aton()` and persisted through `nvs_store_save_sta_ip()`.
5. ESP Wi-Fi station config is set with `esp_wifi_set_config()` and connection begins with `esp_wifi_connect()`.
6. The handler waits 5 seconds, then reports success or failure using `wifi_manager_is_sta_connected()`, `wifi_manager_get_sta_ssid()`, and `wifi_manager_get_sta_ip()`.

## Data Flow: Pump Config API

1. Authenticated clients call GET `/api/pump/config` for persisted timer durations, `auto_start`, relay polarity, read-only GPIO/debounce defaults, and settings load health.
2. Authenticated same-origin clients call POST `/api/pump/config` with a full replacement payload.
3. `main/web_server.c` rejects read-only hardware/debounce fields, validates timer duration bounds and relay polarity, saves through `nvs_store_save_pump_settings()`, then reinitializes `pump_control` from defaults merged with saved settings.
4. If the controller was running before save, the handler stops it to force relay inactive during reinit and restarts only after successful runtime apply.

## Data Flow: Pump Runtime API

- `/api/pump/status` returns machine-friendly pump runtime fields from `pump_control_get_status()` plus `auto_start` and `settings_status` from NVS.
- `/api/pump/start` and `/api/pump/stop` are authenticated same-origin POST routes.
- Start is idempotent when already running; stop is idempotent when already stopped and confirms relay inactive before success.

## Data Flow: Status

- `/api/status` is handled in `handle_api_status()` in `main/web_server.c`.
- Status combines chip metadata, IDF version, MAC addresses, heap metrics, uptime, Wi-Fi mode, AP client count, STA details, and captive DNS state.
- Pump runtime status is intentionally separate under `/api/pump/status`.
- Frontend pages update compact system status and status tables by polling `/api/status`; the dashboard uses `/api/pump/status` for pump runtime state.

## State Management

- Wi-Fi runtime state is stored in static globals in `components/wifi_manager/wifi_manager.c`, protected by `s_wifi_mutex` in most state mutations.
- Pump runtime state is stored in static globals in `components/pump_control/pump_control.c`, protected by `s_pump_mutex`.
- Session state is stored in a fixed static array in `components/session/session.c`, protected by `s_session_mutex`.
- DNS task state is stored in static globals `s_dns_task` and `s_dns_sock` in `main/dns_server.c`.
- Login rate-limit counters are static locals inside `handle_api_login()` in `main/web_server.c`.

## Concurrency Model

- ESP-IDF event callbacks drive Wi-Fi state transitions in `wifi_event_handler()`.
- A component-owned periodic `esp_timer` callback drives pump-control debounce and timer phase checks.
- The captive DNS server runs as a FreeRTOS task created by `xTaskCreate()` in `main/dns_server.c`.
- Wi-Fi scans are asynchronous at the ESP-IDF layer, then converted into a request-scoped blocking wait with a binary semaphore in `main/web_server.c`.
- The main task runs indefinitely and owns watchdog reset plus periodic logging.

## Design Boundary

The firmware now has the pump-control core, NVS-backed pump settings, boot auto-start behavior, authenticated pump API routes, and a local pump-control dashboard. Hardware validation is still deferred to Phase 5 hardware procedures for flash/manual relay and float validation.
