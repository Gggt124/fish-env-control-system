---
phase: 03-authenticated-pump-control-api
plan: 02
subsystem: api
tags: [esp-idf, http-server, pump-control, runtime-status]
requires:
  - phase: 03-01
    provides: pump config API helpers and settings conversion patterns
provides:
  - Authenticated pump runtime status handler
  - Authenticated idempotent pump start handler
  - Authenticated idempotent pump stop handler
  - Shared machine-friendly pump status serializer
affects: [phase-04-web-ui, pump-api, runtime-control]
tech-stack:
  added: []
  patterns: [stable enum strings, nested status snapshots, fail-safe start failure handling]
key-files:
  created:
    - .planning/phases/03-authenticated-pump-control-api/03-02-SUMMARY.md
  modified:
    - main/web_server.c
key-decisions:
  - "Pump runtime status lives in dedicated pump handlers and does not extend the existing system /api/status response."
  - "Start avoids calling pump_control_start() when already running so countdown and timer state are not reset."
  - "Stop confirms stopped and relay inactive state before returning success."
patterns-established:
  - "Pump status enum values are stable lowercase strings for UI parsing."
  - "Runtime action responses include idempotency flags plus a status snapshot."
requirements-completed: [RUN-01, RUN-02, API-03, API-04, API-05, API-06]
duration: 18 min
completed: 2026-05-20
---

# Phase 03 Plan 02: Pump Runtime API Summary

**Authenticated runtime status, start, and stop handlers with stable pump status snapshots**

## Performance

- **Duration:** 18 min
- **Started:** 2026-05-20T00:22:00Z
- **Completed:** 2026-05-20T00:40:00Z
- **Tasks:** 4 completed
- **Files modified:** 1

## Accomplishments

- Added a shared status serializer for booleans, GPIO numbers, seconds, and stable lowercase enum strings.
- Added `GET /api/pump/status` handler without changing the existing system/Wi-Fi `/api/status`.
- Added idempotent start/stop handlers with auth, same-origin checks, status snapshots, and stable error codes.

## Task Commits

1. **Task 1: Add pump status serialization helper** - `1a39bfe` (feat)
2. **Tasks 2-4: Add status/start/stop handlers** - `a4c0f35` (feat)

## Files Created/Modified

- `main/web_server.c` - Pump runtime status serializer and authenticated status/start/stop handlers.

## Decisions Made

- Status responses include `auto_start` and `settings_status` from NVS, but do not duplicate full timer duration config.
- Start returns `already_running:true` without resetting runtime state.
- Stop returns `already_stopped:true` for an already stopped inactive relay and verifies inactive relay state after stop.

## Deviations from Plan

The three route handlers were committed together because they share the status/error response helpers. No behavioral scope was added beyond the plan.

**Total deviations:** 0 auto-fixed.
**Impact:** No scope creep; commit granularity is coarser than individual handler tasks.

## Issues Encountered

None.

## Verification

- `rg "handle_api_pump_status|handle_api_pump_start|handle_api_pump_stop" main/web_server.c` passed.
- `rg "float_state|active_timer|phase|countdown_sec|relay_energized|auto_start|settings_status" main/web_server.c` passed.
- `rg "already_running|already_stopped|start_failed|stop_failed" main/web_server.c` passed.
- `rg "is_same_origin\\(req\\)" main/web_server.c` passed.
- Existing `/api/status` handler remains system/Wi-Fi focused and was not extended with pump fields.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

All Phase 3 handlers exist and are ready for route registration plus firmware build validation in Plan 03-03.

## Self-Check: PASSED

Plan-level verification commands passed and requirements RUN-01, RUN-02, API-03, API-04, API-05, and API-06 are covered by server-side API code.

---
*Phase: 03-authenticated-pump-control-api*
*Completed: 2026-05-20*
