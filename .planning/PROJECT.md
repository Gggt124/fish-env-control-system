# Fish Pump Relay Timer Control

## What This Is

Firmware for an ESP32 DevKit V1 30-pin board that controls a fish pump relay using two configurable timers and one binary float switch. The existing Wi-Fi setup and local web dashboard foundation will become the control surface for setting timer durations, starting/stopping the controller, saving boot behavior, and monitoring float/relay/timer state.

The control logic is intentionally simple and hardware-real: the float switch is an ON/OFF contact, not a continuous water-level sensor. Float OFF selects Timer 1, Float ON selects Timer 2, and the relay follows the active timer's ON/OFF phase.

## Core Value

The pump must switch reliably between Timer 1 and Timer 2 based on the float switch and drive the relay safely according to the selected timer's ON/OFF cycle.

## Current Milestone: v1.1 Dual Relay Cooling And Install UI

**Goal:** Extend the validated pump controller into a safer installable system with two timer/relay channels, a separate DS18B20 cooling channel, and guarded owner-accessible hardware configuration.

**Target features:**
- Dual timer/relay pump control where Float ON selects Timer 1/Relay 1 and Float OFF selects Timer 2/Relay 2.
- Float changes interrupt the active timer, force the old relay OFF, and restart the newly selected timer from its configured start phase.
- DS18B20 cooling control on a dedicated relay with threshold, hysteresis, sensor fault safe-off, and compressor-protection-aware override modes.
- Hardware/Install web flow with safe GPIO dropdown enums, active vs pending GPIO maps, confirmation guardrails, and reboot-required status.
- Owner dashboard remains focused on daily operation while showing pump, cooling, temperature, relay, countdown, and sensor fault state.

## Requirements

### Validated

- ✓ Device boots as ESP-IDF firmware on classic ESP32 target — existing
- ✓ Local web server serves embedded HTML/CSS/JS without external dependencies — existing
- ✓ Login/session flow protects dashboard, status, Wi-Fi, and API routes — existing
- ✓ SoftAP fallback and Wi-Fi STA setup are available through the local UI — existing
- ✓ Wi-Fi credentials and optional static IP configuration persist in NVS — existing
- ✓ Device status JSON reports system, Wi-Fi, heap, uptime, and service state — existing
- ✓ Pump control core initializes GPIO26 relay output inactive during boot and leaves control stopped — Phase 1
- ✓ Pump control core defines GPIO32 active-low binary float input with debounce for timer selection — Phase 1
- ✓ Pump control core implements Timer 1/Timer 2 ON/OFF defaults, bounds, and relay phase behavior — Phase 1
- ✓ ESP-IDF build succeeds with pump control integrated — Phase 1
- ✓ Timer 1/Timer 2 ON/OFF settings persist in NVS through the `pump_cfg` namespace — Phase 2
- ✓ Pump boot behavior loads persisted settings before pump initialization and defaults safely when missing — Phase 2
- ✓ Auto-start is persisted at the firmware layer and disabled saved settings suppress boot start — Phase 2
- ✓ Authenticated `/api/pump/config` returns and saves validated pump settings while keeping GPIO/debounce read-only — Phase 3
- ✓ Authenticated `/api/pump/status` returns machine-friendly pump runtime state — Phase 3
- ✓ Authenticated `/api/pump/start` and `/api/pump/stop` provide idempotent runtime controls with same-origin POST protection — Phase 3
- ✓ User can configure Timer 1 ON/OFF durations from the web UI — Phase 4
- ✓ User can configure Timer 2 ON/OFF durations from the web UI — Phase 4
- ✓ User can start and stop pump control from the web UI — Phase 4
- ✓ User can choose whether pump control auto-starts on boot from authenticated API and web UI controls — Phase 4
- ✓ Dashboard displays active timer, current phase, countdown, float state, relay state, and auto-start state — Phase 4
- ✓ Firmware preserves AP/Wi-Fi setup access while pump control is running — Phase 4
- ✓ Manual hardware test confirms float OFF selects Timer 1 and float ON selects Timer 2 — Phase 5
- ✓ Manual hardware test confirms relay follows ON/OFF phase and Stop forces inactive — Phase 5
- ✓ Manual reboot test confirms timer settings and auto-start preference persist — Phase 5
- ✓ Manual access test confirms Wi-Fi setup, SoftAP fallback, login, and status pages still work while pump control is present — Phase 5

### Active

- [ ] Pump control uses two separate timer/relay channels selected by binary float state.
- [ ] Float state changes restart the selected timer cycle and force the previously selected relay OFF.
- [ ] Temperature cooling control uses DS18B20 and a dedicated relay separate from pump relays.
- [ ] Cooling control provides explicit enable, auto-enable preference, safe override modes, and compressor protection.
- [ ] Web Hardware/Install GPIO map uses firmware-defined safe enum options and pending reboot behavior.

### Out of Scope

- Continuous 0-100% water-level measurement — the real float switch is binary only.
- Multiple float switches or analog level sensors — current design has one binary float switch.
- Relay pin assignment as a hidden hard-code with no configurability — module polarity and wiring need to be explicit.
- OTA update, MQTT/cloud, charts, WebSocket, HTTPS, and multi-user management — not needed for the local controller milestone.
- Production security hardening beyond the existing local prototype model — can be handled in a later hardening pass.

## Current State

**Shipped:** v1.0 MVP on 2026-05-20

The firmware is a complete ESP32 pump relay controller with:
- Hardware-safe pump control core (GPIO32 float, GPIO26 relay, active-low defaults)
- Two configurable timers with NVS persistence and reboot survival
- Authenticated web API for config, status, start, and stop
- Local Thai-language dashboard with live status polling and countdown
- Manual hardware validation passed on real ESP32 DevKit V1

**Codebase:** ~44,900 lines across C, HTML, CSS, JS, CMake, and docs. Build-valid with ESP-IDF 6.1.
**Partition usage:** 49% of dual OTA app slots (`0x1F0000` each on 4MB flash).
**Binary:** `build/fish_pump_relay_timer_control.bin` generates successfully.

## Context

The codebase is a brownfield ESP-IDF project with a reusable local web setup foundation. Codebase mapping exists under `.planning/codebase/` and shows clean component boundaries for app config, NVS storage, session handling, Wi-Fi management, the HTTP server, captive DNS, and static UI assets.

Recommended hardware contract (validated):

- Board: ESP32 DevKit V1 30-pin, classic ESP32.
- Float switch input: GPIO32, internal pull-up enabled, float switch closes to GND when active.
- Relay output: GPIO26, default inactive at boot, polarity configurable.
- Avoid ESP32 flash pins, UART programming pins, input-only pins for relay output, and boot strapping pins for initial relay/float defaults.

## Constraints

- **Framework**: ESP-IDF only — the repository is not Arduino or PlatformIO.
- **Board**: ESP32 DevKit V1 30-pin / classic ESP32 — pin recommendations and build target assume this board.
- **Float input**: Binary switch only — do not model real hardware as continuous water level.
- **GPIO safety**: Use conservative default pins and configurable polarity — relay modules vary and wrong polarity can energize the pump unexpectedly.
- **Boot behavior**: Auto-start is disabled by default but must be user-configurable and persisted.
- **Local operation**: UI must work without internet because SoftAP setup mode has no external connectivity.
- **Existing foundation**: Preserve Wi-Fi setup, SoftAP fallback, login/session, captive DNS, and status routes while adding pump control.
- **Validation**: `idf.py build` remains the main automated validation gate; hardware behavior needs manual flash/device testing.

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Build real firmware rather than only UI simulation | User selected hardware control as the target milestone | Validated in Phase 1 core |
| Use ESP32 DevKit V1 30-pin classic ESP32 | User confirmed board variant | Validated by Phase 1 build target |
| Recommend GPIO32 for float switch input with pull-up to GND | GPIO32 is a conservative input-capable pin with internal pull-up support and avoids boot strapping pins | Validated as Phase 1 source default |
| Recommend GPIO26 for relay output | GPIO26 is a general-purpose output and avoids common boot/programming pins | Validated as Phase 1 source default |
| Make relay polarity configurable | Relay modules are often active-low, so firmware must not assume one electrical polarity forever | Validated in Phase 1 pump config |
| Auto-start pump control disabled by default, with persisted user override | Safer boot behavior after hardware validation; user can enable via UI | Changed during quick task 260520-s4a; firmware persistence validated in Phase 2; authenticated API validated in Phase 3; UI controls validated in Phase 4 |
| Default Timer 1 is ON 20s / OFF 1:00 and Timer 2 is ON 10s / OFF 3:00 | User corrected defaults from the reference flow document | Validated as Phase 1 source defaults |
| Keep pump APIs under `/api/pump/*` | Separates pump control contracts from existing system/Wi-Fi status routes | Validated in Phase 3 |
| Make `/dashboard` the pump control surface | Operator needs timer setup, Start/Stop, and live runtime state before system diagnostics | Validated in Phase 4 |
| Hardware validation on real ESP32 DevKit V1 with GPIO32/GPIO26 wiring | Confirmed float switching, relay phase behavior, Stop safety, and reboot persistence on actual device | Validated in Phase 5 |

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
*Last updated: 2026-05-22 after v1.1 milestone start*
