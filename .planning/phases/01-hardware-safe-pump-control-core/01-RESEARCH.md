# Phase 1 Research: Hardware-Safe Pump Control Core

**Phase:** 01 - Hardware-Safe Pump Control Core  
**Generated:** 2026-05-19  
**Mode:** Inline research fallback for `$gsd-plan-phase 1`  
**Status:** Ready for planning

## Research Summary

Phase 1 should add a reusable ESP-IDF component named `pump_control` and keep all web/API/NVS persistence work out of scope. The safest implementation path is:

1. Add source-level defaults in `components/app_config/app_config.h`.
2. Implement `components/pump_control/` with GPIO setup, config validation, float debounce, timer state, and relay output control.
3. Initialize the component from `main/app_main.c` without starting the pump automatically.
4. Validate with `idf.py build`.

No external repositories were used for this research. The plan is based on project-local references only, so `REFERENCE.md` does not need a new provenance entry.

## Source Context Used

- `.planning/PROJECT.md` defines product scope, hardware defaults, and constraints.
- `.planning/REQUIREMENTS.md` maps Phase 1 requirements: `HW-01..HW-06`, `TIME-05..TIME-07`, `RUN-03`, `RUN-06..RUN-11`, and `VAL-01`.
- `.planning/ROADMAP.md` defines Phase 1 goal and success criteria.
- `.planning/phases/01-hardware-safe-pump-control-core/01-CONTEXT.md` defines decisions `D-01..D-17`.
- `ref-file/flow-document-v6.md` defines the dual timer, binary float switch, and relay phase behavior.
- `components/app_config/app_config.h`, `components/session/`, and `components/wifi_manager/` show local component/API/CMake conventions.
- `main/app_main.c` shows boot ordering and logging style.

## Implementation Findings

### Component Boundary

`pump_control` should live under `components/pump_control/` because it is reusable low-level firmware behavior. It should expose one header and one implementation file, matching existing component conventions.

Recommended files:

- `components/pump_control/CMakeLists.txt`
- `components/pump_control/pump_control.h`
- `components/pump_control/pump_control.c`

Recommended ESP-IDF dependencies:

- `app_config` for defaults.
- `driver` for GPIO APIs.
- `esp_timer` for monotonic timing and periodic state updates.
- `freertos` for mutex protection.

### Public API Shape

The API should stay small and prefix-scoped:

- `pump_control_default_config()` returns a config struct derived from `app_config.h`.
- `pump_control_init(const pump_control_config_t *config)` validates config and initializes GPIO with relay inactive.
- `pump_control_start()` begins debounce/stabilization and then the selected timer ON phase.
- `pump_control_stop()` immediately de-energizes the relay and clears active timer/phase/countdown.
- `pump_control_get_status(pump_control_status_t *out)` exposes current state for future API/UI phases without adding HTTP routes in Phase 1.

This gives Phase 3 a stable component boundary for authenticated pump APIs without forcing API work into Phase 1.

### GPIO Safety

Relay safety should be implemented in the component, not left to callers:

- Default relay GPIO: `GPIO26`.
- Default relay polarity: active-low.
- Active-low relay inactive level: `GPIO_HIGH`.
- During init, prepare/write the inactive level before output enable where ESP-IDF allows it, call `gpio_config`, then write inactive again.
- `stop`, invalid config, stopped state, and initial debounce all force relay inactive.

Float input defaults:

- Default float GPIO: `GPIO32`.
- Internal pull-up enabled.
- Switch-to-GND means active/ON.
- Float OFF selects Timer 1.
- Float ON selects Timer 2.

### Timer And Debounce Behavior

Use elapsed time from `esp_timer_get_time()` rather than decrementing only in the main loop. A component-owned periodic `esp_timer` callback at 100 ms is enough for:

- 100 ms stable debounce.
- Initial stabilization before selecting the first timer.
- Immediate timer switching after a debounced float-state change.
- ON/OFF phase transition checks.

Timer config should reject the whole config if any ON/OFF duration is below 5 seconds or above 24 hours. Do not clamp invalid values.

### Boot Integration

Initialize `pump_control` early in `app_main()` so the relay pin is driven inactive during boot, but do not call `pump_control_start()` in Phase 1. Auto-start is explicitly deferred to Phase 2 because it depends on persisted settings and boot behavior.

`main/CMakeLists.txt` should add `pump_control` to `REQUIRES` only after the component exists.

## Risks And Mitigations

| Risk | Mitigation |
|---|---|
| Relay pulse during boot/init | Set inactive level before and after GPIO output configuration. Initialize component early in boot. |
| Invalid duration causes relay chatter | Reject config if any duration is below 5 seconds. |
| Active-low polarity energizes relay accidentally | Encode polarity in config and default inactive level derived from polarity. Stop/init/invalid states always call relay inactive helper. |
| Float bounce causes rapid timer switching | Require 100 ms stable raw state before accepting a new debounced state. Keep prior confirmed state during pending debounce. |
| Phase 1 accidentally grows into UI/API/NVS work | Keep `pump_control_get_status()` as a component API only; defer HTTP routes, persistence, and UI to later phases. |

## Plan Recommendation

Create three sequential executable plans:

1. `01-01-PLAN.md` - Add source-level pump defaults and validation constants to `app_config.h`.
2. `01-02-PLAN.md` - Create the `pump_control` component with safe GPIO init, validation, debounce, timer state machine, start/stop, and status API.
3. `01-03-PLAN.md` - Wire `pump_control` into boot without auto-start and run `idf.py build`.

This sequence keeps each step buildable or close to buildable, avoids false parallel dependencies, and preserves existing Wi-Fi/web behavior.

## Research Complete

The codebase has enough local context to plan Phase 1 without external research.
