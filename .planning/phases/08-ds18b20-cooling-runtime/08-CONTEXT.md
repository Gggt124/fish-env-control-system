# Phase 8: DS18B20 Cooling Runtime - Context

**Gathered:** 2026-05-23
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 8 adds a separate DS18B20-based cooling runtime that reads temperature, controls the dedicated cooling relay through threshold and hysteresis, fails safe when the sensor is unavailable, supports Auto / Force OFF / Test ON runtime modes, honors the persisted cooling auto-enable boot preference, and enforces compressor minimum off-time by default.

This phase may add the minimum authenticated cooling status surface needed for validation and downstream UI work. It does not implement full cooling mutation/config APIs, hardware GPIO mutation APIs, the Hardware/Install page, owner dashboard redesign, event logs, charts, WebSockets, OTA, cloud/MQTT, HTTPS, or production auth hardening.

</domain>

<decisions>
## Implementation Decisions

### Sensor Fault Behavior
- **D-01:** Runtime DS18B20 read failures should not fault after a single miss. Declare sensor fault after **3 consecutive failed reads**.
- **D-02:** At boot or cooling runtime start, before the first successful temperature reading, treat temperature as `unknown`, keep the cooling relay OFF, and do not immediately declare fault. If 3 consecutive reads fail during startup, then enter sensor fault.
- **D-03:** If DS18B20 returns an impossible or out-of-range value for the supported sensor range, treat it as a sensor fault input rather than a valid temperature.
- **D-04:** After sensor fault, automatically clear the fault only after **2 consecutive successful DS18B20 readings**, then resume the applicable runtime mode.
- **D-05:** In Auto mode, `unknown` or `fault` sensor state forces cooling relay OFF. The only exception is Test ON, which may be allowed for wiring verification under its own timeout and compressor-protection rules.

### Cooling Mode Semantics
- **D-06:** Use `mode` as the main runtime control. `Auto` controls the relay from temperature, `Force OFF` always keeps the cooling relay OFF, and `Test ON` is a temporary manual test mode.
- **D-07:** `auto_enable` is a boot preference only. On boot, `auto_enable:true` starts the cooling runtime in Auto mode; `auto_enable:false` starts in Force OFF.
- **D-08:** If boot starts in Auto but temperature is still unknown, keep `mode=Auto` while holding the cooling relay OFF until a successful DS18B20 reading is available.
- **D-09:** Allow Test ON even when sensor state is `unknown` or `fault` so wiring and relay behavior can be verified without a working temperature reading. Status must clearly report that Test ON is a test/manual override, not automatic temperature cooling.
- **D-10:** Test ON is runtime-only, does not persist across reboot, uses the persisted/default test timeout, and returns to the previous mode when the timeout expires.

### Compressor Protection Timing
- **D-11:** At boot or cooling runtime reinitialization, treat the cooling relay as just turned OFF and enforce the configured compressor minimum off-time before the relay may turn ON, even if temperature is already at or above threshold.
- **D-12:** In Auto mode, when temperature demands cooling during compressor lockout, keep the relay OFF and expose lockout status plus remaining countdown.
- **D-13:** Test ON must obey the same compressor minimum off-time as Auto. If lockout is active, Test ON does not energize the relay and status reports that the test is blocked by compressor protection.
- **D-14:** Restart compressor minimum off-time every time the cooling relay transitions from ON to OFF. This includes Auto hysteresis shutoff, Force OFF, stop/reinit, Test ON timeout, and sensor fault if the relay had been ON.

### Runtime Visibility And Status
- **D-15:** Add a separate authenticated `GET /api/cooling/status` route for Phase 8 cooling runtime status. Keep full cooling configuration/mutation APIs for Phase 9.
- **D-16:** `/api/cooling/status` should expose enough state for debug, basic UI, and manual validation: `mode`, `auto_enable`, `temperature_c`, `temperature_valid`, `sensor_state`, `fault`, `fault_code`, `relay_energized`, `threshold_c`, `hysteresis_c`, `lockout_active`, `lockout_remaining_sec`, and `test_remaining_sec`.
- **D-17:** Also expose demand/blocking state so UI and validation can distinguish "relay is OFF because no cooling is needed" from "cooling is demanded but blocked": include `cooling_demand` and `blocked_reason`.
- **D-18:** Use stable string enums for machine and UI consumers. `sensor_state` should include `unknown`, `ok`, and `fault`. `fault_code` should include at least `none`, `read_failed`, `out_of_range`, and `config_invalid`.
- **D-19:** `blocked_reason` should be a stable enum with values such as `none`, `compressor_lockout`, `sensor_fault`, `force_off`, and `config_invalid`.
- **D-20:** A small cooling summary may be added to `/api/status` only if needed, but cooling runtime details belong in `/api/cooling/status`, not `/api/pump/status`.

### the agent's Discretion
Planner may choose exact C component names, struct names, polling interval, DS18B20 driver strategy, enum constant names, JSON field ordering, and internal helper names as long as the locked runtime behavior above is preserved. If external repositories or driver code are used as references, record them in `REFERENCE.md` per project rules.

### Folded Todos
- **Define safe GPIO map and cooling hardware contract:** Folded into Phase 8 context. Phase 8 must consume the Phase 6 hardware contract for DS18B20 powered-mode wiring, GPIO33 default data pin, external 4.7 kOhm pull-up, GPIO25 cooling relay default, independent cooling relay polarity, safe-off cooling defaults, and 300 second compressor minimum off-time. Hardware-map mutation remains out of scope until Phase 9/10.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Scope And Requirements
- `.planning/PROJECT.md` - Product context, v1.1 milestone goal, Phase 8 active focus, validated hardware defaults, and key safety decisions.
- `.planning/REQUIREMENTS.md` - Requirements COOL-01 through COOL-07 that Phase 8 must satisfy.
- `.planning/ROADMAP.md` - Phase 8 goal, success criteria, and boundaries from Phase 9-10 API/UI work.
- `.planning/todos/pending/define-safe-gpio-map-and-cooling-hardware-contract.md` - Folded todo that originally captured the DS18B20/cooling hardware contract and compressor-protection setup.

### Prior Phase Decisions
- `.planning/phases/06-hardware-contract-and-persistent-map-foundation/06-01-SUMMARY.md` - Hardware role contract, safe GPIO defaults, DS18B20 powered-mode wiring, and cooling relay default GPIO.
- `.planning/phases/06-hardware-contract-and-persistent-map-foundation/06-02-SUMMARY.md` - Cooling settings persistence defaults, independent cooling relay polarity storage, and fail-closed load behavior.
- `.planning/phases/06-hardware-contract-and-persistent-map-foundation/06-03-SUMMARY.md` - Boot-time active hardware map and cooling settings load, read-only API/status exposure, and Phase 8 handoff.
- `.planning/phases/07-dual-timer-relay-pump-runtime/07-CONTEXT.md` - Current pump runtime boundary, dual pump relay behavior, and explicit deferral of DS18B20 cooling runtime to Phase 8.

### Codebase Maps
- `.planning/codebase/STACK.md` - ESP-IDF stack, GPIO driver usage, hardware_map, NVS store, web server, and build workflow.
- `.planning/codebase/ARCHITECTURE.md` - Component boundaries, boot data flow, runtime state ownership, HTTP/API integration, and concurrency model.
- `.planning/codebase/INTEGRATIONS.md` - Local HTTP/API, NVS, Wi-Fi, mDNS, captive DNS, session/auth, and hardware integration surfaces.

### Source Integration Points
- `docs/hardware.md` - Installer-facing DS18B20 wiring, cooling relay, compressor defaults, and `cool_cfg` NVS schema.
- `components/app_config/app_config.h` - Cooling defaults: threshold 30.0 C, hysteresis 1.0 C, auto-enable false, Test ON timeout 10 seconds, compressor minimum off-time 300 seconds, DS18B20 GPIO33, cooling relay GPIO25.
- `components/hardware_map/hardware_map.h` - Hardware roles, cooling mode enum, relay polarity enum, active map fields, and validators.
- `components/hardware_map/hardware_map.c` - Safe GPIO option lists, cooling mode names, defaults, and map validation.
- `components/nvs_store/nvs_store.h` - Persisted cooling settings struct, load status enum, defaults/load/save APIs, and active hardware map APIs.
- `components/nvs_store/nvs_store.c` - `cool_cfg` key ownership, cooling settings validation, default loading, save behavior, and fail-closed handling.
- `main/app_main.c` - Boot-time active hardware map and cooling settings load; Phase 8 should connect runtime initialization here.
- `main/web_server.c` - Existing authenticated API handler patterns, JSON response helpers, same-origin POST conventions, and static route registration model.
- `main/CMakeLists.txt` - Component dependency registration and embedded static file ownership.
- `components/pump_control/pump_control.c` - Existing pattern for component-owned GPIO runtime, relay inactive safety, mutex-protected state, and `esp_timer` tick behavior.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `components/hardware_map/`: owns safe DS18B20 and cooling relay role metadata, defaults, polarity enum, and cooling mode enum without driving hardware.
- `components/nvs_store/`: already persists `nvs_store_cooling_settings_t` with threshold, hysteresis, auto-enable, mode, test timeout, minimum off-time, and cooling relay polarity.
- `components/app_config/app_config.h`: central source for Phase 8 default values.
- `main/web_server.c`: provides the existing auth/session, JSON, static route, and API registration patterns that `GET /api/cooling/status` should follow.
- `components/pump_control/`: useful local pattern for a component-owned hardware runtime with GPIO safe initialization, mutex-protected state, status snapshots, and timer-driven periodic work.

### Established Patterns
- Runtime hardware behavior belongs in a component, not directly in `web_server.c`.
- Higher-level code should use wrapper APIs instead of reaching into storage or GPIO internals.
- NVS wrappers return safe defaults plus explicit load status when values are missing or invalid.
- Relay outputs must initialize inactive and fail safe on invalid config or runtime faults.
- Protected API handlers should use the existing session/auth model; mutation routes require same-origin POST checks, but Phase 8 status is GET-only.
- Frontend and HTTP surfaces are local-first and must not depend on CDN, internet access, WebSocket, or cloud services.

### Integration Points
- Add a cooling runtime component or equivalent component-owned module that consumes the active DS18B20 GPIO, cooling relay GPIO, cooling relay polarity, and persisted cooling settings.
- Initialize cooling runtime from `main/app_main.c` after loading active hardware map and cooling settings. Invalid hardware or cooling settings should suppress unsafe relay operation.
- Register `GET /api/cooling/status` in `main/web_server.c` and serialize the stable runtime/status fields locked above.
- Keep `/api/pump/status` focused on pump runtime. Do not mix cooling runtime into the pump API beyond optional high-level summaries elsewhere.
- Preserve Phase 6/7 behavior: pump relays remain separate from the cooling relay, and Wi-Fi/AP/login/status routes must continue working while cooling runtime runs.

</code_context>

<specifics>
## Specific Ideas

- Cooling behavior is safety-first: no trusted temperature means no automatic relay ON.
- Test ON is allowed for wiring verification even with sensor unknown/fault, but it is still blocked by compressor minimum off-time.
- Boot/reinit compressor lockout intentionally prioritizes compressor protection over immediate cooling.
- Status should make blocked cooling clear: actual relay state, demand, and blocked reason must be distinct.

</specifics>

<deferred>
## Deferred Ideas

None - discussion stayed within phase scope.

</deferred>

---

*Phase: 8-DS18B20 Cooling Runtime*
*Context gathered: 2026-05-23*
