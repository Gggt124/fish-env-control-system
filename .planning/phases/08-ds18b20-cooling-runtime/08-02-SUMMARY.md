---
phase: 08-ds18b20-cooling-runtime
plan: 02
subsystem: firmware-runtime
tags: [cooling-relay, hysteresis, compressor-lockout, test-on, esp32-gpio]
requires:
  - phase: 08-ds18b20-cooling-runtime
    provides: cooling sensor status and public runtime contract from Plan 01
provides:
  - cooling relay GPIO setup with polarity-aware inactive/active writes
  - Auto, Force OFF, and Test ON runtime modes
  - threshold/hysteresis relay control
  - compressor minimum off-time lockout and countdown status
affects: [phase-8, phase-9, cooling-api, owner-dashboard, hardware-validation]
tech-stack:
  added: []
  patterns: [safe relay inactive writes before/after gpio_config, runtime-only Test ON, demand versus blocked reason separation]
key-files:
  created: []
  modified:
    - components/cooling_control/cooling_control.h
    - components/cooling_control/cooling_control.c
key-decisions:
  - "Boot/reinit sets the next allowed ON time to now plus compressor_min_off_sec."
  - "Test ON may be requested with sensor unknown/fault but is still blocked by compressor lockout."
  - "Auto cooling demand remains distinct from actual relay output through cooling_demand and blocked_reason."
patterns-established:
  - "Every cooling relay ON-to-OFF transition restarts compressor protection."
  - "Relay status reflects intended successful writes and fails safe after GPIO write failure."
requirements-completed: [COOL-01, COOL-02, COOL-03, COOL-05, COOL-06, COOL-07]
duration: same-session
completed: 2026-05-23
---

# Phase 8 Plan 02 Summary

**Cooling relay state machine with Auto hysteresis, Force OFF, Test ON timeout, and compressor lockout protection**

## Performance

- **Duration:** same session
- **Completed:** 2026-05-23
- **Tasks:** 4
- **Files modified:** 2

## Accomplishments

- Added polarity-aware cooling relay GPIO setup that forces inactive before and after `gpio_config()`.
- Implemented Auto mode ON at threshold and OFF below threshold minus hysteresis.
- Implemented Force OFF and runtime-only Test ON with previous-mode return after timeout.
- Enforced compressor minimum off-time at boot/reinit and after every ON-to-OFF transition.

## Task Commits

1. **Relay/mode/protection runtime:** `83fed92` (`feat(08): add ds18b20 cooling runtime`)

## Verification

- `rg` checks confirmed relay GPIO writes, threshold/hysteresis, mode fields, Test ON countdown, compressor lockout, and blocked reasons.
- Manual source inspection confirmed unknown/fault/config-invalid/lockout paths call inactive relay writes before returning status.
- `.\scripts\build.ps1` passed and generated `build\fish_pump_relay_timer_control.bin`.

## Deviations from Plan

None beyond the Plan 01 scheduling choice; Plan 02 behavior was implemented inside the planned component boundary.

## User Setup Required

Manual relay validation still requires connecting the cooling relay module to the active/default GPIO25 with the configured polarity before flashing a device.

## Next Phase Readiness

Plan 03 can initialize `cooling_control` at boot and expose the mode, demand, block, lockout, and temperature fields through an authenticated status API.

---
*Phase: 08-ds18b20-cooling-runtime*
*Completed: 2026-05-23*
