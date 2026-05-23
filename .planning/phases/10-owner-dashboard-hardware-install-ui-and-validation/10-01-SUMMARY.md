---
phase: 10-owner-dashboard-hardware-install-ui-and-validation
plan: 01
subsystem: frontend-dashboard
tags: [dashboard, cooling-ui, owner-operations, offline-ui]
requires:
  - phase: 09-02
    provides: authenticated cooling config and runtime mode APIs
  - phase: 09-03
    provides: dual-channel pump config/status API fields
provides:
  - owner-facing cooling operation section on /dashboard
  - cooling config load/save wiring
  - runtime Auto, Force OFF, and Test ON controls
  - stale/fault-safe cooling status rendering
affects: [ui-01, dashboard, cooling-control]
tech-stack:
  added: []
  patterns: [vanilla-xhr, embedded-static-assets, full-replacement-config-post]
key-files:
  created: []
  modified:
    - main/static/dashboard.html
    - main/static/app.js
    - main/static/style.css
key-decisions:
  - "Cooling controls live on the daily dashboard, but GPIO editing remains linked out to Hardware/Install."
  - "Cooling Test ON is routed only through /api/cooling/mode and is not persisted through /api/cooling/config."
  - "Cooling config save sends a full backend-required payload while preserving loaded min-off and relay polarity fields."
requirements-implemented: [UI-01]
duration: same-session
completed: 2026-05-24
---

# Phase 10 Plan 01 Summary

**Owner dashboard cooling operation surface is implemented**

## Performance

- **Duration:** same session
- **Completed:** 2026-05-24
- **Tasks:** 4
- **Files modified:** 3

## Accomplishments

- Added a cooling runtime section to `/dashboard` with temperature, relay state, mode, sensor state, fault state, threshold, hysteresis, auto-enable, blocked reason, lockout, and Test ON countdown targets.
- Added Auto, Force OFF, and Test ON runtime buttons. Test ON is bounded and uses `/api/cooling/mode`.
- Added a cooling config form for threshold, hysteresis, boot auto-enable, and Test ON timeout.
- Wired `/api/cooling/config`, `/api/cooling/status`, and `/api/cooling/mode` through the existing vanilla JavaScript transport.
- Preserved existing pump countdown and system/Wi-Fi quick status behavior, and added a secondary Hardware/Install link.

## Task Commits

1. **Tasks 1-4: dashboard cooling operation UI** - included in final Phase 10 execution commit.

## Verification

- `rg "cooling-temperature|cooling-relay-state|cooling-sensor-state|cooling-auto-enable|cooling-mode-auto|cooling-mode-force-off|cooling-test-on|/hardware" main/static/dashboard.html main/static/style.css` passed.
- `rg "/api/cooling/config|/api/cooling/status|/api/cooling/mode|test_on|threshold_c_x10|hysteresis_c_x10|cooling_relay_polarity" main/static/app.js` passed.
- `rg "/status|/wifi|/hardware|dash-chip|dash-heap-pct|dash-rssi|dash-ap-clients" main/static/dashboard.html main/static/app.js` passed.
- `rg "http://|https://|cdn|Tailwind|Google Fonts|Material Symbols" main/static` returned no matches.
- `node --check main/static/app.js` passed.

## Deviations from Plan

- Browser screenshot verification was not run because the in-app Browser tools were not exposed in this session and the local Node kernel did not have Playwright installed.

## Issues Encountered

None in implementation. Visual QA remains pending.

## User Setup Required

None for source/build validation. Hardware UAT is covered by Plan 03.

## Next Phase Readiness

Plan 02 can consume the existing `/api/hardware/map` API and link from the dashboard without adding more dashboard GPIO controls.

## Self-Check: PASSED

- UI-01 fields are represented in dashboard markup and JavaScript.
- Test ON is runtime-only.
- Cooling config save preserves required backend fields.
- Static assets remain offline-capable.

---
*Phase: 10-owner-dashboard-hardware-install-ui-and-validation*
*Completed: 2026-05-24*
