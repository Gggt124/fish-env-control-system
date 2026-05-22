# Component Guide

## app_config

Header-only component with template-level constants. Downstream projects should
customize this first.

Important values:

- AP SSID and channel
- mDNS hostname and instance name
- default login credentials
- session timeout (`0` disables automatic expiry)
- AP auto-stop default
- HTTP route capacity
- watchdog timeout

## nvs_store

Persists Wi-Fi and optional static STA IP settings.

Namespace: `wifi_cfg`

Keys:

- `sta_ssid`
- `sta_pass`
- `sta_ip`
- `sta_gw`
- `sta_netmask`
- `sta_dns`
- `ap_stop_tmo`
- `ap_auto_stop`

It also owns project-specific persistent pump hardware settings. Raw NVS keys
stay inside `components/nvs_store/nvs_store.c`; higher layers use typed wrappers.

Project namespaces:

- `hw_cfg`: active GPIO map and pending GPIO map; pending values require reboot.
- `pump_cfg`: timer durations, `relay_low` legacy alias, `r1_low`, `r2_low`,
  `t1_start`, `t2_start`, and auto-start.
- `cool_cfg`: threshold, hysteresis, auto-enable, mode, test timeout,
  compressor minimum off-time, and cooling relay polarity.

## hardware_map

Low-level hardware contract for the ESP32 DevKit V1 pin map.

- Defines `hardware_role_t` for float input, pump Relay 1, pump Relay 2,
  DS18B20 data, and cooling relay.
- Exposes conservative role-specific GPIO option lists and defaults.
- Exposes relay polarity, timer start phase, and cooling mode enums.
- Validates maps without initializing GPIO hardware.

Detailed wiring and defaults are documented in `docs/hardware.md`.

## session

Small RAM-only session store for the local web UI.

- Token length: 16 hex chars plus null terminator
- Default max age: `APP_TEMPLATE_SESSION_MAX_AGE_SEC` (`0` means no automatic timeout)
- No reboot persistence
- Intended for local setup only

## wifi_manager

Owns Wi-Fi init and runtime state.

- Starts AP+STA mode
- Starts SoftAP immediately
- Loads saved STA credentials from NVS
- Scans Wi-Fi asynchronously
- Saves new STA credentials on connect
- Restores AP fallback after STA failures
- Optionally stops AP after STA gets IP

Public API is declared in `components/wifi_manager/wifi_manager.h`.

## main web server

`main/web_server.c` is intentionally project-level glue because it references
embedded static file symbols from `main/CMakeLists.txt`.

Template-owned routes:

- login/logout
- dashboard/status/wifi static pages
- Wi-Fi scan/connect/disconnect APIs
- status JSON API

Add product-specific pages and APIs here, but keep auth/session and Wi-Fi route
behavior stable unless the template contract changes.

Phase 6 integration notes:

- `main/app_main.c` loads the active hardware map and current pump settings at
  boot. The existing single-relay runtime consumes only the float GPIO and pump
  Relay 1 GPIO until Phase 7 changes pump behavior.
- `main/web_server.c` keeps `/api/pump/config` compatible with existing clients
  while surfacing hardware map fields as read-only data.
- Relay 2, DS18B20, and cooling relay pins are stored and reported but not
  driven in Phase 6.
