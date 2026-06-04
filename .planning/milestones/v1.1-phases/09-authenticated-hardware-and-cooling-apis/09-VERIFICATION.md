---
phase: 09-authenticated-hardware-and-cooling-apis
status: passed
completed: 2026-05-24
plans: [09-01, 09-02, 09-03]
validation:
  build: passed
  focused_checks: passed
  manual_hardware: not-run
---

# Phase 9 Verification

Phase 9 is verified as complete for repository-level validation.

## Scope Verified

- Authenticated hardware map API added with active/pending map serialization, GPIO option metadata, same-origin mutation protection, and pending-only save behavior.
- Authenticated cooling config and cooling mode APIs added with NVS persistence, active hardware map runtime apply, runtime-only Test ON, and fail-safe stop on apply failure.
- Pump config API updated for canonical `relay1_polarity`, optional `relay2_polarity`, legacy `relay_polarity` compatibility, and read-only hardware GPIO fields.
- API and handoff documentation updated for Phase 10 UI work.

## Checks

- `rg "APP_TEMPLATE_HTTP_MAX_URI_HANDLERS|hardware_map.*json|pending_hardware|float_input_gpio|ds18b20_gpio" components\app_config\app_config.h main\web_server.c`
- `rg "/api/hardware/map|handle_api_hardware_map|nvs_store_load_hardware_map|nvs_store_load_pending_hardware_map|nvs_store_hardware_reboot_required" main\web_server.c`
- `rg "confirm_reboot_required|hardware_map_gpio_allowed_for_role|hardware_map_validate|nvs_store_save_pending_hardware_map|nvs_store_clear_pending_hardware_map|is_same_origin\(req, false\)" main\web_server.c`
- `rg "/api/cooling/config|handle_api_cooling_config_get|nvs_store_load_cooling_settings|api_cooling_add_status_fields|cooling.*limits" main\web_server.c`
- `rg "handle_api_cooling_config_post|nvs_store_save_cooling_settings|cooling_control_init|cooling_control_stop|nvs_store_load_hardware_map|is_same_origin\(req, false\)" main\web_server.c`
- `rg "/api/cooling/mode|handle_api_cooling_mode_post|cooling_control_set_mode|cooling_control_start_test|test_remaining_sec|blocked_reason|test_on" main\web_server.c`
- `rg "relay1_polarity|relay2_polarity|relay_polarity|timer1_start_phase|timer2_start_phase|readonly_fields|api_pump_parse_settings_payload" main\web_server.c`
- `rg "/api/hardware/map|/api/cooling/config|/api/cooling/mode|/api/pump/config|require_auth|is_same_origin\(req, false\)|APP_TEMPLATE_HTTP_MAX_URI_HANDLERS" main\web_server.c components\app_config\app_config.h`
- `rg "/api/hardware/map|/api/cooling/config|/api/cooling/mode|/api/cooling/status|relay1_polarity|relay2_polarity|pending|reboot|Phase 9" docs README.md`
- `rg "Hardware/Install|dashboard redesign|WebSocket|MQTT|OTA|cloud" main components docs`
- `.\scripts\build.ps1`

## Build Result

`.\scripts\build.ps1` passed and generated `build\fish_pump_relay_timer_control.bin`.

- Final binary size: `0x100360`
- Smallest app partition: `0x1f0000`
- Free app partition space: `0xefca0` (48%)

## Hardware Testing

Manual ESP32 flash/device testing was not run in this execution pass. The repository validation gate for this phase is the ESP-IDF build.

## Result

PASSED.
