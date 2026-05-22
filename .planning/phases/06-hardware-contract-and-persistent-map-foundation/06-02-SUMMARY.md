---
phase: 06-hardware-contract-and-persistent-map-foundation
plan: 02
subsystem: storage
tags: [esp-idf, nvs, hardware-map, cooling]
requires:
  - phase: 06-01
    provides: hardware_map structs, enums, defaults, and validation helpers
provides:
  - Active and pending hardware map persistence APIs
  - Independent pump Relay 1, pump Relay 2, and cooling relay polarity storage
  - Cooling settings persistence defaults and validation
affects: [phase-06, phase-07, phase-08, phase-09]
tech-stack:
  added: []
  patterns: [typed NVS wrappers, fail-closed defaults, pending-versus-active map]
key-files:
  created: []
  modified:
    - components/nvs_store/CMakeLists.txt
    - components/nvs_store/nvs_store.h
    - components/nvs_store/nvs_store.c
    - docs/hardware.md
    - docs/components.md
key-decisions:
  - "Raw NVS keys remain file-local to nvs_store.c."
  - "Pending hardware maps are persisted separately and never become active immediately."
  - "Legacy relay_low remains a Relay 1 compatibility alias."
patterns-established:
  - "Load helpers return safe defaults with explicit status when values are missing or invalid."
requirements-completed: [HW-03, HW-05]
duration: 32 min
completed: 2026-05-22
---

# Phase 06 Plan 02: Persistent Schema Summary

**Validated NVS schemas for active/pending GPIO maps, independent relay polarity, timer start phases, and cooling defaults**

## Performance

- **Duration:** 32 min
- **Started:** 2026-05-22T05:35:00Z
- **Completed:** 2026-05-22T06:07:00Z
- **Tasks:** 5
- **Files modified:** 5

## Accomplishments

- Added `hardware_map` dependency to `nvs_store`.
- Added active hardware map load/save/clear APIs, pending hardware map APIs, and reboot-required detection.
- Extended pump settings with Relay 1 polarity, Relay 2 polarity, Timer 1 start phase, and Timer 2 start phase while preserving existing v1.0 keys.
- Added cooling settings defaults, validation, load/save/clear APIs, and schema documentation.

## Task Commits

Inline execution produced one production commit covering the five storage-schema tasks:

1. **Tasks 1-5: storage contracts, active/pending maps, pump migration, cooling settings, docs** - `29d832a` (feat)

## Files Created/Modified

- `components/nvs_store/nvs_store.h` - Public v1.1 storage structs, statuses, and APIs.
- `components/nvs_store/nvs_store.c` - `hw_cfg`, `pump_cfg`, and `cool_cfg` NVS ownership.
- `components/nvs_store/CMakeLists.txt` - Adds `hardware_map` dependency.
- `docs/hardware.md` - Documents NVS schema and pending map semantics.
- `docs/components.md` - Documents storage namespaces.

## Decisions Made

- Missing active hardware map returns defaults with a missing status; invalid active maps fail closed to defaults with an invalid status.
- Pending hardware maps use `pend_valid` and require explicit activation by reboot/later phase logic.
- Cooling defaults are safe off: mode force-off, auto-enable false, test timeout 10 seconds, minimum off-time 300 seconds.

## Deviations from Plan

None - implementation scope matched the planned storage foundation.

## Issues Encountered

None.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Plan 03 can consume active hardware maps and expanded settings at boot and in read-only API/status surfaces.

## Self-Check: PASSED

- `rg` checks for hardware map APIs, pump migration keys, cooling keys, and schema documentation passed.

---
*Phase: 06-hardware-contract-and-persistent-map-foundation*
*Completed: 2026-05-22*
