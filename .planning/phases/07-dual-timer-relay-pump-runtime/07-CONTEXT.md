# Phase 7: Dual Timer Relay Pump Runtime - Context

**Gathered:** 2026-05-23
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 7 replaces the current single-relay selected-timer runtime with a two-channel pump runtime driven by one binary float switch. Float ON selects Timer 1 and Relay 1. Float OFF selects Timer 2 and Relay 2. The selected channel runs its own ON/OFF timer cycle, float transitions stop the previous channel and restart the selected channel from its configured start phase, and disabled/stopped pump control keeps both pump relays OFF while still reading and reporting float state.

This phase may make the minimum API/status and dashboard updates needed to make the new dual-relay runtime understandable and testable. It does not implement DS18B20 cooling runtime, authenticated hardware mutation APIs, the full Hardware/Install page, event logs, charts, WebSockets, OTA, cloud/MQTT, HTTPS, or production auth hardening.

</domain>

<decisions>
## Implementation Decisions

### Float Transition Timing
- **D-01:** During debounce, wait for debounce confirmation before switching channels. Do not turn relays off or switch channels based only on raw float-signal changes.
- **D-02:** After debounce confirms a real float-state change, turn the old relay OFF first, then start the newly selected timer/channel. Relay 1 and Relay 2 must never intentionally overlap ON.
- **D-03:** At Start or auto-start, if float state is still unknown during stabilization, keep Relay 1 and Relay 2 OFF until float debounce confirms state.
- **D-04:** The firmware should keep reading float state while pump control is stopped so dashboard/status can show Float ON/OFF before Start instead of staying unknown.
- **D-05:** When stopped, status should show the timer that is ready to run if Start is pressed: Float ON -> Timer 1 ready, Float OFF -> Timer 2 ready. Both relays remain OFF.

### Dual Relay API And Status Contract
- **D-06:** Phase 7 should add dual-relay status fields while preserving compatibility fields. Keep `relay_energized` as a compatibility alias for the relay of the active/ready channel.
- **D-07:** Add an `active_relay` status string with values `relay1`, `relay2`, and `none`.
- **D-08:** Add separate relay energized fields, expected as `relay1_energized` and `relay2_energized`, so hardware validation can confirm only one pump relay is ON.
- **D-09:** In stopped state, `active_timer` and `active_relay` should show the ready channel selected by float state, but `running:false`, `relay1_energized:false`, `relay2_energized:false`, and `relay_energized:false` must make clear that no relay is energized.
- **D-10:** If firmware detects an impossible or unsafe relay state such as both pump relays energized together, GPIO write failure, or relay state mismatch, fail safe: force both relays OFF, stop the controller, and expose a status fault/config-invalid signal instead of continuing silently.

### Timer Start Phase Semantics
- **D-11:** If Timer 1 or Timer 2 has `start_phase = OFF`, Start, auto-start, and float transitions into that channel begin the real OFF phase. The selected relay remains OFF and countdown starts from that timer's OFF duration before entering ON.
- **D-12:** Every confirmed float transition stops the previous timer and restarts the newly selected channel from that channel's configured `start_phase`. Do not wait for the old phase to finish and do not inherit phase/countdown from the old channel.
- **D-13:** When stopped, dashboard/status should preview the ready channel's `start_phase` and corresponding duration as `phase` and `countdown_sec`, while `running:false` and both relay energized fields remain false.
- **D-14:** If timer durations or start phases are saved while the controller is running, preserve the existing safe save semantics: force relays safe/off, reinitialize with the new config, and if the controller was running before save, restart from the current float state and selected channel's configured `start_phase`.

### Minimum Dashboard Clarity
- **D-15:** Phase 7 should make only the minimum static dashboard/UI updates needed so the mapping and status are correct. Do not redesign the full dashboard in this phase.
- **D-16:** UI wording must state the new mapping directly: Float ON -> Timer 1 / Relay 1 and Float OFF -> Timer 2 / Relay 2. This supersedes the v1.0 wording that tied Float ON to Timer 2.
- **D-17:** Dashboard should show Relay 1 and Relay 2 state separately in Phase 7 so manual hardware testing can verify there is no relay overlap.
- **D-18:** Stopped-state preview wording should make it clear the channel is ready to start, not currently running, for example ready Timer/Relay plus start phase/countdown while Relay 1 and Relay 2 cards remain OFF.

### the agent's Discretion
Planner may choose exact C struct names, internal helper names, JSON object placement, UI element IDs, and Thai microcopy details as long as the locked behavior above is preserved. Planner may decide whether status faults are represented with a boolean such as `fault`, a stable string such as `fault_code`, or `config_valid:false` plus an error field, as long as unsafe relay behavior fails safe and is visible to API/UI consumers.

### Folded Todos
- **Define safe GPIO map and cooling hardware contract:** Folded into Phase 7 context for GPIO/relay safety. Phase 7 must consume the Phase 6 hardware contract for active float GPIO, Relay 1 GPIO, Relay 2 GPIO, independent relay polarity, and timer start phases. Cooling hardware remains out of runtime scope until Phase 8.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Scope And Requirements
- `.planning/PROJECT.md` - Product context, v1.1 milestone goal, active Phase 7 focus, validated hardware defaults, and key decisions.
- `.planning/REQUIREMENTS.md` - Requirements PUMP-01 through PUMP-06 that Phase 7 must satisfy.
- `.planning/ROADMAP.md` - Phase 7 goal, success criteria, and boundaries from Phase 8-10 work.

### Prior Phase Decisions
- `.planning/phases/03-authenticated-pump-control-api/03-CONTEXT.md` - Locked `/api/pump/*` route shape, compatibility status fields, save/apply semantics, error style, and start/stop response behavior.
- `.planning/phases/04-web-pump-control-ui/04-CONTEXT.md` - Locked dashboard polling, countdown, degraded status behavior, and Thai wording patterns that Phase 7 should minimally update.
- `.planning/phases/05-hardware-validation-and-regression-pass/05-CONTEXT.md` - Manual hardware validation model and v1.0 float/relay behavior evidence.
- `.planning/phases/06-hardware-contract-and-persistent-map-foundation/06-01-SUMMARY.md` - Hardware role contract, safe GPIO defaults, Relay 2 default GPIO27, and hardware_map component boundary.
- `.planning/phases/06-hardware-contract-and-persistent-map-foundation/06-02-SUMMARY.md` - Active/pending hardware map persistence, independent Relay 1/Relay 2 polarity, and timer start-phase schema.
- `.planning/phases/06-hardware-contract-and-persistent-map-foundation/06-03-SUMMARY.md` - Boot/API integration handoff for active hardware map, read-only status fields, and Phase 7 readiness.

### Codebase Maps
- `.planning/codebase/STACK.md` - ESP-IDF stack, GPIO driver usage, pump_control and hardware_map components, NVS store, and build workflow.
- `.planning/codebase/ARCHITECTURE.md` - Component boundaries, boot data flow, pump runtime state, HTTP/API integration, and concurrency model.
- `.planning/codebase/CONVENTIONS.md` - C component API style, error handling, HTTP patterns, frontend conventions, and build practices.

### Source Integration Points
- `docs/hardware.md` - Installer-facing hardware role defaults, safe GPIO choices, relay polarity warning, active/pending map semantics, and NVS schema.
- `components/pump_control/pump_control.h` - Current single-relay public runtime config/status contract that Phase 7 will extend to two pump relays.
- `components/pump_control/pump_control.c` - Existing debounce, timer selection, relay inactive handling, phase transitions, periodic tick, and start/stop implementation.
- `components/hardware_map/hardware_map.h` - Role enums, `hardware_map_t`, independent relay polarity map, timer start phase enum, and validators.
- `components/nvs_store/nvs_store.h` - Persisted pump settings including Relay 1/Relay 2 polarity and Timer 1/Timer 2 start phases.
- `main/app_main.c` - Boot-time hardware map/settings load, pump runtime config merge, auto-start suppression, and service startup sequence.
- `main/web_server.c` - Pump config/status/start/stop handlers, read-only hardware fields, same-origin/auth patterns, and JSON compatibility surface.
- `main/static/app.js` - Dashboard status rendering, countdown ticking, config save, and pump API usage that need minimal mapping/status updates.
- `main/static/dashboard.html` - Pump dashboard shell where Relay 1/Relay 2 and ready-state wording should appear.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `components/pump_control/`: owns pump runtime state, debounce, timer phase transitions, relay inactive safety, start/stop behavior, and status snapshots. Phase 7 should extend this component rather than moving pump control into web/server code.
- `components/hardware_map/`: already defines active hardware roles and safe defaults for float, pump Relay 1, and pump Relay 2.
- `components/nvs_store/`: already persists Timer 1/Timer 2 durations, Relay 1/Relay 2 polarity, Timer 1/Timer 2 start phases, and active hardware map data.
- `main/web_server.c`: already serializes pump config/status and applies safe save/reinit semantics for pump config changes.
- `main/static/app.js` and `main/static/dashboard.html`: already render the pump dashboard and can be minimally updated for dual-relay fields and corrected float mapping.

### Established Patterns
- Pump runtime state is component-owned and guarded by `s_pump_mutex`.
- A periodic `esp_timer` callback drives debounce and phase transitions.
- Higher-level code uses wrappers such as `pump_control_*()` and `nvs_store_*()` instead of reaching into component internals.
- Config save validates, saves to NVS, then applies runtime changes; if the controller was running before save, it is stopped safe and restarted after successful reinit.
- Protected API handlers preserve session auth and same-origin POST checks.
- The frontend is embedded static HTML/CSS/vanilla JS with no CDN or external dependencies.

### Integration Points
- `pump_control_config_t` needs to represent separate Relay 1 and Relay 2 GPIOs, polarities, and timer start phases.
- `pump_control_status_t` needs to represent separate relay energized states and `active_relay`, while preserving existing status fields where practical.
- `app_main.c` needs to merge active hardware map fields for float, Relay 1, Relay 2, Relay 1 polarity, Relay 2 polarity, timer durations, and start phases into pump runtime config.
- `web_server.c` needs to preserve existing pump API compatibility while adding dual-relay status/config output and read-only safety fields.
- Static dashboard JS/HTML needs minimal changes to render the new mapping and separate Relay 1/Relay 2 state.

</code_context>

<specifics>
## Specific Ideas

- The user explicitly wants dashboard/status to show float state before Start; stopped state should not look stuck at `unknown` when the float can be read.
- In stopped state, the UI should preview the ready timer/relay and its start phase/countdown, but must not imply the pump is currently running.
- Float ON maps to Timer 1 / Relay 1, and Float OFF maps to Timer 2 / Relay 2 for v1.1. Treat older v1.0 wording as superseded.
- Manual hardware testing should be able to confirm Relay 1 and Relay 2 do not overlap ON from the dashboard.

</specifics>

<deferred>
## Deferred Ideas

None - discussion stayed within Phase 7 scope.

</deferred>

---

*Phase: 7-Dual Timer Relay Pump Runtime*
*Context gathered: 2026-05-23*
