---
generated: 2026-05-18
type: project-research
dimension: architecture
---

# Research: Architecture

## Component Boundary

Add `components/pump_control/` for hardware and state-machine logic. Keep `main/web_server.c` as routing/API glue and `main/static/` as UI only.

## State Machine

States:

- stopped
- running with active timer `t1` or `t2`
- current phase `on` or `off`
- countdown seconds remaining

Inputs:

- float switch GPIO level
- saved timer configuration
- start/stop commands
- auto-start setting

Outputs:

- relay GPIO level
- status struct for API/UI

## Loop Model

Use a 1-second control tick. On each tick:

1. Read debounced float state.
2. Select timer from float state.
3. If selected timer changed, reset phase to ON.
4. Decrement countdown.
5. If countdown expires, toggle phase and reload duration.
6. Set relay to active polarity only during ON phase.

## Persistence

Use NVS for:

- Timer 1 ON seconds
- Timer 1 OFF seconds
- Timer 2 ON seconds
- Timer 2 OFF seconds
- Auto-start enabled
- Relay polarity

Consider a separate namespace such as `pump_cfg` to avoid mixing pump settings with Wi-Fi keys in `wifi_cfg`.

## API Surface

Recommended routes:

- `GET /pump` or dashboard section for UI
- `GET /api/pump/config`
- `POST /api/pump/config`
- `POST /api/pump/start`
- `POST /api/pump/stop`
- `GET /api/pump/status`

All routes except static CSS/JS and login should remain authenticated like current dashboard/status/wifi routes.

## Build Order

1. Add pump settings storage and config defaults.
2. Add pump control component with state machine and simulated/unit-like internal state where possible.
3. Add hardware GPIO initialization and relay inactive safety.
4. Add HTTP APIs.
5. Add UI.
6. Build and hardware-test.

