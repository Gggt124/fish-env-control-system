---
phase: 03-authenticated-pump-control-api
status: passed
verified: 2026-05-20
requirements:
  - RUN-01
  - RUN-02
  - API-01
  - API-02
  - API-03
  - API-04
  - API-05
  - API-06
---

# Phase 03 Verification: Authenticated Pump Control API

## Result

status: passed

Phase 3 achieved its phase goal: authenticated JSON APIs now expose pump configuration, pump status, start, and stop controls while preserving the existing session auth and same-origin POST protection patterns. The existing system `/api/status` endpoint remains unchanged for pump runtime fields, and the firmware builds successfully.

## Evidence

| Must Have | Evidence | Result |
|---|---|---|
| Pump config read exists | `handle_api_pump_config_get()` returns timers, `auto_start`, `relay_polarity`, read-only `float_gpio`, `relay_gpio`, `debounce_ms`, and `settings_status` | PASS |
| Pump config save exists | `handle_api_pump_config_post()` parses a full replacement payload, rejects read-only fields, validates durations/polarity, saves with `nvs_store_save_pump_settings()`, then applies runtime config | PASS |
| Save-before-apply safety | POST config saves NVS before calling `pump_control_init()` and restarts only when `before_status.running` was true | PASS |
| Config validation has stable first-error responses | `api_pump_parse_settings_payload()` returns one error code/message path such as `missing_field`, `duration_out_of_range`, `read_only_field`, or `invalid_relay_polarity` | PASS |
| Pump runtime status exists | `handle_api_pump_status()` uses `api_pump_add_status_fields()` for `running`, `initialized`, `config_valid`, `initial_stabilizing`, `float_state`, `active_timer`, `phase`, `countdown_sec`, `relay_energized`, GPIOs, `auto_start`, and `settings_status` | PASS |
| Start is authenticated and idempotent | `handle_api_pump_start()` requires auth and same-origin, returns `already_running:true` without calling `pump_control_start()` when already running | PASS |
| Stop is authenticated and idempotent | `handle_api_pump_stop()` requires auth and same-origin, returns `already_stopped:true` when already stopped, and confirms stopped plus relay inactive state before success | PASS |
| Start failure fails safe | `handle_api_pump_start()` calls `pump_control_stop()` on start failure and returns `start_failed` with a pump status snapshot | PASS |
| All five pump routes registered | `web_server_start()` registers GET/POST `/api/pump/config`, GET `/api/pump/status`, POST `/api/pump/start`, and POST `/api/pump/stop` | PASS |
| Route capacity is sufficient | Route count is 18 and `APP_TEMPLATE_HTTP_MAX_URI_HANDLERS` is 18 | PASS |
| Existing `/api/status` remains system-only | Inspection of `handle_api_status()` shows system, MAC, memory, uptime, Wi-Fi, and DNS fields only | PASS |
| No Phase 3 static UI changes | `rg "/api/pump|timer1_on_sec|relay_polarity" main/static` returned no matches | PASS |
| Firmware builds | `.\scripts\build.ps1` completed successfully with ESP-IDF 6.1 | PASS |
| Firmware binary exists | `Test-Path build\fish_pump_relay_timer_control.bin` returned `True` | PASS |
| Schema drift gate | `gsd-sdk query verify.schema-drift 03` returned `drift_detected:false` | PASS |

## Automated Checks

- `rg "/api/pump/config|/api/pump/status|/api/pump/start|/api/pump/stop" main/web_server.c`
- `rg "require_auth\\(req\\)" main/web_server.c`
- `rg "is_same_origin\\(req\\)" main/web_server.c`
- `rg "timer1_on_sec|timer1_off_sec|timer2_on_sec|timer2_off_sec|auto_start|relay_polarity" main/web_server.c`
- `rg "float_state|active_timer|phase|countdown_sec|relay_energized|settings_status" main/web_server.c`
- `rg "already_running|already_stopped" main/web_server.c`
- `rg "/api/pump|timer1_on_sec|relay_polarity" main/static`
- `Select-String -Path main\web_server.c -Pattern "static esp_err_t handle_api_status" -Context 0,90`
- `gsd-sdk query verify.schema-drift 03`
- `.\scripts\build.ps1`
- `Test-Path build\fish_pump_relay_timer_control.bin`

## Requirement Coverage

- **RUN-01:** Covered by authenticated `POST /api/pump/start` route and `pump_control_start()` integration.
- **RUN-02:** Covered by authenticated `POST /api/pump/stop` route and relay-inactive confirmation.
- **API-01:** Covered by authenticated `GET /api/pump/config` response fields.
- **API-02:** Covered by authenticated same-origin `POST /api/pump/config` validation, NVS save, and runtime apply.
- **API-03:** Covered by start route, `already_running` idempotency flag, and status snapshot.
- **API-04:** Covered by stop route, `already_stopped` idempotency flag, and status snapshot.
- **API-05:** Covered by machine-friendly `GET /api/pump/status` fields.
- **API-06:** Covered by `require_auth(req)` on all pump routes and `is_same_origin(req)` on pump POST routes.

## Notes

- The first `.\scripts\build.ps1` invocation exceeded the tool timeout while the ESP-IDF process continued. The remaining build processes finished, and a second build wrapper run completed successfully with captured output.
- Build output reported binary size `0xec480`, leaving `0x13b80` bytes free in the smallest app partition.

## Human Verification

None required for Phase 3. Hardware flashing and real relay/float behavior remain Phase 5 validation scope.

## Gaps

None.
