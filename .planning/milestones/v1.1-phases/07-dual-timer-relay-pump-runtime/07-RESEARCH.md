# Phase 7 Research: Dual Timer Relay Pump Runtime

**Phase:** 07-dual-timer-relay-pump-runtime  
**Date:** 2026-05-23  
**Status:** Complete

## Scope Read

Phase 7 changes the pump runtime from one relay output shared by two timer profiles to two independent pump relay channels selected by the binary float switch:

- `Float ON` selects Timer 1 / Relay 1 and forces Relay 2 OFF.
- `Float OFF` selects Timer 2 / Relay 2 and forces Relay 1 OFF.
- Confirmed float transitions stop the previous channel and restart the newly selected channel from its configured start phase.
- Stopped/disabled pump control keeps both relays OFF while still reading float state and previewing the channel that would run if Start is pressed.

Cooling runtime, GPIO mutation APIs, the full Hardware/Install UI, event logs, charts, cloud features, and production auth hardening remain out of scope.

## Existing Implementation

### Pump Runtime

`components/pump_control/` is the correct owner for Phase 7 runtime behavior. It already contains:

- fixed static runtime state protected by `s_pump_mutex`
- a 100 ms `esp_timer` tick
- float input debounce
- timer phase switching
- safe relay inactive writes
- idempotent `pump_control_start()` and `pump_control_stop()`
- status snapshots via `pump_control_get_status()`

The current design has one `relay_gpio`, one `relay_polarity`, and one `relay_energized` flag. The current timer selection also follows the old v1.0 mapping in code: Float ON selects Timer 2 and Float OFF selects Timer 1. Phase 7 must reverse this to the v1.1 mapping locked in ROADMAP/REQUIREMENTS/CONTEXT.

The periodic callback currently returns early when `!s_running`, so stopped status cannot keep float state fresh. Phase 7 needs the tick to keep reading/debouncing float while initialized but stopped, without energizing relays or advancing timer phases.

### Hardware And Storage Foundation

Phase 6 already supplies the foundation needed by Phase 7:

- `hardware_map_t` includes `float_input_gpio`, `pump_relay1_gpio`, and `pump_relay2_gpio`.
- `nvs_store_pump_settings_t` includes `relay1_active_low`, `relay2_active_low`, `timer1_start_phase`, and `timer2_start_phase`.
- `app_main.c` already loads the active hardware map and pump settings, but currently maps only active float GPIO and Relay 1 into the single-relay runtime.
- `web_server.c` already exposes read-only `relay1_polarity`, `relay2_polarity`, `timer1_start_phase`, and `timer2_start_phase`, but currently rejects start-phase fields in POST `/api/pump/config`.

### Web/API Surface

Existing pump routes should remain:

- `GET /api/pump/config`
- `POST /api/pump/config`
- `GET /api/pump/status`
- `POST /api/pump/start`
- `POST /api/pump/stop`

Phase 7 should preserve compatibility fields such as `relay_gpio`, `relay_polarity`, and `relay_energized`, while adding dual-relay status fields:

- `active_relay: "relay1" | "relay2" | "none"`
- `relay1_energized: bool`
- `relay2_energized: bool`

`relay_energized` should stay as a compatibility alias for the active/ready channel. In stopped state it must be false even when `active_timer`/`active_relay` preview the channel that would start.

PUMP-04 requires the user to configure independent Timer 1 and Timer 2 start phases. The smallest fit is to extend existing pump config save/read and dashboard timer form with `timer1_start_phase` and `timer2_start_phase`, without adding full hardware GPIO mutation APIs.

## Recommended Plan Split

### Plan 07-01: Pump Control Dual-Relay Runtime Core

Extend `pump_control` first because every later layer depends on reliable runtime fields. This plan should:

- add Relay 1 and Relay 2 GPIO/polarity config fields
- initialize both relay GPIOs inactive before runtime starts
- add active relay and per-relay energized status fields
- implement Float ON -> Timer 1 / Relay 1 and Float OFF -> Timer 2 / Relay 2
- respect per-timer start phases
- keep float state fresh while stopped and preview ready timer/phase/countdown
- fail safe by forcing both relays OFF on unsafe relay write/state conditions

### Plan 07-02: Boot And Pump API Integration

Wire the runtime to Phase 6 storage and existing routes:

- merge active hardware map Relay 1/Relay 2 into `pump_control_config_t`
- merge Relay 1/Relay 2 polarity and Timer 1/Timer 2 start phases from NVS
- serialize new status fields
- make `/api/pump/config` accept validated `timer1_start_phase` and `timer2_start_phase`
- preserve old field compatibility and existing auth/same-origin behavior
- preserve safe save semantics: save first, stop safe/reinit/restart only after successful NVS commit

### Plan 07-03: Minimal Dashboard Update And Validation

Update the local UI only enough to support Phase 7:

- correct float mapping text to v1.1
- show Relay 1 and Relay 2 separately
- show ready-to-start wording while stopped
- add Timer 1 and Timer 2 start-phase controls to satisfy PUMP-04
- consume new status fields while tolerating old responses during degraded states
- run ESP-IDF build validation

## Key Risks

- **Relay overlap risk:** runtime helpers must turn the old relay OFF before starting the newly selected channel, and status should make overlap visible.
- **Stopped preview ambiguity:** `active_timer`/`active_relay` can preview ready channel while stopped, but `running:false` and energized fields must make non-running state unambiguous.
- **Start phase edge cases:** `start_phase = OFF` must be a real OFF phase, including on float transitions and after safe config reinit.
- **Compatibility risk:** existing UI expects `relay_energized` and `relay_gpio`; keep them additive rather than replacing them.
- **Scope creep:** do not add cooling runtime, GPIO mutation APIs, install UI, or full dashboard redesign.

## Verification Guidance

Automated validation remains `.\scripts\build.ps1` and `Test-Path build\fish_pump_relay_timer_control.bin`.

Focused source checks should confirm:

- `pump_control_config_t` has Relay 1 and Relay 2 fields.
- both relays are written inactive on init/stop/fault.
- Float ON selects Timer 1/Relay 1 and Float OFF selects Timer 2/Relay 2.
- start phases are consumed in runtime selection.
- `/api/pump/status` includes `active_relay`, `relay1_energized`, and `relay2_energized`.
- dashboard code renders Relay 1/Relay 2 separately and no longer uses old Float ON -> Timer 2 wording.

Manual hardware validation after execution should cover:

1. Float ON selects Timer 1 / Relay 1 and Relay 2 remains OFF.
2. Float OFF selects Timer 2 / Relay 2 and Relay 1 remains OFF.
3. Float transitions stop the old relay before the new channel starts.
4. `start_phase = OFF` begins with the selected relay OFF and countdown from OFF duration.
5. Stop forces both relays OFF while float state continues updating on dashboard.

## RESEARCH COMPLETE
