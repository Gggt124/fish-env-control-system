# Phase 3: Authenticated Pump Control API - Context

**Gathered:** 2026-05-20
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 3 exposes authenticated JSON APIs for pump configuration, pump runtime status, and start/stop control. It must preserve the existing login/session middleware, same-origin POST protection, Wi-Fi setup access, SoftAP fallback, and current `/api/status` system endpoint behavior. This phase does not add the web UI controls, editable GPIO/debounce settings, event logs, charts, WebSocket updates, hardware validation, or production auth hardening.

</domain>

<decisions>
## Implementation Decisions

### Route Shape
- **D-01:** Pump APIs use a dedicated `/api/pump/*` namespace so pump control does not collide with system, Wi-Fi, or future APIs.
- **D-02:** Pump config uses `GET /api/pump/config` for read and `POST /api/pump/config` for save. Do not add a separate `/save` route.
- **D-03:** Pump runtime status lives at `GET /api/pump/status`. Do not extend the existing `GET /api/status` system endpoint in Phase 3.
- **D-04:** Start and stop are separate action routes: `POST /api/pump/start` and `POST /api/pump/stop`.
- **D-05:** All new pump routes must use the existing protected API auth pattern. POST routes must also use the existing same-origin `Origin`/`Referer` mitigation pattern.

### Config Contract
- **D-06:** `GET /api/pump/config` returns Timer 1 and Timer 2 ON/OFF durations, `auto_start`, `relay_polarity`, and read-only GPIO/debounce defaults.
- **D-07:** Timer durations in JSON use seconds only, for example `timer1_on_sec`, `timer1_off_sec`, `timer2_on_sec`, and `timer2_off_sec`. Phase 4 UI can convert to minutes/seconds.
- **D-08:** Editable config fields for `POST /api/pump/config` are the four timer duration fields, `auto_start`, and `relay_polarity`.
- **D-09:** Hardware/debounce fields such as `float_gpio`, `relay_gpio`, and `debounce_ms` are read-only in Phase 3. If a save request includes them, reject the request with a read-only-field error instead of ignoring them.
- **D-10:** Config save is full replacement, not partial update. The request payload must include all editable fields.
- **D-11:** Save behavior must follow the Phase 2 locked order: validate the complete payload, save to NVS successfully, then apply runtime changes. If NVS save/commit fails, runtime pump config and state must remain unchanged.
- **D-12:** If config is saved while pump control is running, apply it safely by stopping the relay inactive, reinitializing with the new config, and restarting only because it was running before the save. If stopped, reinitialize stopped and keep relay inactive.
- **D-13:** `auto_start` remains a boot-only setting. Changing it through config save must not by itself start or stop the currently running pump.

### Start And Stop Responses
- **D-14:** `POST /api/pump/start` is idempotent. If pump control is already running, return success with `already_running: true`; do not reset countdown or timer state.
- **D-15:** `POST /api/pump/stop` is idempotent. If pump control is already stopped, return success with `already_stopped: true` and keep the relay inactive.
- **D-16:** Successful start/stop responses include a pump status snapshot so the web UI can update immediately without a second status request.
- **D-17:** If start fails because config is invalid or pump control is not ready, fail safe by calling or confirming stop/relay inactive, then return `ok: false`, a stable error code, and a status snapshot if one can be read.

### Error Style
- **D-18:** Config validation errors use HTTP 400 with `ok: false`.
- **D-19:** Runtime/internal failures use meaningful HTTP status codes with `ok: false`, for example HTTP 500 for internal/NVS failures and HTTP 409 for state or config conflicts.
- **D-20:** Error bodies include a stable `error` code and a short `message`, for example `{"ok":false,"error":"duration_out_of_range","message":"Timer durations must be 5..86400 seconds"}`.
- **D-21:** When multiple validation errors exist, return the first error only. Do not build an array of all validation errors in this phase.

### Status Detail
- **D-22:** `GET /api/pump/status` returns machine-friendly fields, not display-ready labels. Use booleans, seconds, and stable strings that the Phase 4 UI can render.
- **D-23:** Enum-like values in JSON use lowercase stable strings, for example `float_state: "on" | "off" | "unknown"`, `active_timer: "timer1" | "timer2" | "none"`, and `phase: "on" | "off" | "idle"`.
- **D-24:** Pump status includes runtime fields plus `auto_start`. It does not duplicate full timer duration config; clients should use `/api/pump/config` for editable settings.
- **D-25:** Pump status includes a `settings_status` value that reflects Phase 2 load health, using values such as `loaded`, `defaults_missing`, `defaults_invalid`, and `defaults_error`.

### the agent's Discretion
Planner may choose exact helper function names, JSON object nesting, route registration ordering, and final stable error-code names as long as the decisions above are preserved. Planner may add small internal helpers to convert C enums/status structs to JSON strings and to share pump status serialization between `/api/pump/status` and start/stop responses.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Scope And Requirements
- `.planning/PROJECT.md` - Product context, active Phase 3 requirements, hardware defaults, timer defaults, and current key decisions.
- `.planning/REQUIREMENTS.md` - Requirement IDs covered by Phase 3: RUN-01, RUN-02, API-01, API-02, API-03, API-04, API-05, and API-06.
- `.planning/ROADMAP.md` - Phase 3 goal, success criteria, and boundary from Phase 4 UI work.
- `.planning/phases/01-hardware-safe-pump-control-core/01-CONTEXT.md` - Locked start/stop idempotency, relay inactive safety, timer validation, and fail-safe behavior.
- `.planning/phases/02-pump-settings-persistence-and-boot-behavior/02-CONTEXT.md` - Locked persistence, auto-start, save/apply, and invalid-settings decisions that Phase 3 APIs must preserve.

### Codebase Maps
- `.planning/codebase/STACK.md` - ESP-IDF stack, HTTP/cJSON dependency, pump-control component, and build workflow.
- `.planning/codebase/ARCHITECTURE.md` - Component boundaries, web-server ownership, session auth, pump-control state, and integration points.
- `.planning/codebase/INTEGRATIONS.md` - Existing browser/HTTP, session, Wi-Fi, NVS, mDNS, and local-only integration model.

### Source Integration Points
- `main/web_server.c` - Existing route registration, protected API auth, same-origin POST checks, cJSON parsing, and JSON response patterns.
- `main/app_main.c` - Current boot-time pump settings load, default merge, auto-start suppression, and fail-safe startup behavior.
- `components/pump_control/pump_control.h` - Runtime config/status structs and public start/stop/status APIs exposed by the pump component.
- `components/nvs_store/nvs_store.h` - Persisted pump settings struct, load-status enum, defaults, load, save, and clear APIs.
- `components/app_config/app_config.h` - Product defaults for HTTP handler capacity, timer bounds, GPIO defaults, relay polarity default, debounce, and auto-start default.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `main/web_server.c`: already has `require_auth()`, `is_same_origin()`, `send_json()`, cJSON usage, protected Wi-Fi APIs, and `/api/status` serialization patterns that Phase 3 should extend for pump APIs.
- `components/pump_control/`: already owns runtime validation, GPIO setup, relay inactive behavior, debounce, timer phase state, start/stop, and status snapshots.
- `components/nvs_store/`: already owns pump setting defaults, load-status reporting, validation, and NVS save/load. Pump APIs should call this wrapper rather than raw NVS.
- `components/app_config/app_config.h`: already contains pump timer min/max, GPIO defaults, relay polarity default, debounce, auto-start default, and HTTP handler capacity.

### Established Patterns
- HTTP API handlers authenticate at the start and protected POST handlers call the same-origin checker before mutating state.
- JSON is built with cJSON and returned through `send_json()`.
- Component APIs hide ESP-IDF details behind prefix-scoped wrappers.
- Runtime state remains component-owned and guarded internally; web handlers should request snapshots instead of reaching into component globals.
- Existing Wi-Fi APIs sometimes return HTTP 200 with `ok:false`, but Phase 3 pump APIs should use the stricter error style decided above.

### Integration Points
- `main/web_server.c` must register four new routes: `GET/POST /api/pump/config`, `GET /api/pump/status`, `POST /api/pump/start`, and `POST /api/pump/stop`.
- `APP_TEMPLATE_HTTP_MAX_URI_HANDLERS` may need to increase because Phase 3 adds new URI handlers.
- A shared pump-status JSON helper in `main/web_server.c` can avoid duplicating status serialization for `/api/pump/status`, `/api/pump/start`, and `/api/pump/stop`.
- Config save likely needs a small apply helper mirroring `app_main.c`'s settings-to-runtime-config merge while preserving the Phase 2 save-before-apply decision.
- The existing `/api/status` system response should remain unchanged in Phase 3.

</code_context>

<specifics>
## Specific Ideas

- Expected route set: `GET /api/pump/config`, `POST /api/pump/config`, `GET /api/pump/status`, `POST /api/pump/start`, and `POST /api/pump/stop`.
- Expected editable config fields: `timer1_on_sec`, `timer1_off_sec`, `timer2_on_sec`, `timer2_off_sec`, `auto_start`, and `relay_polarity`.
- Expected read-only config/status hardware fields may include `float_gpio`, `relay_gpio`, and `debounce_ms`.
- Expected status fields include `running`, `config_valid`, `float_state`, `active_timer`, `phase`, `countdown_sec`, `relay_energized`, `auto_start`, and `settings_status`.

</specifics>

<deferred>
## Deferred Ideas

None - discussion stayed within phase scope.

</deferred>

---

*Phase: 3-Authenticated Pump Control API*
*Context gathered: 2026-05-20*
