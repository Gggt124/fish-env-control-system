---
phase: 08-ds18b20-cooling-runtime
plan: 01
subsystem: firmware-runtime
tags: [esp-idf, ds18b20, onewire, cooling-control, sensor-fault]
requires:
  - phase: 06-hardware-contract-and-persistent-map-foundation
    provides: active DS18B20 GPIO, cooling relay GPIO, and cooling settings defaults
provides:
  - cooling_control component boundary
  - ESP-IDF managed DS18B20 and 1-Wire dependencies
  - DS18B20 discovery and periodic temperature read loop
  - unknown/ok/fault sensor status with 3-failure and 2-success thresholds
affects: [phase-8, phase-9, cooling-api, hardware-validation]
tech-stack:
  added: [espressif/onewire_bus, espressif/ds18b20]
  patterns: [component-owned FreeRTOS polling task, mutex-protected status snapshots, safe-off sensor fault handling]
key-files:
  created:
    - components/cooling_control/CMakeLists.txt
    - components/cooling_control/idf_component.yml
    - components/cooling_control/cooling_control.h
    - components/cooling_control/cooling_control.c
  modified: []
key-decisions:
  - "The DS18B20 conversion/read loop runs in a component-owned FreeRTOS task so conversion delay does not block an esp_timer callback."
  - "The first discovered DS18B20 device is used for Phase 8; multi-sensor selection remains out of scope."
  - "85.0 C DS18B20 power-on state is treated as invalid input and feeds the safe-off fault path."
patterns-established:
  - "Cooling runtime exposes prefix-scoped config/status types and hides driver APIs inside the component."
  - "Sensor failure never immediately energizes or trusts stale automatic cooling data."
requirements-completed: [COOL-01, COOL-04]
duration: same-session
completed: 2026-05-23
---

# Phase 8 Plan 01 Summary

**ESP-IDF DS18B20 cooling sensor foundation with managed 1-Wire dependency and fail-safe fault thresholds**

## Performance

- **Duration:** same session
- **Completed:** 2026-05-23
- **Tasks:** 4
- **Files modified:** 4

## Accomplishments

- Added `components/cooling_control` with CMake, manifest, public header, and implementation.
- Declared ESP-IDF-native `espressif/onewire_bus` and `espressif/ds18b20` dependencies.
- Implemented DS18B20 bus discovery on the configured GPIO and first-sensor binding.
- Implemented periodic read behavior with startup `unknown`, fault after 3 consecutive failures, and recovery after 2 consecutive successful reads.

## Task Commits

1. **Sensor runtime foundation:** `83fed92` (`feat(08): add ds18b20 cooling runtime`)

## Verification

- `rg` checks confirmed managed dependency declarations, `cooling_control_status_t`, DS18B20 discovery/read calls, sensor state, fault codes, and read counters.
- `.\scripts\build.ps1` passed and generated `build\fish_pump_relay_timer_control.bin`.
- `Test-Path build\fish_pump_relay_timer_control.bin` returned `True`.

## Deviations from Plan

- Used a FreeRTOS task instead of `esp_timer` for DS18B20 reads because conversion delay is blocking and should not run inside a timer callback.
- `dependencies.lock` was updated by ESP-IDF component manager but is ignored by project `.gitignore`; dependency provenance is tracked in component manifests and `REFERENCE.md`.

## User Setup Required

Wire one powered-mode DS18B20 on the active/default GPIO33 with an external 4.7 kOhm pull-up to 3.3 V before hardware validation.

## Next Phase Readiness

Plan 02 can use the component status contract to drive relay mode, hysteresis, Test ON, and compressor lockout behavior.

---
*Phase: 08-ds18b20-cooling-runtime*
*Completed: 2026-05-23*
