# Phase 2: Pump Settings Persistence And Boot Behavior - Context

**Gathered:** 2026-05-19
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 2 persists pump timer settings, relay polarity, and auto-start behavior in NVS, then changes boot behavior so the pump control core initializes from persisted settings before deciding whether to start. It does not add authenticated pump HTTP APIs, web UI controls, editable GPIO/debounce settings, event logs, factory-reset routes, or hardware validation.

</domain>

<decisions>
## Implementation Decisions

### Boot Auto-Start Timing
- **D-01:** Boot order should become `nvs_store_init()` -> load pump settings -> merge persisted values with `pump_control_default_config()` -> `pump_control_init()` -> `pump_control_start()` only when the resolved auto-start setting is enabled.
- **D-02:** Auto-start happens immediately after NVS pump settings are loaded and validated. Do not wait for Wi-Fi, HTTP server, mDNS, or captive DNS startup before starting pump control.
- **D-03:** If saved auto-start is disabled, firmware still initializes `pump_control` with the resolved config and leaves it stopped with relay inactive. This keeps the runtime ready for later API start commands without energizing the relay.
- **D-04:** If `pump_control_init()` or `pump_control_start()` fails during auto-start, firmware must fail safe: relay inactive, pump stopped, error logged, and Wi-Fi/web startup continues so the device remains reachable for correction.

### Missing And Invalid Saved Settings
- **D-05:** Missing pump settings in NVS are a clean first-boot state, not an error. Firmware uses product defaults and the default auto-start value, which is enabled.
- **D-06:** Invalid saved pump settings must not be auto-started. Firmware falls back to default pump config, initializes `pump_control` stopped with relay inactive, logs a warning, and leaves auto-start suppressed for that boot.
- **D-07:** Firmware must not automatically overwrite, clear, or repair invalid saved pump settings in NVS. A future API/UI save should be the explicit repair path.
- **D-08:** Pump settings load should expose a load status that distinguishes at least: loaded saved settings, defaults used because settings were missing, and defaults used because saved settings were invalid. Future status/API code should be able to report this clearly.

### Settings Ownership
- **D-09:** Phase 2 persists only Timer 1 ON/OFF seconds, Timer 2 ON/OFF seconds, relay polarity, and auto-start. GPIO pins, float polarity, and debounce duration remain source defaults in this phase.
- **D-10:** Raw NVS access for pump settings belongs in `components/nvs_store/`, following the existing Wi-Fi/AP config persistence pattern. `pump_control` must remain a runtime/GPIO component and must not read or write NVS directly.
- **D-11:** Pump settings should use a separate NVS namespace, approximately `pump_cfg`, rather than adding pump keys to the existing `wifi_cfg` namespace.
- **D-12:** Use a dedicated persisted-settings struct containing only persisted fields and load metadata. Merge that struct with `pump_control_default_config()` at boot to produce the runtime `pump_control_config_t`.

### Future Save Behavior
- **D-13:** Future save behavior should validate and save settings successfully before mutating runtime pump state. If NVS save/commit fails, runtime config and state remain unchanged and the API should return an error.
- **D-14:** When a future API saves pump config while pump control is running, the safe apply behavior is: validate -> save NVS -> stop relay inactive -> re-initialize with the new config -> restart only if it was running before the save.
- **D-15:** When a future API saves pump config while pump control is stopped, the safe apply behavior is: validate -> save NVS -> re-initialize stopped with the new config -> keep relay inactive.
- **D-16:** `auto_start` is a boot-only setting. Changing it to disabled must not stop a pump that is currently running; runtime start/stop remains a separate control.
- **D-17:** A clear/default helper for pump settings may be added if useful for tests or future APIs, but Phase 2 must not add an HTTP route or UI for clearing pump settings.

### the agent's Discretion
Planner may choose exact function names, enum names, NVS key names, and load-status type names as long as the decisions above and existing component conventions are preserved. Planner may decide whether the optional clear/default helper is worth adding in Phase 2; if added, it must stay internal to component APIs and must not expose a route or UI.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Scope And Requirements
- `.planning/PROJECT.md` — Product context, active Phase 2 requirements, timer defaults, auto-start default, and hardware safety constraints.
- `.planning/REQUIREMENTS.md` — Requirement IDs covered by Phase 2: TIME-08, RUN-04, and RUN-05. Also documents v2 GPIO/debounce configurability as out of this phase's scope.
- `.planning/ROADMAP.md` — Phase 2 goal, success criteria, and phase boundary.
- `.planning/phases/01-hardware-safe-pump-control-core/01-CONTEXT.md` — Locked Phase 1 decisions for relay safety, timer validation, stop/start semantics, and default pump config behavior.

### Codebase Maps
- `.planning/codebase/STACK.md` — ESP-IDF stack, current components, NVS usage, and build workflow.
- `.planning/codebase/ARCHITECTURE.md` — Boot flow, component boundaries, state management, and extension points for persistence and pump control.
- `.planning/codebase/CONVENTIONS.md` — Component API, error handling, memory, concurrency, and documentation conventions.

### Source Integration Points
- `components/app_config/app_config.h` — Product defaults for pump GPIO, relay polarity, timer durations, timer bounds, and debounce.
- `components/pump_control/pump_control.h` — Runtime config/status types and public pump-control APIs.
- `components/pump_control/pump_control.c` — Runtime validation, relay inactive handling, start/stop behavior, timer state machine, and default config merge target.
- `components/nvs_store/nvs_store.h` — Existing NVS wrapper API pattern to extend for pump settings.
- `components/nvs_store/nvs_store.c` — Existing namespace/key handling, default fallback behavior, and commit pattern.
- `main/app_main.c` — Boot orchestration point that must move pump initialization after NVS initialization and apply auto-start behavior.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `components/nvs_store/`: existing persistence wrapper for Wi-Fi credentials, AP config, and static IP settings. Phase 2 should add pump settings APIs here rather than opening raw NVS from `main` or `pump_control`.
- `components/pump_control/`: runtime pump-control component already validates timer bounds and relay polarity, initializes relay inactive, starts/stops safely, and exposes status snapshots.
- `components/app_config/app_config.h`: source of product defaults for timer durations, relay polarity, GPIO pins, debounce, and timer bounds.

### Established Patterns
- Components expose one header and one implementation file with prefix-scoped APIs.
- NVS load functions use source defaults when keys are missing and keep raw key names hidden inside `nvs_store.c`.
- Startup should log-and-continue for non-network service failures when continuing preserves local recovery access.
- Pump control currently keeps runtime state private and protected by `s_pump_mutex`; persistence should not break that boundary.

### Integration Points
- `main/app_main.c` currently initializes pump control before NVS with default config. Phase 2 should reorder this so NVS initializes first, pump settings load next, then pump control initializes from the resolved config.
- `components/nvs_store/CMakeLists.txt` may need to depend on `pump_control` if the persisted struct reuses pump timer/polarity types. Planner should avoid circular dependencies and may define narrow storage types if cleaner.
- Future Phase 3 API should reuse the Phase 2 save/load/apply semantics instead of inventing separate runtime behavior.

</code_context>

<specifics>
## Specific Ideas

- Use an NVS namespace named approximately `pump_cfg`.
- Persisted fields are Timer 1 ON seconds, Timer 1 OFF seconds, Timer 2 ON seconds, Timer 2 OFF seconds, relay polarity, and auto-start.
- Missing settings should behave like a clean new device: defaults plus auto-start enabled.
- Invalid saved settings should be visible through a load-status value for future API/status reporting.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.

</deferred>

---

*Phase: 2-Pump Settings Persistence And Boot Behavior*
*Context gathered: 2026-05-19*
