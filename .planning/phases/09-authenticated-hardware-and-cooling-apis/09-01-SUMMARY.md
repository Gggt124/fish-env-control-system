---
phase: 09-authenticated-hardware-and-cooling-apis
plan: 01
subsystem: firmware-api
tags: [hardware-map, cjson, local-api, auth, pending-config]
requires:
  - phase: 06-hardware-contract-and-persistent-map-foundation
    provides: hardware_map safe role options and active/pending NVS storage
provides:
  - authenticated GET /api/hardware/map
  - authenticated same-origin POST /api/hardware/map
  - safe GPIO option serialization grouped by hardware role
  - pending hardware map save/clear behavior with reboot-required status
affects: [phase-09, phase-10, hardware-install-ui]
tech-stack:
  added: []
  patterns: [authenticated GET route, same-origin POST route, pending reboot-required config]
key-files:
  created: []
  modified:
    - components/app_config/app_config.h
    - main/web_server.c
key-decisions:
  - "Hardware GPIO updates save only to the pending hardware map; active runtime pins are not changed by /api/hardware/map."
  - "The hardware map POST requires confirm_reboot_required:true before saving pending wiring changes."
  - "If the submitted hardware map matches the active map, pending hardware settings are cleared."
requirements-completed: [UI-03, UI-04, UI-05, UI-06, UI-07]
duration: same-session
completed: 2026-05-24
---

# Phase 9 Plan 01 Summary

**Authenticated hardware map APIs now expose active/pending GPIO state and safe dropdown options**

## Performance

- **Duration:** same session
- **Completed:** 2026-05-24
- **Tasks:** 4
- **Files modified:** 2

## Accomplishments

- Increased HTTP route capacity from 20 to 32 handlers for the expanded Phase 9 API surface.
- Added hardware map JSON serialization for active and pending GPIO maps.
- Added safe GPIO options grouped by role for float input, pump Relay 1, pump Relay 2, DS18B20 data, and cooling relay.
- Added authenticated `GET /api/hardware/map` returning active map, pending map, pending status, reboot-required state, and safe options.
- Added authenticated same-origin `POST /api/hardware/map` with server-side `confirm_reboot_required:true`, role-specific safe GPIO validation, complete-map validation, pending NVS save, and pending clear when submitted values match active values.

## Task Commits

1. **Tasks 1-4: hardware map API and route capacity** - `9440a60` (`feat(09-01): add authenticated hardware map API`)

## Verification

- `rg "APP_TEMPLATE_HTTP_MAX_URI_HANDLERS|hardware_map.*json|pending_hardware|float_input_gpio|ds18b20_gpio" components/app_config/app_config.h main/web_server.c`
- `rg "hardware_map_options_for_role|input_capable|output_capable|internal_pull_capable|is_default|options" main/web_server.c`
- `rg "/api/hardware/map|handle_api_hardware_map|nvs_store_load_hardware_map|nvs_store_load_pending_hardware_map|nvs_store_hardware_reboot_required" main/web_server.c`
- `rg "confirm_reboot_required|hardware_map_gpio_allowed_for_role|hardware_map_validate|nvs_store_save_pending_hardware_map|nvs_store_clear_pending_hardware_map|is_same_origin\\(req, false\\)" main/web_server.c`
- Inspected hardware map POST flow to confirm it calls pending save/clear only and does not apply runtime pin changes.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None.

## User Setup Required

None.

## Next Phase Readiness

Plan 09-02 can add cooling configuration and mode APIs on top of the expanded route capacity and existing cooling runtime.

## Self-Check: PASSED

- Active and pending hardware maps serialize with stable field names.
- Safe GPIO option arrays come from `hardware_map_options_for_role()`.
- GPIO POST parsing validates role-specific allowed values and the complete map.
- `confirm_reboot_required:true` is mandatory.
- Mutation route uses existing session auth and strict same-origin protection.

---
*Phase: 09-authenticated-hardware-and-cooling-apis*
*Completed: 2026-05-24*
