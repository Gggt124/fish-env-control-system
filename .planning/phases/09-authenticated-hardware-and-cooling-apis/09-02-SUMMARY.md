---
phase: 09-authenticated-hardware-and-cooling-apis
plan: 02
subsystem: firmware-api
tags: [cooling-config, cooling-mode, cjson, local-api, auth]
requires:
  - phase: 09-01
    provides: route capacity and hardware API foundation
  - phase: 08-ds18b20-cooling-runtime
    provides: cooling_control runtime/status APIs
provides:
  - authenticated GET /api/cooling/config
  - authenticated same-origin POST /api/cooling/config
  - authenticated same-origin POST /api/cooling/mode
  - validation for cooling threshold, hysteresis, auto-enable, mode, test timeout, compressor min-off, and relay polarity
affects: [phase-09, phase-10, owner-dashboard]
tech-stack:
  added: []
  patterns: [validated config POST, runtime-only Test ON, active-hardware runtime apply]
key-files:
  created: []
  modified:
    - main/web_server.c
key-decisions:
  - "Cooling config POST rejects persisted test_on mode; Test ON is exposed only through /api/cooling/mode."
  - "Cooling config runtime apply rebuilds config from the active hardware map and saved settings, never pending GPIO values."
  - "Cooling mode route persists Auto/Force OFF mode metadata but leaves auto_enable unchanged unless changed through config."
requirements-completed: [UI-07]
duration: same-session
completed: 2026-05-24
---

# Phase 9 Plan 02 Summary

**Authenticated cooling config and runtime mode APIs now sit on top of the Phase 8 cooling runtime**

## Performance

- **Duration:** same session
- **Completed:** 2026-05-24
- **Tasks:** 4
- **Files modified:** 1

## Accomplishments

- Added cooling settings status naming, config serialization, limits/enums serialization, mode parsing, relay polarity parsing, and numeric range validators.
- Added authenticated `GET /api/cooling/config` returning persisted cooling settings, validation limits, enum values, and current cooling runtime status.
- Added authenticated same-origin `POST /api/cooling/config` with full validation, NVS save, active-map runtime apply, fail-safe `cooling_control_stop()` on apply failure, and explicit apply-failure JSON.
- Added authenticated same-origin `POST /api/cooling/mode` for runtime Auto, Force OFF, and Test ON. Test ON calls `cooling_control_start_test()` and is not saved to NVS.

## Task Commits

1. **Tasks 1-4: cooling config/mode APIs** - `a2d8763` (`feat(09-02): add cooling config APIs`)

## Verification

- `rg "cooling.*config|threshold_c_x10|hysteresis_c_x10|test_timeout_sec|compressor_min_off_sec|cooling_relay_polarity|parse.*cooling" main/web_server.c`
- `rg "/api/cooling/config|handle_api_cooling_config_get|nvs_store_load_cooling_settings|api_cooling_add_status_fields|cooling.*limits" main/web_server.c`
- `rg "handle_api_cooling_config_post|nvs_store_save_cooling_settings|cooling_control_init|cooling_control_stop|nvs_store_load_hardware_map|is_same_origin\\(req, false\\)" main/web_server.c`
- `rg "/api/cooling/mode|handle_api_cooling_mode_post|cooling_control_set_mode|cooling_control_start_test|test_remaining_sec|blocked_reason|test_on" main/web_server.c`
- `.\scripts\build.ps1` passed and generated `build\fish_pump_relay_timer_control.bin`.
- `Test-Path build\fish_pump_relay_timer_control.bin` returned `True`.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None.

## User Setup Required

None.

## Next Phase Readiness

Plan 09-03 can complete dual-channel pump config mutation support, document the Phase 9 API surface, and run final build validation.

## Self-Check: PASSED

- Config POST validates all planned cooling fields before save.
- Config POST rejects `mode:"test_on"` for persistence.
- Runtime apply uses active hardware map values.
- Mode POST exposes runtime Test ON without persistence.
- New cooling mutation routes require existing session auth and strict same-origin checks.

---
*Phase: 09-authenticated-hardware-and-cooling-apis*
*Completed: 2026-05-24*
