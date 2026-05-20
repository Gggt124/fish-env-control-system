# Phase 5: Hardware Validation And Regression Pass - Context

**Gathered:** 2026-05-20
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 5 validates the completed ESP32 pump controller on real hardware and confirms that the existing Wi-Fi setup, SoftAP fallback, login, dashboard, status, Wi-Fi page, and pump UI still work after pump control has been added. This phase is a hardware and regression validation pass only. It does not add new relay behavior, new GPIO configurability, event logs, charts, OTA, MQTT/cloud, WebSockets, HTTPS, or production security hardening.

</domain>

<decisions>
## Implementation Decisions

### Hardware Validation Results
- **D-01:** Relay validation uses the currently configured firmware defaults: GPIO26 as relay output and active-low relay polarity. The user has tested this wiring on real hardware and reported it works.
- **D-02:** Float validation uses the currently configured firmware defaults: GPIO32 as binary float input with internal pull-up and switch-to-GND treated as ON. The user reported Float OFF selects Timer 1 and Float ON selects Timer 2 correctly.
- **D-03:** Relay phase validation passed on hardware: relay output follows ON/OFF timer phase behavior and Stop forces the relay inactive immediately.
- **D-04:** Reboot validation passed on hardware: persisted timer settings and auto-start behavior survive reboot and take effect as expected.
- **D-05:** Web and Wi-Fi regression validation passed after hardware wiring: SoftAP, login, dashboard, status page, Wi-Fi page, Wi-Fi scan/connect, and pump UI remain reachable.

### Validation Approach
- **D-06:** Phase 5 should treat the user's manual hardware results as the primary evidence source because the project has no automated ESP32 hardware test rig.
- **D-07:** Planner/verifier should preserve build validation as a required gate, but build success alone is not sufficient for this phase. Manual hardware validation is the requirement-closing evidence for VAL-02 through VAL-05.
- **D-08:** Findings, if any are discovered during repeat validation, should be documented in phase verification notes or project docs rather than converted into new Phase 5 feature work.

### the agent's Discretion
Planner and verifier may choose the exact checklist format, verification note location, and command sequencing for the final pass, as long as the documented user hardware results above are preserved and the phase remains validation-only.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Scope And Requirements
- `.planning/PROJECT.md` - Product context, current validated state, hardware defaults, timer defaults, and active Phase 5 validation requirement.
- `.planning/REQUIREMENTS.md` - Requirement IDs covered by Phase 5: VAL-02, VAL-03, VAL-04, and VAL-05.
- `.planning/ROADMAP.md` - Phase 5 goal, success criteria, and validation-only boundary.
- `.planning/phases/01-hardware-safe-pump-control-core/01-CONTEXT.md` - Locked relay safety, float GPIO behavior, timer switching, start/stop semantics, and relay inactive safety.
- `.planning/phases/02-pump-settings-persistence-and-boot-behavior/02-CONTEXT.md` - Locked persistence and auto-start behavior that Phase 5 reboot validation confirms.
- `.planning/phases/03-authenticated-pump-control-api/03-CONTEXT.md` - Locked pump API routes, status fields, start/stop behavior, and auth expectations that support UI/regression testing.
- `.planning/phases/04-web-pump-control-ui/04-CONTEXT.md` - Locked dashboard behavior, timer UI, Start/Stop controls, Thai status wording, and refresh behavior that Phase 5 validates on device.

### Codebase Maps
- `.planning/codebase/TESTING.md` - Existing validation model, build command, manual device testing practice, and residual risk.
- `.planning/codebase/CONCERNS.md` - Hardware validation and Wi-Fi availability risks to keep visible during regression.
- `.planning/codebase/STRUCTURE.md` - Source layout and integration points for pump control, web server, and static UI.

### Source Integration Points
- `components/app_config/app_config.h` - Current hardware defaults: GPIO32 float input, GPIO26 relay output, active-low float, active-low relay, timer defaults, and debounce.
- `components/pump_control/pump_control.h` - Public pump config/status contract used to reason about float, relay, timer, and countdown state.
- `components/pump_control/pump_control.c` - GPIO setup, relay inactive handling, float sampling/debounce, timer selection, phase transitions, and stop behavior.
- `main/app_main.c` - Boot-time settings load, pump initialization, auto-start behavior, HTTP server startup, and service initialization sequence.
- `main/web_server.c` - Pump APIs, status/config JSON, same-origin/auth protections, and static route serving used by manual regression.
- `main/static/dashboard.html` - Pump control dashboard shell under test.
- `main/static/app.js` - Pump UI status polling, countdown, config save, Start/Stop, Wi-Fi UI, and auth redirect behavior under test.
- `main/static/style.css` - Embedded no-CDN UI styling under test.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `scripts/build.ps1`: Preferred local build validation entry point before flash/device testing.
- `components/pump_control/`: Owns relay GPIO26 output, float GPIO32 input, timer selection, ON/OFF phase transitions, countdown, and Stop safety.
- `components/nvs_store/`: Owns persisted timer settings, relay polarity, and auto-start settings that reboot validation confirms.
- `main/web_server.c` and `main/static/`: Own the local browser/API surface for dashboard, status, Wi-Fi setup, login, and pump controls.

### Established Patterns
- The project validates through `idf.py build` plus manual ESP32 device testing; there are no automated unit or hardware-in-loop tests in this phase.
- Pump hardware behavior is intentionally binary and GPIO-real: Float OFF selects Timer 1, Float ON selects Timer 2, and relay is energized only during ON phase.
- Wi-Fi and web setup are local-first and must remain reachable without internet through SoftAP fallback and the embedded static UI.

### Integration Points
- Final validation should include a clean build, flash/device test, manual float/relay/timer checks, reboot persistence check, and web/Wi-Fi regression check.
- Any future fixes discovered by validation should preserve the existing component boundaries: pump logic in `components/pump_control/`, persistence in `components/nvs_store/`, routes/static UI in `main/`.

</code_context>

<specifics>
## Specific Ideas

- User confirmed real relay wiring works with GPIO26.
- User confirmed hardware checks 1-4 passed: Float OFF -> Timer 1, Float ON -> Timer 2, relay follows ON/OFF and Stop forces inactive, and reboot persistence works.
- User confirmed the remaining web/Wi-Fi regression check passed.
- Current active hardware pin map for this validation is GPIO32 for the float switch and GPIO26 for the relay.

</specifics>

<deferred>
## Deferred Ideas

None - discussion stayed within phase scope.

</deferred>

---

*Phase: 5-Hardware Validation And Regression Pass*
*Context gathered: 2026-05-20*
