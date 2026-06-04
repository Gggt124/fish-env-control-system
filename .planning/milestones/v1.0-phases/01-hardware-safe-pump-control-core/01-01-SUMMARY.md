---
phase: 01-hardware-safe-pump-control-core
plan: 01
subsystem: firmware-config
tags: [esp-idf, gpio, relay, timer, debounce]
requires: []
provides:
  - Pump control GPIO defaults
  - Relay and float polarity defaults
  - Timer default and validation constants
  - Float debounce duration constant
affects: [pump_control, hardware-safe-pump-control-core]
tech-stack:
  added: []
  patterns:
    - Centralized firmware defaults in app_config.h
key-files:
  created: []
  modified:
    - components/app_config/app_config.h
key-decisions:
  - "GPIO32 is the default active-low float input."
  - "GPIO26 is the default active-low relay output."
  - "Timer defaults and anti-chatter bounds are centralized in app_config.h."
patterns-established:
  - "Pump control constants use the existing APP_TEMPLATE_ prefix in the shared app_config boundary."
requirements-completed:
  - HW-02
  - HW-03
  - HW-05
  - HW-06
  - TIME-05
  - TIME-06
  - TIME-07
duration: 2 min
completed: 2026-05-19
---

# Phase 01 Plan 01: Pump Defaults Summary

**Pump GPIO, relay polarity, timer default, timer bounds, and debounce constants centralized in app_config.h**

## Performance

- **Duration:** 2 min
- **Started:** 2026-05-19T00:00:00Z
- **Completed:** 2026-05-19T00:02:00Z
- **Tasks:** 2
- **Files modified:** 1

## Accomplishments

- Added GPIO32 active-low float switch defaults and GPIO26 active-low relay defaults.
- Added Timer 1 and Timer 2 product defaults from the phase context.
- Added timer min/max bounds and 100 ms float debounce duration for the pump component to consume.

## Task Commits

Each task was committed atomically:

1. **Task 1: Add pump hardware constants** - `3aa430e` (feat)
2. **Task 2: Add timer and debounce constants** - `3aa430e` (feat)

**Plan metadata:** pending docs commit

## Files Created/Modified

- `components/app_config/app_config.h` - Defines pump GPIO, polarity, timer, debounce, and validation constants.

## Decisions Made

None - followed plan as specified.

## Deviations from Plan

None - plan executed exactly as written.

**Total deviations:** 0 auto-fixed.
**Impact:** No scope creep; later pump-control component work can consume the defaults directly.

## Issues Encountered

None.

## User Setup Required

None - no external service configuration required.

## Verification

- `rg "APP_TEMPLATE_PUMP_(FLOAT_GPIO|RELAY_GPIO|RELAY_ACTIVE_LOW|FLOAT_ACTIVE_LOW)" components/app_config/app_config.h` passed.
- `rg "APP_TEMPLATE_PUMP_TIMER|APP_TEMPLATE_PUMP_FLOAT_DEBOUNCE_MS" components/app_config/app_config.h` passed.
- `rg "APP_TEMPLATE_PUMP_TIMER1_ON_SEC\s+20" components/app_config/app_config.h` passed.
- `rg "APP_TEMPLATE_PUMP_TIMER2_OFF_SEC\s+180" components/app_config/app_config.h` passed.
- `rg "APP_TEMPLATE_PUMP_TIMER_MIN_SEC\s+5" components/app_config/app_config.h` passed.
- `rg "APP_TEMPLATE_PUMP_FLOAT_DEBOUNCE_MS\s+100" components/app_config/app_config.h` passed.

## Next Phase Readiness

Plan 01-02 can create `components/pump_control/` and derive its default config from these constants.

---
*Phase: 01-hardware-safe-pump-control-core*
*Completed: 2026-05-19*
