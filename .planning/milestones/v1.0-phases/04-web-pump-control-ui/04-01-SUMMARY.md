---
phase: 04-web-pump-control-ui
plan: 01
subsystem: ui
tags: [embedded-frontend, dashboard, pump-control, css]
requires:
  - phase: 03-authenticated-pump-control-api
    provides: authenticated pump API routes consumed by the dashboard
provides:
  - Pump-first dashboard runtime panel
  - Timer 1 and Timer 2 minute/second settings form
  - Compact Wi-Fi and system quick-status section
affects: [phase-04, phase-05-hardware-validation, dashboard]
tech-stack:
  added: []
  patterns: [static-html-dashboard, no-cdn-css, stable-dom-ids]
key-files:
  created: []
  modified:
    - main/static/dashboard.html
    - main/static/style.css
key-decisions:
  - "Pump runtime state and countdown are first-viewport priority."
  - "Timer fields are split into minute and second inputs while GPIO/debounce settings remain hidden."
patterns-established:
  - "Runtime panel IDs are stable API targets for app.js wiring."
  - "System/Wi-Fi information is compact and secondary on the pump dashboard."
requirements-completed: [UI-01, UI-03, UI-04, UI-05, UI-06, UI-07]
duration: 15 min
completed: 2026-05-20
---

# Phase 04 Plan 01: Pump Dashboard Structure Summary

**Pump-first dashboard structure with runtime controls, timer settings, and compact system status**

## Performance

- **Duration:** 15 min
- **Started:** 2026-05-20T12:25:00+07:00
- **Completed:** 2026-05-20T12:40:55+07:00
- **Tasks:** 3
- **Files modified:** 2

## Accomplishments

- Replaced the old Wi-Fi/system-first dashboard cards with a pump runtime panel.
- Added stable DOM IDs for running state, countdown, active timer, phase, relay, float, sync state, errors, Start, and Stop.
- Added Timer 1 and Timer 2 ON/OFF minute/second inputs plus auto-start and unsaved/save feedback placeholders.
- Preserved `/status`, `/wifi`, logout navigation, stylesheet, script include, and compact system quick status.

## Task Commits

1. **Tasks 1-3: Dashboard runtime, settings, and quick status layout** - `0eac134` (feat)

## Files Created/Modified

- `main/static/dashboard.html` - Pump runtime panel, timer form, auto-start control, and compact quick status.
- `main/static/style.css` - Responsive pump runtime, timer form, validation, stale/error, and quick-link styling.

## Decisions Made

- Used a dense operational dashboard layout rather than a decorative card mosaic.
- Kept Start and Stop as separate buttons in the top runtime panel.
- Kept binary float wording tied to Timer 1/Timer 2 selection.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None.

## Verification

- `rg "pump-runtime|pump-countdown|pump-start-btn|pump-stop-btn|pump-float-state|pump-relay-state" main/static/dashboard.html main/static/style.css` passed.
- `rg "timer1-on-min|timer1-off-sec|timer2-on-min|timer2-off-sec|pump-auto-start|pump-save-config|unsaved" main/static/dashboard.html` passed.
- `rg "Relay control and scheduling will be added" main/static/dashboard.html` returned no matches.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

The dashboard DOM is ready for pump API wiring in Plan 04-02.

---
*Phase: 04-web-pump-control-ui*
*Completed: 2026-05-20*
