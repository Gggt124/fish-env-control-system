# Phase 1: Hardware-Safe Pump Control Core - Context

**Gathered:** 2026-05-19
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 1 adds the firmware-only pump-control core: safe GPIO defaults, relay inactive initialization, binary float-switch reading, timer selection, ON/OFF phase transitions, stop behavior, and build validation. It does not add NVS persistence, web APIs, UI controls, auto-start integration, pause/resume, event logs, or hardware validation beyond `idf.py build`.

</domain>

<decisions>
## Implementation Decisions

### Relay Polarity And Inactive Safety
- **D-01:** Default relay polarity is active-low because the target relay module is active-low: GPIO LOW energizes the relay, GPIO HIGH is inactive.
- **D-02:** Phase 1 pump core initializes in `stopped` state with relay inactive. It must not auto-start pump control during boot/init.
- **D-03:** Invalid pin, polarity, or timer config is fail-safe: do not start the state machine, keep relay inactive, and report invalid config/status to the caller.
- **D-04:** Relay GPIO initialization must minimize pulse risk. Set or prepare the inactive level before enabling output where ESP-IDF allows it, then write the inactive level again after `gpio_config`.

### Float Switch Change And Debounce
- **D-05:** Float input uses a 100 ms stable debounce. A new raw state is accepted only after it remains stable for at least 100 ms.
- **D-06:** When the debounced float state changes, switch immediately to the newly selected timer and reset into that timer's ON phase with its ON duration loaded.
- **D-07:** While debounce is pending, keep using the last confirmed float state. Countdown continues, active timer does not change, and relay is not forced inactive solely because the raw input is bouncing.
- **D-08:** On pump core start, debounce the float input before selecting the first timer. During this initial stabilization window, relay remains inactive and no timer phase starts yet.

### Stop And Restart Semantics
- **D-09:** `stop` while running immediately sets relay inactive, sets `running=false`, and clears active timer, phase, and countdown to idle.
- **D-10:** `start` from stopped debounces the float input first, selects Timer 1 or Timer 2 from the confirmed float state, and starts that timer's ON phase.
- **D-11:** `start` while already running is no-op success. It must not reset countdown or change relay state.
- **D-12:** `stop` while already stopped is no-op success and keeps relay inactive.
- **D-13:** Future UI/API should prevent repeated Start clicks while running, but the Phase 1 core still handles repeated `start` safely.

### Timer Validation And Anti-Chatter
- **D-14:** Every Timer 1 and Timer 2 ON/OFF duration must be at least 5 seconds.
- **D-15:** Every timer phase duration must be at most 24 hours.
- **D-16:** Timer config outside the valid range rejects the entire config. Do not clamp values and do not silently fall back to defaults.
- **D-17:** Product timer defaults should be declared as `app_config.h` constants, and the pump component should expose a default config struct derived from those constants.

### the agent's Discretion
Planner may choose exact internal API names, task/timer implementation details, enum names, and status struct layout as long as the decisions above and existing component conventions are preserved.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Scope And Requirements
- `.planning/PROJECT.md` — Product context, active requirements, default timer values, hardware defaults, and key constraints.
- `.planning/REQUIREMENTS.md` — Requirement IDs covered by Phase 1, including HW-01..HW-06, TIME-05..TIME-07, RUN-03, RUN-06..RUN-11, and VAL-01.
- `.planning/ROADMAP.md` — Phase 1 goal, success criteria, and phase boundaries.

### Flow Reference
- `ref-file/flow-document-v6.md` — Dual Timer + Float Switch + Relay behavior reference. Use binary float switch logic, but prefer `.planning/PROJECT.md` and `.planning/REQUIREMENTS.md` where timer defaults differ from this reference.
- `ref-file/flow-diagram-5.html` — Prototype/simulation reference for the timer/float/relay flow. Treat it as behavioral reference only; Phase 1 target is real ESP32 firmware, not UI simulation.

### Codebase Maps
- `.planning/codebase/STACK.md` — ESP-IDF stack, existing components, build workflow, and dependency constraints.
- `.planning/codebase/ARCHITECTURE.md` — Boot flow, component boundaries, state management, and integration points.
- `.planning/codebase/CONVENTIONS.md` — C style, component API pattern, error handling, memory, concurrency, and documentation conventions.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `components/app_config/app_config.h`: central place for product constants; add GPIO defaults, relay polarity default, timer defaults, debounce duration, and min/max duration constants here.
- `components/*` pattern: reusable low-level modules expose one header and one implementation file with prefix-scoped APIs; the pump core should follow this component style.
- `main/app_main.c`: current boot orchestration point; Phase 1 can initialize the pump core after existing system services without starting the pump automatically.

### Established Patterns
- Components declare ESP-IDF dependencies in their own `CMakeLists.txt` via `REQUIRES`.
- Public component APIs should stay small and hide raw ESP-IDF details from higher layers where practical.
- Shared mutable state in components should use file-local `static` state and FreeRTOS synchronization when needed.
- Firmware validation gate for this phase is `idf.py build`; no unit tests currently exist.

### Integration Points
- Add a new reusable `components/pump_control/` component for GPIO setup, config validation, float debounce, timer state, and relay output decisions.
- Include the new component from `main/CMakeLists.txt`/component dependencies as needed, while keeping web/API/UI work out of Phase 1.
- Preserve existing Wi-Fi setup, SoftAP fallback, login/session, captive DNS, and static web serving behavior.

</code_context>

<specifics>
## Specific Ideas

- Hardware defaults are GPIO32 for the active-low float input with internal pull-up, and GPIO26 for the relay output.
- Relay inactive level for the selected active-low relay is GPIO HIGH.
- Timer defaults are Timer 1 ON 20 seconds / OFF 1 minute, and Timer 2 ON 10 seconds / OFF 3 minutes.
- Float OFF selects Timer 1. Float ON selects Timer 2. Float state changes reset the newly selected timer into ON phase.

</specifics>

<deferred>
## Deferred Ideas

- Add Pause/Resume as a future capability with its own UI/API behavior. This is not part of Phase 1 start/stop core scope.

</deferred>

---

*Phase: 1-Hardware-Safe Pump Control Core*
*Context gathered: 2026-05-19*
