---
phase: 06-hardware-contract-and-persistent-map-foundation
plan: 01
subsystem: hardware
tags: [esp-idf, gpio, hardware-map, ds18b20]
requires: []
provides:
  - Firmware-owned hardware role contract for Phase 6
  - Safe ESP32 DevKit V1 GPIO defaults and role option lists
  - DS18B20 powered-mode wiring documentation
affects: [phase-06, phase-07, phase-08, phase-09, phase-10]
tech-stack:
  added: [hardware_map component]
  patterns: [role-scoped GPIO validation, typed hardware enums]
key-files:
  created:
    - components/hardware_map/CMakeLists.txt
    - components/hardware_map/hardware_map.h
    - components/hardware_map/hardware_map.c
    - docs/hardware.md
  modified:
    - components/app_config/app_config.h
    - docs/components.md
    - README.md
key-decisions:
  - "GPIO32/GPIO26 remain float and pump Relay 1 defaults for v1.0 compatibility."
  - "Relay 2, DS18B20, and cooling relay defaults are GPIO27, GPIO33, and GPIO25."
  - "DS18B20 uses powered-mode documentation with an external 4.7 kOhm pull-up."
patterns-established:
  - "hardware_map owns role names, safe option metadata, defaults, and validation without runtime side effects."
requirements-completed: [HW-01, HW-02, HW-04]
duration: 25 min
completed: 2026-05-22
---

# Phase 06 Plan 01: Hardware Contract Summary

**Typed ESP32 hardware role contract with safe GPIO defaults and DS18B20 powered-mode wiring documentation**

## Performance

- **Duration:** 25 min
- **Started:** 2026-05-22T05:10:00Z
- **Completed:** 2026-05-22T05:35:00Z
- **Tasks:** 4
- **Files modified:** 7

## Accomplishments

- Added reusable `hardware_map` component with role enums, relay polarity, timer start phase, cooling mode, defaults, option lists, and validation helpers.
- Added Phase 6 default constants for float GPIO32, pump Relay 1 GPIO26, pump Relay 2 GPIO27, DS18B20 GPIO33, cooling relay GPIO25, cooling thresholds, and compressor protection defaults.
- Documented ESP32 DevKit V1 hardware assumptions, relay polarity warning, DS18B20 powered-mode wiring, 4.7 kOhm pull-up, and compressor defaults.

## Task Commits

Inline execution produced one production commit covering the four tightly coupled hardware-contract tasks:

1. **Tasks 1-4: hardware_map component, defaults, validation, and docs** - `de3c320` (feat)

## Files Created/Modified

- `components/hardware_map/hardware_map.h` - Public typed hardware role and validation API.
- `components/hardware_map/hardware_map.c` - Safe GPIO option lists, defaults, names, and validators.
- `components/app_config/app_config.h` - Phase 6 hardware and cooling default constants.
- `docs/hardware.md` - Installer-facing hardware contract.
- `docs/components.md` - Component boundary documentation.
- `README.md` - Link to the hardware contract.

## Decisions Made

- Kept `hardware_map` independent from NVS, pump runtime, and web server code.
- Allowed GPIO34-GPIO39 only for float input metadata with external pull requirements; excluded them from output and DS18B20 options.
- Documented Relay 2, DS18B20, and cooling relay as defaults only; no runtime behavior was added.

## Deviations from Plan

None - implementation scope matched the planned hardware contract.

## Issues Encountered

None.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Plan 02 can use `hardware_map_t`, relay polarity enums, timer start phase enums, cooling mode enums, and `hardware_map_validate()` for persistent schemas.

## Self-Check: PASSED

- `rg` checks for hardware types, defaults, validation helpers, and hardware documentation passed.

---
*Phase: 06-hardware-contract-and-persistent-map-foundation*
*Completed: 2026-05-22*
