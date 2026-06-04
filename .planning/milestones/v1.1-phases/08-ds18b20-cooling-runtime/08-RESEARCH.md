# Phase 8 Research: DS18B20 Cooling Runtime

**Phase:** 08-ds18b20-cooling-runtime  
**Date:** 2026-05-23  
**Status:** Complete

## Scope Read

Phase 8 adds a separate cooling runtime. It must read a DS18B20 temperature sensor, drive a dedicated cooling relay through threshold/hysteresis, fail safe with relay OFF on missing/unreadable sensor, support Auto / Force OFF / Test ON mode semantics, honor the cooling auto-enable boot preference, and enforce compressor minimum off-time by default.

Full cooling configuration/mutation APIs, hardware-map mutation APIs, Hardware/Install UI, dashboard redesign, event logs, charts, WebSockets, OTA, cloud/MQTT, HTTPS, and production auth hardening remain out of scope.

## Official Component Research

Espressif publishes official ESP-IDF-compatible components for both the 1-Wire bus and DS18B20 device driver:

- `espressif/onewire_bus` v1.1.1 is the current registry component for Dallas 1-Wire bus support. It supports RMT and UART backends, device discovery/enumeration, byte/bit read/write, CRC8, and multiple devices on one bus. The registry install command is `idf.py add-dependency "espressif/onewire_bus^1.1.1"`. Source: https://components.espressif.com/components/espressif/onewire_bus
- `espressif/ds18b20` v0.3.1 is the current registry component for DS18B20 sensors. It depends on `onewire_bus`, enumerates devices from the 1-Wire bus, then triggers conversion and reads temperatures through `ds18b20_trigger_temperature_conversion_for_all()` and `ds18b20_get_temperature()`. Source: https://components.espressif.com/components/espressif/ds18b20/
- The DS18B20 registry changelog notes detection for the uninitialized 85.0 C power-on state returning `ESP_ERR_INVALID_STATE`; Phase 8 should treat this as invalid/not-ready input rather than a real temperature. Source: https://components.espressif.com/components/espressif/ds18b20/versions/0.3.1/changelog

Recommendation: use the Espressif registry components instead of hand-rolling 1-Wire timing. This keeps the project ESP-IDF-native and avoids importing Arduino/PlatformIO libraries.

## Existing Implementation

### Hardware And Storage Foundation

Phase 6 already provides the hardware/storage foundation:

- `hardware_map_t` has `ds18b20_data_gpio` and `cooling_relay_gpio`.
- `hardware_map_cooling_mode_t` has `auto`, `force_off`, and `test_on`.
- `nvs_store_cooling_settings_t` stores threshold, hysteresis, auto-enable, mode, test timeout, compressor minimum off-time, and cooling relay polarity.
- Defaults are safe-off: threshold 30.0 C, hysteresis 1.0 C, auto-enable false, mode force-off, test timeout 10 seconds, compressor minimum off-time 300 seconds.
- `docs/hardware.md` documents powered-mode DS18B20 wiring with 3.3 V, GND, DQ, and an external 4.7 kOhm pull-up.

### Runtime Pattern To Reuse

`components/pump_control/` is the closest runtime pattern:

- component-owned static state
- mutex-protected snapshots
- GPIO validation and fail-safe relay writes
- `esp_timer`-driven periodic work
- public config/status contract consumed by boot and web server code

Phase 8 should add a separate `components/cooling_control/` component rather than mixing cooling state into `pump_control` or `web_server.c`.

### Boot And API Surface

`main/app_main.c` already loads active hardware map and cooling settings, but only logs them. Phase 8 should initialize the cooling runtime from the active DS18B20 GPIO, cooling relay GPIO, relay polarity, and persisted cooling settings.

`main/web_server.c` already has authenticated GET API patterns, JSON helpers, and route registration. Phase 8 should add `GET /api/cooling/status` only. Full cooling config/mutation APIs remain Phase 9.

## Recommended Plan Split

### Plan 08-01: Cooling Control Component And DS18B20 Sensor State

Create the `cooling_control` component, add official Espressif DS18B20/1-Wire component dependencies, and implement the public config/status contract plus sensor read/fault state:

- initialize 1-Wire on active DS18B20 GPIO
- enumerate exactly one DS18B20 for the current hardware contract
- handle startup unknown safely
- fault after 3 consecutive read failures
- recover after 2 consecutive successful reads
- treat out-of-range and invalid/uninitialized readings as fault inputs

### Plan 08-02: Cooling Relay, Modes, Hysteresis, And Compressor Protection

Extend `cooling_control` with relay control and mode state machine:

- configure cooling relay inactive at init and fail paths
- Auto turns relay ON at or above threshold and OFF below threshold minus hysteresis
- Force OFF always keeps relay OFF
- Test ON is runtime-only, timeout-limited, returns to previous mode, and may run with sensor unknown/fault
- boot/reinit starts compressor lockout
- Auto and Test ON both obey minimum off-time
- every ON-to-OFF transition restarts lockout

### Plan 08-03: Boot Integration, Cooling Status API, Docs, And Build

Wire the runtime into boot and expose validation surface:

- add `cooling_control` to build graph
- initialize from active hardware map plus persisted cooling settings
- suppress unsafe relay behavior on invalid hardware/settings
- add authenticated `GET /api/cooling/status`
- serialize stable status fields and enums
- update docs/development notes and `REFERENCE.md`
- run ESP-IDF build validation

## Key Risks

- **Short-cycle risk:** boot/reinit must not immediately energize cooling relay if downstream hardware is a compressor.
- **False sensor faults:** single read miss should not fault; use the locked 3-failure threshold and 2-success recovery.
- **Test ON confusion:** Test ON can run without a good sensor, but status must make it clear this is manual/test behavior and still subject to compressor lockout.
- **Dependency drift:** adding managed components may update `dependencies.lock`; executor should review the lockfile diff and keep dependency versions bounded.
- **Scope creep:** do not add cooling config POST routes, hardware GPIO mutation routes, or dashboard redesign in Phase 8.

## Verification Guidance

Automated validation remains `.\scripts\build.ps1` and `Test-Path build\fish_pump_relay_timer_control.bin`.

Focused source checks should confirm:

- `components/cooling_control/` exists with a public config/status contract.
- official `espressif/ds18b20` and `espressif/onewire_bus` dependencies are declared.
- cooling relay is forced inactive on init failure, stop/reinit, sensor fault, Force OFF, and compressor lockout.
- threshold/hysteresis and lockout state are machine-readable in status.
- `GET /api/cooling/status` is authenticated and does not mutate settings.
- `/api/pump/status` remains pump-focused.

Manual hardware validation after execution should cover:

1. Missing/disconnected DS18B20 forces cooling relay OFF and reports sensor fault after 3 failed reads.
2. Reconnected sensor clears fault after 2 successful reads.
3. Auto mode energizes relay at or above threshold after lockout and de-energizes below threshold minus hysteresis.
4. Force OFF keeps relay OFF regardless of temperature.
5. Test ON times out and returns to the previous mode, but is blocked during compressor lockout.

## RESEARCH COMPLETE
