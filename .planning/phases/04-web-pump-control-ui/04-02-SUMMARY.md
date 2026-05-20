---
phase: 04-web-pump-control-ui
plan: 02
subsystem: ui
tags: [vanilla-js, pump-api, validation, dashboard]
requires:
  - phase: 04-web-pump-control-ui
    provides: stable pump dashboard DOM IDs from 04-01
  - phase: 03-authenticated-pump-control-api
    provides: authenticated /api/pump/config, /api/pump/start, and /api/pump/stop
provides:
  - Pump configuration load and save flow
  - Timer minute/second validation and seconds-only API payload mapping
  - Unsaved-change tracking
  - Pending-safe Start and Stop actions
affects: [phase-04, phase-05-hardware-validation, main/static/app.js]
tech-stack:
  added: []
  patterns: [xhr-api-helpers, dashboard-state, field-level-validation]
key-files:
  created: []
  modified:
    - main/static/app.js
    - main/static/dashboard.html
key-decisions:
  - "The UI preserves loaded relay_polarity while never sending hardware/debounce fields."
  - "Start/Stop remains independent from unsaved timer form edits."
patterns-established:
  - "Pump dashboard state is centralized in app.js and initialized only on /dashboard."
  - "Duration pairs validate locally to 5..86400 seconds before posting the full replacement payload."
requirements-completed: [TIME-01, TIME-02, TIME-03, TIME-04, UI-01, UI-02, UI-03, UI-07]
duration: 35 min
completed: 2026-05-20
---

# Phase 04 Plan 02: Pump Dashboard API Wiring Summary

**Vanilla JavaScript pump settings flow with validation, full replacement saves, unsaved tracking, and Start/Stop actions**

## Performance

- **Duration:** 35 min
- **Started:** 2026-05-20T12:25:00+07:00
- **Completed:** 2026-05-20T12:40:55+07:00
- **Tasks:** 4
- **Files modified:** 2

## Accomplishments

- Added `initPumpDashboard()` for `/dashboard` while preserving status and Wi-Fi page initialization.
- Added `loadPumpConfig()` to populate all eight timer inputs and auto-start from `GET /api/pump/config`.
- Added validation for required integer minute/second fields, second subfields, and 5..86400 second totals.
- Added full replacement `POST /api/pump/config` payloads with timer fields, auto-start, and preserved `relay_polarity`.
- Added unsaved-change warning plus pending-safe `/api/pump/start` and `/api/pump/stop` actions.

## Task Commits

1. **Tasks 1-4: Config, validation, unsaved state, and runtime actions** - `ce11572` (feat)

## Files Created/Modified

- `main/static/app.js` - Pump dashboard state, API calls, validation, save flow, and Start/Stop actions.
- `main/static/dashboard.html` - Initial disabled state for Start/Stop before first status sync.

## Decisions Made

- Preserved `relay_polarity` from the loaded config rather than adding a Phase 4 edit control.
- Did not submit read-only hardware fields in save payloads.
- Kept Start/Stop available according to applied runtime status, not form dirty state.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None.

## Verification

- `node --check main/static/app.js` passed.
- `rg "/api/pump/config|/api/pump/start|/api/pump/stop" main/static/app.js` passed.
- `rg "timer1_on_sec|timer1_off_sec|timer2_on_sec|timer2_off_sec|auto_start|relay_polarity" main/static/app.js` passed.
- `rg "float_gpio|relay_gpio|debounce_ms" main/static/app.js` returned no matches.
- `rg "86400|5" main/static/app.js` passed.
- `rg "unsaved|pending|already_running|already_stopped" main/static/app.js` passed.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

The dashboard can load, edit, save, and apply pump commands against the Phase 3 API. Plan 04-03 live status behavior is ready for verification and build evidence.

---
*Phase: 04-web-pump-control-ui*
*Completed: 2026-05-20*
