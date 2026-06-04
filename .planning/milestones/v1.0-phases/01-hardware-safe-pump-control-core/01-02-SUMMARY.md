---
phase: 01-hardware-safe-pump-control-core
plan: 02
subsystem: firmware-component
tags: [esp-idf, gpio, relay, timer, debounce, freertos]
requires:
  - phase: 01-01
    provides: Pump control defaults in app_config.h
provides:
  - Reusable pump_control component
  - Hardware-safe relay initialization
  - Binary float debounce and timer selection
  - ON/OFF timer phase state machine
  - Pump control status API
affects: [pump_control, boot-integration, pump-api, pump-ui]
tech-stack:
  added: []
  patterns:
    - ESP-IDF component with one public header and one implementation file
    - FreeRTOS mutex protected file-local component state
    - esp_timer periodic callback for debounce and phase checks
key-files:
  created:
    - components/pump_control/CMakeLists.txt
    - components/pump_control/pump_control.h
    - components/pump_control/pump_control.c
  modified: []
key-decisions:
  - "The relay inactive level is derived from configured polarity and written before and after output GPIO configuration."
  - "Pump start debounces the float input before selecting the first timer, keeping the relay inactive during stabilization."
  - "Invalid timer or GPIO config rejects the whole config instead of clamping values."
patterns-established:
  - "Pump runtime state is hidden behind pump_control_get_status rather than exposing internal pointers."
requirements-completed:
  - HW-02
  - HW-04
  - HW-06
  - TIME-05
  - TIME-06
  - TIME-07
  - RUN-03
  - RUN-06
  - RUN-07
  - RUN-08
  - RUN-09
  - RUN-10
  - RUN-11
duration: 12 min
completed: 2026-05-19
---

# Phase 01 Plan 02: Pump Control Component Summary

**Reusable ESP-IDF pump_control component with safe relay init, binary float debounce, timer selection, and ON/OFF relay phases**

## Performance

- **Duration:** 12 min
- **Started:** 2026-05-19T00:02:00Z
- **Completed:** 2026-05-19T00:14:00Z
- **Tasks:** 3
- **Files modified:** 3

## Accomplishments

- Added `components/pump_control/` as a reusable ESP-IDF component.
- Exposed a small public API for default config, init, start, stop, and status reads.
- Implemented fail-safe relay initialization, timer duration validation, float debounce, timer switching, ON/OFF phase transitions, and stop behavior.

## Task Commits

Each task was committed atomically:

1. **Task 1: Add component registration and public API** - `a50fbde` (feat)
2. **Task 2: Implement safe GPIO init and config validation** - `61128a9` (feat)
3. **Task 3: Implement debounce, timer state machine, start/stop, and status** - `61128a9` (feat)

**Plan metadata:** pending docs commit

## Files Created/Modified

- `components/pump_control/CMakeLists.txt` - Registers the component and ESP-IDF dependencies.
- `components/pump_control/pump_control.h` - Defines the public config/status types and five core functions.
- `components/pump_control/pump_control.c` - Implements validation, GPIO setup, relay control, debounce, timer state, start/stop, and status copy.

## Decisions Made

None - followed plan as specified.

## Deviations from Plan

None - plan executed exactly as written.

**Total deviations:** 0 auto-fixed.
**Impact:** No scope creep; web API, UI, NVS persistence, auto-start, and hardware testing remain deferred.

## Issues Encountered

None.

## User Setup Required

None - no external service configuration required.

## Verification

- `rg "pump_control_(default_config|init|start|stop|get_status)" components/pump_control/pump_control.h` passed.
- `rg "REQUIRES app_config driver esp_timer freertos" components/pump_control/CMakeLists.txt` passed.
- `rg "gpio_set_level|gpio_config|APP_TEMPLATE_PUMP_TIMER_MIN_SEC|APP_TEMPLATE_PUMP_TIMER_MAX_SEC" components/pump_control/pump_control.c` passed.
- `rg "esp_timer_start_periodic|PUMP_CONTROL_TIMER_1|PUMP_CONTROL_PHASE_ON|pump_control_stop|pump_control_get_status" components/pump_control` passed.
- `rg "idf_component_register" components/pump_control/CMakeLists.txt` passed.
- `idf.py build` is deferred to plan 01-03, where the component is wired into the firmware.

## Next Phase Readiness

Plan 01-03 can add `pump_control` to `main` dependencies and call `pump_control_init(pump_control_default_config())` during boot without starting the pump.

---
*Phase: 01-hardware-safe-pump-control-core*
*Completed: 2026-05-19*
