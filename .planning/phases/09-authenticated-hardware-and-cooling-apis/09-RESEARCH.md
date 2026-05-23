# Phase 9 Research: Authenticated Hardware And Cooling APIs

**Phase:** 09-authenticated-hardware-and-cooling-apis  
**Date:** 2026-05-24  
**Status:** Complete  
**Context note:** No `09-CONTEXT.md` exists. This research uses `ROADMAP.md`, `REQUIREMENTS.md`, previous phase summaries, and the implemented source contracts.

## Scope Read

Phase 9 exposes the Phase 6 hardware map, Phase 7 dual pump channels, and Phase 8 cooling runtime through authenticated local APIs. It must not build the owner dashboard or Hardware/Install page yet; Phase 10 consumes these APIs for UI.

In scope:

- Active and pending GPIO map reads.
- Safe GPIO enum option reads for each hardware role.
- Pending GPIO map saves with reboot-required status and explicit wiring/reboot confirmation.
- Cooling configuration and runtime mode mutation APIs.
- Pump configuration API support for both relay channels and independent timer start phases.
- Auth and same-origin POST protection on every new mutation route.
- ESP-IDF build validation after route count and handlers change.

Out of scope:

- Dashboard redesign, Hardware/Install page UI, charts, WebSockets, MQTT/cloud, OTA flow, HTTPS, multi-user roles, CSRF token framework, and relay/timer behavior changes beyond applying valid settings through existing runtime APIs.

## Existing Implementation

### HTTP/Auth Patterns

`main/web_server.c` already provides the route and security patterns Phase 9 should reuse:

- `require_auth(req)` protects authenticated APIs.
- `is_same_origin(req, false)` blocks mutation routes unless `Origin` or `Referer` matches the device's AP IP, STA IP, or mDNS host.
- `send_json()` and `send_api_error()` centralize JSON responses.
- cJSON is already used for request parsing and response generation.
- Current route registration has 19 handlers while `APP_TEMPLATE_HTTP_MAX_URI_HANDLERS` is 20, so Phase 9 must raise the route capacity before adding more API routes.

Mutation routes already using the stricter same-origin pattern include `/api/pump/config`, `/api/pump/start`, and `/api/pump/stop`.

### Hardware Map Foundation

Phase 6 provides the safe GPIO contract:

- `hardware_map_t` has `float_input_gpio`, `pump_relay1_gpio`, `pump_relay2_gpio`, `ds18b20_data_gpio`, and `cooling_relay_gpio`.
- `hardware_map_options_for_role()` returns role-specific safe options for dropdown-style UI.
- `hardware_map_gpio_allowed_for_role()` validates a GPIO against a specific role.
- `hardware_map_validate()` validates a complete map, including role-specific safety and no duplicate pins.
- `hardware_map_reboot_required()` detects active-vs-pending differences.
- NVS has active and pending map APIs: `nvs_store_load_hardware_map()`, `nvs_store_save_hardware_map()`, `nvs_store_load_pending_hardware_map()`, `nvs_store_save_pending_hardware_map()`, `nvs_store_clear_pending_hardware_map()`, and `nvs_store_hardware_reboot_required()`.

Recommendation: add a dedicated hardware map API instead of continuing to overload `/api/pump/config` with read-only hardware fields. Keep active map immutable at runtime; POST saves only pending values.

### Pump API State

Phase 7 updated `pump_control` for two relay channels and independent start phases. The runtime status already reports:

- `relay1_energized`, `relay2_energized`
- `relay1_gpio`, `relay2_gpio`
- `active_timer`, `active_relay`
- `timer1_start_phase`, `timer2_start_phase` through persisted settings

`/api/pump/config` currently exposes dual-channel fields, but POST still treats `relay_polarity` as the editable legacy field and mirrors it into Relay 1. Relay 2 polarity remains effectively read-only from the API. Phase 9 should make `relay1_polarity` and `relay2_polarity` editable while preserving legacy `relay_polarity` compatibility for existing UI code.

Hardware GPIO fields must remain read-only in `/api/pump/config`; changing GPIO pins belongs to the pending hardware-map API.

### Cooling Runtime And Storage

Phase 8 provides the cooling runtime:

- `cooling_control_get_status()` returns mode, auto-enable, temperature validity, sensor state/fault, relay state, threshold/hysteresis, lockout, Test ON remaining time, demand, and blocked reason.
- `cooling_control_set_mode()` supports Auto and Force OFF and delegates Test ON to `cooling_control_start_test()`.
- `cooling_control_start_test()` starts bounded runtime-only Test ON.
- `nvs_store_cooling_settings_t` persists threshold, hysteresis, auto-enable, mode, test timeout, compressor min-off, and cooling relay polarity.
- `nvs_store_save_cooling_settings()` validates threshold `-55.0 C..125.0 C`, hysteresis `0.1 C..50.0 C`, test timeout `1..3600` seconds, compressor min-off `0..86400` seconds, valid cooling mode, and valid relay polarity.

Recommendation: add `/api/cooling/config` for persisted cooling settings and `/api/cooling/mode` for runtime mode changes. Persist Auto and Force OFF as safe modes, but never persist Test ON across reboot. Test ON should call `cooling_control_start_test()` and return status showing timeout/lockout behavior.

## Recommended API Shape

### Hardware Map API

`GET /api/hardware/map`

Return:

- `ok`
- `active`: active GPIO role map
- `pending`: pending map if present, otherwise null or active-equivalent object with `pending_valid:false`
- `reboot_required`
- `statuses`: active/pending load status strings
- `options`: safe GPIO option arrays grouped by role, including GPIO value, label, role, capability flags, and default marker

`POST /api/hardware/map`

Accept:

- `confirm_reboot_required: true`
- `map.float_input_gpio`
- `map.pump_relay1_gpio`
- `map.pump_relay2_gpio`
- `map.ds18b20_gpio`
- `map.cooling_relay_gpio`

Behavior:

- Require auth and `is_same_origin(req, false)`.
- Reject missing confirmation before parsing/saving.
- Reject non-number values and values not present in `hardware_map_options_for_role()` for each role.
- Validate the complete map with `hardware_map_validate()`.
- Save pending map with `nvs_store_save_pending_hardware_map()`.
- If posted map equals active map, clear pending map and return `reboot_required:false`.
- Do not call `nvs_store_save_hardware_map()` and do not reinitialize pump/cooling runtime from pending pins.

### Cooling APIs

`GET /api/cooling/config`

Return saved cooling settings plus current status. Include limits/enums for frontend dropdowns and inputs.

`POST /api/cooling/config`

Accept threshold/hysteresis, auto-enable, mode, test timeout, optional compressor min-off, and cooling relay polarity. Validate with the same ranges as `nvs_store_save_cooling_settings()` before saving. Reject persisted `test_on` mode in config; Test ON belongs to the runtime mode route.

After saving, rebuild `cooling_control_config_t` from active hardware map plus persisted settings and reinitialize/apply runtime safely. If runtime apply fails, keep relay OFF and return a JSON error with saved settings/status.

`POST /api/cooling/mode`

Accept `mode: "auto" | "force_off" | "test_on"`. Auto/Force OFF call `cooling_control_set_mode()` and may persist the non-test mode. Test ON calls `cooling_control_start_test()` and must not persist Test ON to NVS. Return current cooling status.

### Pump Config API

Update existing `/api/pump/config`:

- GET should continue to include dual-channel fields.
- POST should accept `relay1_polarity` and `relay2_polarity` independently.
- Legacy `relay_polarity` should remain accepted as a compatibility alias for Relay 1 when explicit `relay1_polarity` is absent.
- POST should continue to accept `timer1_start_phase` and `timer2_start_phase` independently.
- Hardware GPIO fields remain read-only and rejected in pump config payloads.
- Applying valid settings should reinitialize pump runtime from the active hardware map and restart only if it was previously running, matching current behavior.

## Recommended Plan Split

### Plan 09-01: Hardware Map API

Add route capacity, hardware map serializers, safe option arrays, `GET /api/hardware/map`, and `POST /api/hardware/map` with pending-save and confirmation behavior.

### Plan 09-02: Cooling Config And Mode APIs

Add cooling config serializers/parsers, `GET /api/cooling/config`, `POST /api/cooling/config`, and `POST /api/cooling/mode`, using active hardware and safe runtime apply behavior.

### Plan 09-03: Pump Config Completion, Docs, And Build

Complete `/api/pump/config` dual-channel mutation support, update docs/handoff notes, run source checks, and run the ESP-IDF build.

## Key Risks

- **Route capacity:** Current capacity has only one free handler. Increase `APP_TEMPLATE_HTTP_MAX_URI_HANDLERS` before registering new routes.
- **Unsafe runtime pin changes:** Hardware map POST must save pending GPIO values only; active runtime pins should change only after reboot.
- **Confirmation bypass:** Hardware map POST should reject saves unless the explicit confirmation field is true.
- **Test ON persistence:** Test ON must remain runtime-only even though a cooling mode enum exists in storage.
- **Partial apply after save:** Cooling config POST may save settings but fail runtime reinit if hardware is invalid. The response must be explicit and relay-safe.
- **Legacy payload compatibility:** Existing pump config UI may still send `relay_polarity`; do not break it while adding Relay 1/Relay 2 polarity fields.

## Verification Guidance

Focused checks:

- `rg "/api/hardware/map|confirm_reboot_required|nvs_store_save_pending_hardware_map|hardware_map_options_for_role|hardware_map_gpio_allowed_for_role" main/web_server.c`
- `rg "/api/cooling/config|/api/cooling/mode|nvs_store_save_cooling_settings|cooling_control_set_mode|cooling_control_start_test" main/web_server.c`
- `rg "relay1_polarity|relay2_polarity|timer1_start_phase|timer2_start_phase|readonly_fields" main/web_server.c`
- `rg "is_same_origin\\(req, false\\).*hardware|is_same_origin\\(req, false\\).*cooling|is_same_origin\\(req, false\\).*pump" main/web_server.c`
- `rg "APP_TEMPLATE_HTTP_MAX_URI_HANDLERS" components/app_config/app_config.h`
- `.\scripts\build.ps1`
- `Test-Path build\fish_pump_relay_timer_control.bin`

No external repositories were consulted for Phase 9 research, so no `REFERENCE.md` update is required unless execution later uses external examples.

## RESEARCH COMPLETE
