# Fish Pump Relay Timer Control

## What This Is

Firmware for an ESP32 DevKit V1 30-pin board that controls a fish pump relay using two configurable timers and one binary float switch. The existing Wi-Fi setup and local web dashboard foundation will become the control surface for setting timer durations, starting/stopping the controller, saving boot behavior, and monitoring float/relay/timer state.

The control logic is intentionally simple and hardware-real: the float switch is an ON/OFF contact, not a continuous water-level sensor. Float OFF selects Timer 1, Float ON selects Timer 2, and the relay follows the active timer's ON/OFF phase.

## Core Value

The pump must switch reliably between Timer 1 and Timer 2 based on the float switch and drive the relay safely according to the selected timer's ON/OFF cycle.

## Requirements

### Validated

- ✓ Device boots as ESP-IDF firmware on classic ESP32 target — existing
- ✓ Local web server serves embedded HTML/CSS/JS without external dependencies — existing
- ✓ Login/session flow protects dashboard, status, Wi-Fi, and API routes — existing
- ✓ SoftAP fallback and Wi-Fi STA setup are available through the local UI — existing
- ✓ Wi-Fi credentials and optional static IP configuration persist in NVS — existing
- ✓ Device status JSON reports system, Wi-Fi, heap, uptime, and service state — existing

### Active

- [ ] User can configure Timer 1 ON/OFF durations from the web UI.
- [ ] User can configure Timer 2 ON/OFF durations from the web UI.
- [ ] Timer settings persist in NVS and survive reboot.
- [ ] User can start and stop pump control from the web UI.
- [ ] User can choose whether pump control auto-starts on boot; default is auto-start enabled.
- [ ] Firmware reads a binary float switch input and treats switch-to-GND as active-low.
- [ ] Float OFF selects Timer 1; Float ON selects Timer 2.
- [ ] Switching float state resets the newly selected timer to its ON phase.
- [ ] Relay output energizes during ON phase and de-energizes during OFF phase.
- [ ] Relay GPIO and relay polarity are configurable because relay modules may be active-high or active-low.
- [ ] Dashboard displays active timer, current phase, countdown, float state, relay state, and auto-start state.
- [ ] Firmware preserves AP/Wi-Fi setup access while pump control is running.

### Out of Scope

- Continuous 0-100% water-level measurement — the real float switch is binary only.
- Multiple float switches or analog level sensors — current design has one binary float switch.
- Relay pin assignment as a hidden hard-code with no configurability — module polarity and wiring need to be explicit.
- OTA update, MQTT/cloud, charts, WebSocket, HTTPS, and multi-user management — not needed for the local controller milestone.
- Production security hardening beyond the existing local prototype model — can be handled in a later hardening pass.

## Context

The current codebase is a brownfield ESP-IDF project with a reusable local web setup foundation. Codebase mapping exists under `.planning/codebase/` and shows clean component boundaries for app config, NVS storage, session handling, Wi-Fi management, the HTTP server, captive DNS, and static UI assets.

The user supplied `ref-file/flow-document-v6.md` and `ref-file/flow-diagram-5.html` as the design reference for the next product behavior. The flow defines a Dual Timer + Float Switch + Relay controller:

- Timer 1 default: ON 20 seconds, OFF 1 minute.
- Timer 2 default: ON 10 seconds, OFF 3 minutes.
- Float switch is binary only.
- Float OFF / water below switch point selects Timer 1.
- Float ON / water above switch point selects Timer 2.
- Each timer alternates ON and OFF phases.
- Relay is energized only during ON phase.
- A UI simulation exists in `ref-file/flow-diagram-5.html`, but the milestone target is real ESP32 firmware controlling GPIO, not just a prototype.

Recommended initial hardware contract:

- Board: ESP32 DevKit V1 30-pin, classic ESP32.
- Float switch input: GPIO32, internal pull-up enabled, float switch closes to GND when active.
- Relay output: GPIO26, default inactive at boot, polarity configurable.
- Avoid ESP32 flash pins, UART programming pins, input-only pins for relay output, and boot strapping pins for initial relay/float defaults.

## Constraints

- **Framework**: ESP-IDF only — the repository is not Arduino or PlatformIO.
- **Board**: ESP32 DevKit V1 30-pin / classic ESP32 — pin recommendations and build target assume this board.
- **Float input**: Binary switch only — do not model real hardware as continuous water level.
- **GPIO safety**: Use conservative default pins and configurable polarity — relay modules vary and wrong polarity can energize the pump unexpectedly.
- **Boot behavior**: Auto-start is enabled by default but must be user-configurable and persisted.
- **Local operation**: UI must work without internet because SoftAP setup mode has no external connectivity.
- **Existing foundation**: Preserve Wi-Fi setup, SoftAP fallback, login/session, captive DNS, and status routes while adding pump control.
- **Validation**: `idf.py build` remains the main automated validation gate; hardware behavior needs manual flash/device testing.

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Build real firmware rather than only UI simulation | User selected hardware control as the target milestone | — Pending |
| Use ESP32 DevKit V1 30-pin classic ESP32 | User confirmed board variant | — Pending |
| Recommend GPIO32 for float switch input with pull-up to GND | GPIO32 is a conservative input-capable pin with internal pull-up support and avoids boot strapping pins | — Pending |
| Recommend GPIO26 for relay output | GPIO26 is a general-purpose output and avoids common boot/programming pins | — Pending |
| Make relay polarity configurable | Relay modules are often active-low, so firmware must not assume one electrical polarity forever | — Pending |
| Auto-start pump control by default, with persisted user override | User wants automatic boot operation but wants the ability to disable it | — Pending |
| Default Timer 1 is ON 20s / OFF 1:00 and Timer 2 is ON 10s / OFF 3:00 | User corrected defaults from the reference flow document | — Pending |

## Evolution

This document evolves at phase transitions and milestone boundaries.

**After each phase transition** (via `$gsd-transition`):
1. Requirements invalidated? -> Move to Out of Scope with reason
2. Requirements validated? -> Move to Validated with phase reference
3. New requirements emerged? -> Add to Active
4. Decisions to log? -> Add to Key Decisions
5. "What This Is" still accurate? -> Update if drifted

**After each milestone** (via `$gsd-complete-milestone`):
1. Full review of all sections
2. Core Value check - still the right priority?
3. Audit Out of Scope - reasons still valid?
4. Update Context with current state

---
*Last updated: 2026-05-18 after initialization*
