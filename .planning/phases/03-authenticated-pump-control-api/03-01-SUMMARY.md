---
phase: 03-authenticated-pump-control-api
plan: 01
subsystem: api
tags: [esp-idf, http-server, cjson, nvs, pump-control]
requires:
  - phase: 01-hardware-safe-pump-control-core
    provides: pump_control runtime defaults, status, start, stop, and safe relay inactive behavior
  - phase: 02-pump-settings-persistence-and-boot-behavior
    provides: NVS-backed pump settings load/save contract
provides:
  - Authenticated pump config read endpoint contract
  - Authenticated full-replacement pump config save endpoint logic
  - Pump settings JSON conversion and safe runtime apply helpers
affects: [phase-04-web-ui, pump-api, local-dashboard]
tech-stack:
  added: []
  patterns: [protected pump API handlers, save-before-apply runtime reinit, read-only hardware defaults]
key-files:
  created:
    - .planning/phases/03-authenticated-pump-control-api/03-01-SUMMARY.md
  modified:
    - main/web_server.c
key-decisions:
  - "Pump config responses expose GPIO and debounce values as read-only defaults from pump_control_default_config()."
  - "Pump config save persists NVS settings before applying runtime changes, and restarts only when the controller was already running."
patterns-established:
  - "Pump API errors use ok:false with stable error codes and short messages."
  - "Editable pump settings are parsed as a full replacement payload."
requirements-completed: [API-01, API-02, API-06]
duration: 22 min
completed: 2026-05-20
---

# Phase 03 Plan 01: Pump Config API Summary

**Authenticated pump configuration read/save API with NVS persistence and safe runtime reinitialization**

## Performance

- **Duration:** 22 min
- **Started:** 2026-05-20T00:00:00Z
- **Completed:** 2026-05-20T00:22:00Z
- **Tasks:** 3 completed
- **Files modified:** 1

## Accomplishments

- Added pump API helper functions for settings load status strings, relay polarity strings, and NVS-to-runtime config mapping.
- Added authenticated `GET /api/pump/config` response fields for timers, `auto_start`, `relay_polarity`, read-only GPIO/debounce defaults, and `settings_status`.
- Added authenticated same-origin `POST /api/pump/config` full-replacement save with first-error validation, read-only field rejection, NVS save-before-apply, and safe runtime restart behavior.

## Task Commits

1. **Task 1: Add pump web-server helper layer** - `587783f` (feat)
2. **Tasks 2-3: Implement GET/POST /api/pump/config** - `649d082` (feat)

## Files Created/Modified

- `main/web_server.c` - Pump config helpers and authenticated read/save handlers.

## Decisions Made

- GPIO/debounce values are surfaced from `pump_control_default_config()` and rejected if sent in save payloads.
- `auto_start` remains boot-only; config save only restarts runtime when the controller was already running before the save.
- Runtime config application starts from `pump_control_default_config()` so hardware defaults remain centralized.

## Deviations from Plan

The GET and POST config handlers were committed together because their validation and response helpers are shared. No behavioral scope was added beyond the plan.

**Total deviations:** 0 auto-fixed.
**Impact:** No scope creep; commit granularity is coarser than the task list for the interdependent handlers.

## Issues Encountered

None.

## Verification

- `rg "GET /api/pump/config|POST /api/pump/config|handle_api_pump_config" main/web_server.c` passed.
- `rg "require_auth\\(req\\)|is_same_origin\\(req\\)" main/web_server.c` passed.
- `rg "timer1_on_sec|timer1_off_sec|timer2_on_sec|timer2_off_sec|auto_start|relay_polarity" main/web_server.c` passed.
- `rg "float_gpio|relay_gpio|debounce_ms|read_only" main/web_server.c` passed.
- `rg "nvs_store_save_pump_settings|pump_control_init|pump_control_stop|pump_control_start" main/web_server.c` passed.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Pump config API helpers are ready for reuse by the runtime status/start/stop routes in Plan 03-02.

## Self-Check: PASSED

Plan-level verification commands passed and requirements API-01, API-02, and API-06 are covered by server-side API code.

---
*Phase: 03-authenticated-pump-control-api*
*Completed: 2026-05-20*
