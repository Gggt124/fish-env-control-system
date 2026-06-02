---
phase: 10-owner-dashboard-hardware-install-ui-and-validation
plan: 02
subsystem: frontend-hardware-install
tags: [hardware-install, gpio-map, pending-map, protected-route]
requires:
  - phase: 09-01
    provides: authenticated hardware map active/pending/options API
  - phase: 10-01
    provides: dashboard navigation link to Hardware/Install
provides:
  - protected /hardware static page route
  - embedded hardware.html asset
  - wiring-first Hardware/Install page
  - safe GPIO dropdown edit flow with reboot confirmation
affects: [ui-02, hardware-map, web-server]
tech-stack:
  added: []
  patterns: [esp-idf-embed-files, filename-only-binary-symbols, dropdowns-from-api-options]
key-files:
  created:
    - main/static/hardware.html
  modified:
    - main/CMakeLists.txt
    - main/web_server.c
    - main/static/dashboard.html
    - main/static/status.html
    - main/static/wifi.html
    - main/static/app.js
    - main/static/style.css
key-decisions:
  - "The hardware page uses _binary_hardware_html_* symbols, matching ESP-IDF filename-only EMBED_FILES naming."
  - "GPIO editors are select dropdowns populated from firmware safe option arrays; there is no manual numeric GPIO entry."
  - "Pending GPIOs are selected for editing when present, while active GPIOs stay separately visible."
requirements-implemented: [UI-02]
requirements-completed: [UI-02]
duration: same-session
completed: 2026-05-24
---

# Phase 10 Plan 02 Summary

**Protected Hardware/Install page and pending GPIO flow are implemented**

## Performance

- **Duration:** same session
- **Completed:** 2026-05-24
- **Tasks:** 4
- **Files modified:** 8
- **Files created:** 1

## Accomplishments

- Added `main/static/hardware.html` and embedded it through `main/CMakeLists.txt`.
- Added `_binary_hardware_html_start/end`, `handle_get_hardware()`, and a protected `GET /hardware` route.
- Added Hardware/Install navigation to dashboard, status, and Wi-Fi pages.
- Built a wiring-first page layout with active GPIO summary, pending reboot summary, safe dropdown editors, reboot confirmation, and secondary technical pinout metadata.
- Wired `loadHardwareMap()` and `saveHardwareMap()` against `/api/hardware/map`, including `confirm_reboot_required: true` and dirty-state save gating.
- Rendered technical pin capability flags from API metadata: `input_capable`, `output_capable`, `internal_pull_capable`, and `is_default`.

## Task Commits

1. **Tasks 1-4: Hardware/Install route and UI** - included in final Phase 10 execution commit.

## Verification

- `rg "hardware.html|_binary_hardware_html_start|_binary_hardware_html_end|handle_get_hardware|/hardware" main/CMakeLists.txt main/web_server.c main/static` passed.
- `rg "hardware-wiring|hardware-reboot-banner|hardware-active-summary|hardware-pending-summary|hardware-map-form|hardware-confirm-reboot|hardware-save-map|technical-pinout" main/static/hardware.html main/static/style.css` passed.
- `rg "initHardwareInstall|loadHardwareMap|/api/hardware/map|confirm_reboot_required|pending_valid|reboot_required|hardware-confirm-reboot|hardware-save-map" main/static/app.js main/static/hardware.html` passed.
- `rg "input_capable|output_capable|internal_pull_capable|is_default" main/static/app.js main/static/hardware.html` passed.
- `node --check main/static/app.js` passed.

## Deviations from Plan

- Browser screenshot verification was not run because browser automation was unavailable in this session.

## Issues Encountered

- The initial patch missed local CMake formatting and was reapplied in smaller patches after reading the file.

## User Setup Required

None for source/build validation. Manual hardware map reboot validation still requires a flashed ESP32.

## Next Phase Readiness

Plan 03 can validate embed symbols through ESP-IDF build and document the completed owner/installer surfaces.

## Self-Check: PASSED

- `/hardware` is session-protected.
- `hardware.html` is embedded with filename-only binary symbols.
- GPIO choices come from API option arrays.
- Pending values and reboot-required state are visible.
- Save is blocked until the reboot confirmation checkbox is checked.

---
*Phase: 10-owner-dashboard-hardware-install-ui-and-validation*
*Completed: 2026-05-24*
