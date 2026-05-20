---
phase: 04-web-pump-control-ui
plan: 03
subsystem: ui
tags: [pump-status, countdown, stale-state, esp-idf-build]
requires:
  - phase: 04-web-pump-control-ui
    provides: dashboard DOM and pump API wiring from 04-01 and 04-02
  - phase: 03-authenticated-pump-control-api
    provides: authenticated /api/pump/status and status snapshots in Start/Stop responses
provides:
  - Thai pump status rendering for running, float, relay, timer, and phase states
  - Two-second status polling while visible
  - One-second local countdown ticking from device snapshots
  - Degraded and stale runtime status handling
  - ESP-IDF build evidence with embedded frontend assets
affects: [phase-04-verification, phase-05-hardware-validation, dashboard]
tech-stack:
  added: []
  patterns: [authoritative-device-sync, local-countdown-estimate, stale-runtime-gate]
key-files:
  created:
    - build/fish_pump_relay_timer_control.bin
  modified:
    - main/static/app.js
    - main/static/dashboard.html
    - main/static/style.css
key-decisions:
  - "Device pump status snapshots always overwrite local countdown estimates."
  - "Runtime Start/Stop controls disable when status is stale for more than 5 seconds."
  - "Hardware validation remains deferred to Phase 5."
patterns-established:
  - "Dashboard polling stops while hidden and immediately resyncs on visibility restore."
  - "Temporary status failures show stale/waiting state without blocking timer config editing."
requirements-completed: [TIME-01, TIME-02, TIME-03, TIME-04, UI-01, UI-02, UI-03, UI-04, UI-05, UI-06, UI-07]
duration: 45 min
completed: 2026-05-20
---

# Phase 04 Plan 03: Live Pump Status Summary

**Live pump dashboard status with Thai operational labels, local countdown ticking, stale handling, and ESP-IDF build validation**

## Performance

- **Duration:** 45 min
- **Started:** 2026-05-20T12:25:00+07:00
- **Completed:** 2026-05-20T12:40:55+07:00
- **Tasks:** 5
- **Files modified:** 4

## Accomplishments

- Rendered machine-friendly pump status into Thai labels for controller running state, binary float switch, relay output, active timer, and timer phase.
- Added `/api/pump/status` polling every 2 seconds while the dashboard is visible.
- Added one-second local countdown ticking from the latest successful device snapshot.
- Added visibility handling so hidden tabs stop local timers and visible restore triggers immediate sync.
- Added stale/degraded runtime state handling that disables Start/Stop after more than 5 seconds without successful sync while keeping config editing available.
- Validated that the embedded frontend builds into `build/fish_pump_relay_timer_control.bin`.

## Task Commits

1. **Tasks 1-3: Status rendering, polling, countdown, and degraded states** - `ce11572` (feat)
2. **Task 4: Static checks and ESP-IDF build validation** - build evidence recorded in this summary.
3. **Task 5: Requirement coverage summary** - this file.

## Files Created/Modified

- `main/static/app.js` - Live status polling, countdown ticking, status label rendering, stale detection, visibility handling, and quick status preservation.
- `main/static/dashboard.html` - Runtime IDs and initially disabled Start/Stop controls.
- `main/static/style.css` - Runtime, timer, alert, stale, disabled, and compact status styling.
- `build/fish_pump_relay_timer_control.bin` - Generated firmware binary from `.\scripts\build.ps1`.

## Decisions Made

- Kept local countdown as an estimate only; `/api/pump/status` remains authoritative.
- Used `ระบบปั๊ม`, `ลูกลอย`, and `รีเลย์` wording to avoid conflating controller state with relay output or treating the float as analog water level.
- Deferred hardware/manual validation to Phase 5 as planned.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- Automated visual screenshot testing was not available because the local Node runtime did not have Playwright installed. Static frontend checks, JavaScript syntax validation, and ESP-IDF build validation were completed instead.
- The ESP-IDF build log included upstream component validation warnings and a non-fatal `fatal: Needed a single revision` message while the command still returned success and generated the binary.

## Verification

- `node --check main/static/app.js` passed.
- `rg "/api/pump/config|/api/pump/status|/api/pump/start|/api/pump/stop" main/static/app.js` passed.
- `rg "timer1-on-min|timer1-on-sec|timer1-off-min|timer1-off-sec|timer2-on-min|timer2-on-sec|timer2-off-min|timer2-off-sec" main/static/dashboard.html` passed.
- `rg "ลูกลอย|รีเลย์|ช่วงเปิด|ช่วงปิด|countdown_sec|MM:SS" main/static/app.js main/static/dashboard.html` passed through Thai label and countdown field matches.
- `rg "http://|https://|cdn|Tailwind|Google Fonts|Material Symbols" main/static` returned no matches.
- `rg "float_gpio|relay_gpio|debounce_ms" main/static/dashboard.html` returned no matches.
- `rg "0-100|water level|ระดับน้ำ.*%" main/static` returned no matches.
- `.\scripts\build.ps1` passed and generated `build/fish_pump_relay_timer_control.bin`.
- `Test-Path build\fish_pump_relay_timer_control.bin` returned `True`.

## Requirement Coverage

- **TIME-01:** Timer 1 ON is editable through minute/second fields and saved as `timer1_on_sec`.
- **TIME-02:** Timer 1 OFF is editable through minute/second fields and saved as `timer1_off_sec`.
- **TIME-03:** Timer 2 ON is editable through minute/second fields and saved as `timer2_on_sec`.
- **TIME-04:** Timer 2 OFF is editable through minute/second fields and saved as `timer2_off_sec`.
- **UI-01:** Dashboard exposes Timer 1/Timer 2 ON/OFF settings.
- **UI-02:** Dashboard saves full replacement pump config through the authenticated API.
- **UI-03:** Dashboard provides separate Start and Stop actions.
- **UI-04:** Dashboard displays active timer, phase, and countdown.
- **UI-05:** Dashboard displays binary float switch state tied to Timer 1/Timer 2.
- **UI-06:** Dashboard displays relay ON/OFF with operator wording.
- **UI-07:** Static frontend remains local-only with no CDN or external assets.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Phase 4 is build-valid and ready for verifier review. Phase 5 should handle hardware flash/manual validation against the ESP32 DevKit V1, float switch, and relay module.

---
*Phase: 04-web-pump-control-ui*
*Completed: 2026-05-20*
