---
phase: 07-dual-timer-relay-pump-runtime
plan: 01
subsystem: firmware-runtime
tags: [esp-idf, pump-control, gpio, relay, debounce]
requires:
  - phase: 06-hardware-contract-and-persistent-map-foundation
    provides: active hardware map defaults and relay/timer persistence contract
provides:
  - Dual-relay `pump_control` runtime contract
  - Debounced Float ON/OFF channel selection
  - Relay fail-safe and stopped-state preview semantics
affects: [pump_control, web_server, dashboard, hardware-validation]
tech-stack:
  added: []
  patterns: [additive C struct migration, both-relays-off fail-safe helper]
key-files:
  created: []
  modified:
    - components/pump_control/pump_control.h
    - components/pump_control/pump_control.c
key-decisions:
  - "Float ON selects Timer 1 / Relay 1; Float OFF selects Timer 2 / Relay 2."
  - "Stopped runtime continues debouncing float and previews ready channel while both relays stay OFF."
  - "Runtime faults force both relays OFF and expose status.fault/config_valid=false."
patterns-established:
  - "Relay writes go through one polarity-aware helper and one both-relays-off fail-safe path."
  - "Compatibility fields remain while new per-relay fields are added."
requirements-completed: [PUMP-01, PUMP-02, PUMP-03, PUMP-04, PUMP-05, PUMP-06]
duration: same-session
completed: 2026-05-23
---

# Phase 7 Plan 01 Summary

**Dual-relay ESP-IDF pump runtime with debounced float selection, start phases, stopped preview, and fail-safe relay writes**

## Performance

- **Duration:** same session
- **Completed:** 2026-05-23
- **Tasks:** 4
- **Files modified:** 2

## Accomplishments
- Added Relay 1/Relay 2 GPIO, polarity, energized status, `active_relay`, `fault`, and per-timer start phase fields.
- Implemented confirmed Float ON -> Timer 1 / Relay 1 and Float OFF -> Timer 2 / Relay 2 selection.
- Added stopped preview so dashboard can show float/ready channel before Start while both relays remain OFF.
- Added fail-safe paths that force both relays inactive on invalid config, stop, write failure, or fault.

## Task Commits

1. **Core runtime:** `0d69deb` (`feat(07-01): add dual relay pump runtime core`)

## Verification
- `rg` checks for public dual-relay fields, start phases, GPIO write paths, and stopped-state preview passed.
- `.\scripts\build.ps1` passed and generated `build\fish_pump_relay_timer_control.bin`.

## Deviations from Plan

None - plan executed as written.

## User Setup Required

Manual hardware validation after flashing is still required to confirm Relay 1/Relay 2 physical wiring and no-overlap behavior.

## Next Phase Readiness

Boot/API integration can consume the additive runtime fields without breaking old compatibility fields.

---
*Phase: 07-dual-timer-relay-pump-runtime*
*Completed: 2026-05-23*
