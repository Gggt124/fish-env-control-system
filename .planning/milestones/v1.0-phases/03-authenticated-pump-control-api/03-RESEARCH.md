# Phase 3 Research: Authenticated Pump Control API

**Researched:** 2026-05-20
**Phase:** 03-authenticated-pump-control-api

## Research Complete

Phase 3 can be implemented inside the existing `main/web_server.c` boundary. The reusable pump runtime and persistence components already expose the core APIs needed by the web layer:

- `pump_control_get_status()`, `pump_control_start()`, `pump_control_stop()`, `pump_control_init()`
- `nvs_store_load_pump_settings()`, `nvs_store_save_pump_settings()`, `nvs_store_pump_settings_defaults()`
- product defaults and bounds in `components/app_config/app_config.h`

No new component is needed. `main/CMakeLists.txt` already requires both `nvs_store` and `pump_control`, so adding web handlers does not require CMake dependency changes.

## Existing Patterns To Reuse

- Protected API handlers begin with `require_auth(req)` and return `401 Unauthorized` JSON when no valid session is present.
- Protected POST mutation handlers call `is_same_origin(req)` before reading or mutating state.
- JSON responses are created with cJSON and returned through `send_json(req, json, status)`.
- Static and API routes are registered in the `routes[]` array inside `web_server_start()`.
- `APP_TEMPLATE_HTTP_MAX_URI_HANDLERS` is currently `18`; the existing server registers 13 routes, and Phase 3 adds 5 pump routes, exactly reaching the configured capacity.

## Integration Findings

### Config API

`GET /api/pump/config` should load persisted pump settings through `nvs_store_load_pump_settings()`. This lets the response expose both the resolved settings and the settings load health without duplicating boot-only state from `app_main.c`.

`POST /api/pump/config` should parse a full replacement payload containing:

- `timer1_on_sec`
- `timer1_off_sec`
- `timer2_on_sec`
- `timer2_off_sec`
- `auto_start`
- `relay_polarity`

The response contract should use a stable string for relay polarity. Recommended values are `active_low` and `active_high`; internally these map to `nvs_store_pump_settings_t.relay_active_low` and `pump_control_relay_polarity_t`.

Read-only fields `float_gpio`, `relay_gpio`, and `debounce_ms` should be rejected when present in save payloads. This keeps Phase 3 aligned with the source-configured hardware boundary and avoids implying Phase 4/v2 GPIO configurability.

Save ordering should be:

1. Parse and validate the complete request.
2. Save to NVS with `nvs_store_save_pump_settings()`.
3. Only after save succeeds, reinitialize the runtime pump config.

If pump control was running before config save, the safe apply sequence is stop/inactive, initialize with the new config, then restart. If pump control was stopped, initialize stopped and leave the relay inactive. Changing `auto_start` must not start or stop the current runtime by itself.

### Status And Actions

`pump_control_get_status()` provides the runtime fields needed for `/api/pump/status`. The web layer must convert enum values to stable lowercase strings:

- float state: `on`, `off`, `unknown`
- active timer: `timer1`, `timer2`, `none`
- phase: `on`, `off`, `idle`

`POST /api/pump/start` and `POST /api/pump/stop` can be idempotent by reading a status snapshot before calling the runtime action. If already running/stopped, return success with `already_running` or `already_stopped` and do not reset countdown or timer state.

On start failure, the fail-safe response should call or confirm `pump_control_stop()`, then return `ok:false` with a stable error code and a best-effort status snapshot.

### Scope Boundaries

Do not change the existing `/api/status` system endpoint in Phase 3. Pump runtime status belongs in `/api/pump/status`.

Do not add UI controls or static frontend changes in Phase 3. Phase 4 owns the user-facing web pump control UI.

Do not add editable GPIO/debounce settings, event logs, WebSocket updates, OTA, MQTT/cloud, CSRF tokens, HTTPS, or multi-user auth in this phase.

## Risks And Mitigations

| Risk | Mitigation |
|------|------------|
| Handler capacity exceeded after adding pump routes | Count registered routes and verify `APP_TEMPLATE_HTTP_MAX_URI_HANDLERS >= 18`; update only if needed. |
| NVS save failure mutates runtime config | Parse and validate first, then save, then apply runtime only after save succeeds. |
| Config save while running energizes relay unexpectedly | Stop before reinitializing, rely on `pump_control_init()` inactive relay behavior, restart only if it was already running. |
| Status strings drift from Phase 4 expectations | Centralize enum-to-string helpers in `web_server.c`. |
| Pump APIs accidentally bypass existing local auth model | Use `require_auth()` for all pump routes and `is_same_origin()` for all pump POST handlers. |

## Recommended Plan Shape

1. Add shared pump JSON/config helpers plus `GET/POST /api/pump/config`.
2. Add pump status serialization plus `GET /api/pump/status`, `POST /api/pump/start`, and `POST /api/pump/stop`.
3. Register routes, verify handler capacity, run static checks, build, and summarize Phase 3 coverage.

## Research Complete
