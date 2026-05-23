---
phase: 07-dual-timer-relay-pump-runtime
plan: 02
subsystem: firmware-api
tags: [esp-idf, http-api, nvs, pump-control, dual-relay]
requires:
  - phase: 07-dual-timer-relay-pump-runtime
    provides: dual-relay pump_control runtime contract from Plan 01
provides:
  - Boot-time mapping of hardware map and pump settings into dual-relay runtime
  - Backward-compatible pump status API with Relay 1/Relay 2 fields
  - Authenticated Timer 1/Timer 2 start phase save path
affects: [app_main, web_server, dashboard, nvs]
tech-stack:
  added: []
  patterns: [full-replacement config save, save-stop-reinit-restart runtime apply]
key-files:
  created: []
  modified:
    - main/app_main.c
    - main/web_server.c
key-decisions:
  - "`relay_gpio` API remains Relay 1 compatibility; `active_relay_gpio` carries the active/ready relay GPIO."
  - "Timer start phase values are stable strings: `on` and `off`."
  - "Config save still commits NVS before stopping/reinitializing runtime."
patterns-established:
  - "Map hardware-layer enums to pump_control enums at integration boundaries."
  - "Expose new API fields additively while preserving existing client fields."
requirements-completed: [PUMP-01, PUMP-02, PUMP-03, PUMP-04, PUMP-05, PUMP-06]
duration: same-session
completed: 2026-05-23
---

# Phase 7 Plan 02 Summary

**Boot and authenticated pump APIs now initialize, report, persist, and safely apply the dual-relay runtime contract**

## Performance

- **Duration:** same session
- **Completed:** 2026-05-23
- **Tasks:** 4
- **Files modified:** 2

## Accomplishments
- Boot now passes float GPIO, Relay 1 GPIO, Relay 2 GPIO, relay polarities, timer durations, and start phases into `pump_control`.
- `/api/pump/status` now includes `active_relay`, `relay1_energized`, `relay2_energized`, `fault`, and `active_relay_gpio`.
- `/api/pump/config` now accepts and persists `timer1_start_phase` and `timer2_start_phase` as `on`/`off`.
- Safe save behavior remains: save NVS first, stop if running, reinit runtime, restart only if previously running.

## Task Commits

1. **Boot/API integration:** `2856dc8` (`feat(07-02): wire dual relay runtime into boot and API`)

## Verification
- `rg` checks confirmed boot mapping, dual-relay status fields, read-only hardware fields, start phase validation, and runtime apply order.
- `.\scripts\build.ps1` passed and generated `build\fish_pump_relay_timer_control.bin`.

## Deviations from Plan

None - plan executed as written.

## User Setup Required

Manual API validation on flashed hardware is recommended for config save while running.

## Next Phase Readiness

Dashboard code can consume `active_relay`, per-relay energized fields, and start phase config fields directly.

---
*Phase: 07-dual-timer-relay-pump-runtime*
*Completed: 2026-05-23*
