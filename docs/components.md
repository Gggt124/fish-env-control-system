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

## cooling_control

Component-owned DS18B20 cooling runtime.

- Uses the active hardware map DS18B20 GPIO and cooling relay GPIO.
- Uses ESP-IDF managed `espressif/onewire_bus` and `espressif/ds18b20`
  components for powered-mode DS18B20 reads.
- Keeps startup temperature as `unknown` until the first valid reading.
- Declares sensor fault after 3 consecutive read failures and clears it after
  2 consecutive successful readings.
- Rediscovers the one-wire bus after read failures so a missing or reconnected
  DS18B20 can recover without reboot.
- Controls the dedicated cooling relay separately from pump relays.
- In Auto mode, turns cooling ON at or above threshold and OFF below threshold
  minus hysteresis.
- Force OFF always holds the cooling relay inactive.
- Test ON is runtime-only, timeout-limited, and still obeys compressor
  minimum off-time.
- Boot/reinit treats the relay as just turned OFF and enforces the configured
  minimum off-time before any ON transition.

`main/web_server.c` exposes runtime state through authenticated
`GET /api/cooling/status`. Phase 9 also exposes authenticated cooling config
and mode APIs:

- `GET /api/cooling/config` returns persisted threshold, hysteresis,
  auto-enable, mode, test timeout, compressor minimum off-time, relay polarity,
  limits, enum values, and current cooling status.
- `POST /api/cooling/config` validates and saves safe persistent settings,
  then reapplies the runtime using the active hardware map.
- `POST /api/cooling/mode` changes runtime mode. `test_on` is runtime-only and
  is not persisted across reboot.

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
The HTTP server keeps a small open-socket limit, enables LRU purge, sends
`Connection: close`, and uses short send/receive timeouts so browser polling
cannot exhaust the ESP32/LwIP socket table during long-running dashboard use.

Template-owned routes:

- login/logout
- dashboard/status/wifi/hardware static pages
- Wi-Fi scan/connect/disconnect APIs
- status JSON API

Add product-specific pages and APIs here, but keep auth/session and Wi-Fi route
behavior stable unless the template contract changes.

Phase 6 through 8 integration notes:

- `main/app_main.c` loads the active hardware map and current pump settings at
  boot. Phase 7 pump runtime consumes float, Relay 1, and Relay 2.
- `main/web_server.c` keeps `/api/pump/config` compatible with existing clients
  while surfacing hardware map fields as read-only data.
- Phase 8 initializes `cooling_control` from active DS18B20/cooling relay GPIOs
  and persisted cooling settings, then exposes status through
  `/api/cooling/status` without adding cooling mutation routes.

Phase 9 API notes:

- `/api/hardware/map` exposes active GPIO values, pending GPIO values, safe
  role-specific GPIO option lists, and reboot-required state. POST saves
  pending values only and requires `confirm_reboot_required: true`.
- `/api/pump/config` supports dual-channel pump settings: `relay1_polarity`,
  `relay2_polarity`, `timer1_start_phase`, and `timer2_start_phase`. The
  legacy `relay_polarity` field remains a Relay 1 compatibility alias.
- GPIO pin changes are not accepted through `/api/pump/config`; they belong to
  `/api/hardware/map` and take effect only after reboot.
Phase 10 UI notes:

- `/dashboard` is the daily operation surface. It shows pump enabled/running
  state, float state, active timer/relay, countdown, cooling temperature,
  cooling relay state, threshold, boot auto-enable, sensor fault state, lockout,
  and bounded Test ON controls.
- `/hardware` is the protected installer surface. It embeds `hardware.html`,
  renders wiring first, then active/pending GPIO summaries, then technical
  option metadata from `/api/hardware/map`.
- The frontend populates GPIO dropdowns from firmware-provided safe option
  arrays and POSTs pending changes only with `confirm_reboot_required: true`.
  No hardcoded duplicate GPIO safety list or freeform GPIO input belongs in the
  UI.
