# Fish Pump Relay Timer Control

## What This Is

Firmware for an ESP32 DevKit V1 30-pin board that controls two fish-pump relay channels from one binary float switch and a separate DS18B20 cooling relay. The local web dashboard is the offline control surface for timer settings, pump operation, cooling settings, guarded hardware mapping, Wi-Fi setup, and runtime diagnostics.

The control logic is intentionally simple and hardware-real: the float switch is an ON/OFF contact, not a continuous water-level sensor. Float ON selects Timer 1/Relay 1, Float OFF selects Timer 2/Relay 2, and only the selected pump relay follows its timer's ON/OFF phase.

## Core Value

The pump must switch reliably between Timer 1 and Timer 2 based on the float switch and drive the relay safely according to the selected timer's ON/OFF cycle.

## Milestone Status

**Latest shipped:** v1.1 Dual Relay Cooling And Install UI on 2026-06-02

## Current Milestone: v1.2 Owner UI Polish And Hardware Readiness

**Goal:** Improve operator confidence and setup usability without changing stable v1.1 firmware behavior.

**Target features:**
- Improve the full local web UI: login, app shell, owner dashboard, Hardware/Install, Wi-Fi setup, and system status.
- Clarify owner dashboard hierarchy, runtime state presentation, and installer guidance.
- Improve responsive layout, accessibility, navigation, copy, and loading/error/empty states across all pages.
- Screenshot-verify UI changes while preserving ESP32 memory and performance safety.
- Close hardware-readiness documentation for the external 4.7 kOhm DS18B20 DQ-to-3.3 V pull-up.
- Use `frontend-ui-ux`, `ui-ux-pro-max`, and `impeccable` as design and review gates, adapted to the offline embedded ESP32 constraints.

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
- ✓ Firmware-defined safe GPIO options exist for float input, pump Relay 1, pump Relay 2, DS18B20 data, and cooling relay — Phase 6
- ✓ Conservative ESP32 DevKit V1 defaults are GPIO32, GPIO26, GPIO27, GPIO33, and GPIO25 for the Phase 6 hardware roles — Phase 6
- ✓ Relay 1, Relay 2, and cooling relay polarity are represented independently in persistent settings — Phase 6
- ✓ DS18B20 powered-mode wiring assumptions are documented with 3.3 V rail and 4.7 kOhm pull-up — Phase 6
- ✓ Active and pending GPIO maps, relay polarity, cooling settings, and timer start phases are stored through documented NVS schemas — Phase 6
- ✓ Pump control uses two separate timer/relay channels selected by binary float state — v1.1
- ✓ Float state changes restart the selected timer cycle and force the previously selected relay OFF — v1.1
- ✓ Temperature cooling control uses DS18B20 and a dedicated relay separate from pump relays — v1.1
- ✓ Cooling control provides explicit enable, auto-enable preference, safe override modes, and compressor protection — v1.1
- ✓ Web Hardware/Install GPIO map exposes firmware-defined safe enum options and pending reboot behavior — v1.1

### Active

- Improve login, app shell, owner dashboard, Hardware/Install, Wi-Fi setup, and system status UI without changing stable relay/timer/cooling behavior.
- Improve navigation, visual hierarchy, operator confidence, and setup guidance for daily and installer workflows.
- Improve responsive layout, accessibility, copy, and loading/error/empty states across every local UI page.
- Screenshot-verify UI changes and keep the embedded frontend memory/performance safe for ESP32.
- Close documentation for the external 4.7 kOhm DS18B20 DQ-to-3.3 V pull-up requirement.

### Out of Scope

- Continuous 0-100% water-level measurement — the real float switch is binary only.
- Multiple float switches or analog level sensors — current design has one binary float switch.
- Relay pin assignment as a hidden hard-code with no configurability — module polarity and wiring need to be explicit.
- OTA update, MQTT/cloud, charts, WebSocket, HTTPS, and multi-user management — not needed for the local controller milestone.
- Production security hardening beyond the existing local prototype model — can be handled in a later hardening pass.

## Current State

**Shipped:** v1.1 Dual Relay Cooling And Install UI on 2026-06-02
**Current milestone:** v1.2 Owner UI Polish And Hardware Readiness

The firmware is a complete local ESP32 pump and cooling controller with:
- GPIO32 float input selecting GPIO26 Relay 1 or GPIO27 Relay 2 timer channels
- Dedicated GPIO33 DS18B20 cooling input and GPIO25 cooling relay control
- NVS-backed pump, cooling, polarity, and active/pending hardware-map settings
- Authenticated Thai-language owner dashboard and Hardware/Install page
- SoftAP fallback, STA configuration, captive DNS, and bounded long-uptime diagnostics
- Real-board hardware UAT and a `13:38:10` soak with no reboot, watchdog trip, or monotonic heap loss

**Codebase:** ESP-IDF C firmware with embedded HTML, CSS, JS, CMake, and docs. Build-valid with ESP-IDF 6.0.1.
**Partition usage:** 45% free in dual OTA app slots (`0x1F0000` each on 4MB flash).
**Binary:** `build/fish_pump_relay_timer_control.bin` generates successfully.

## Context

The codebase is a brownfield ESP-IDF project with a reusable local web setup foundation. Codebase mapping exists under `.planning/codebase/` and shows clean component boundaries for app config, NVS storage, session handling, Wi-Fi management, the HTTP server, captive DNS, and static UI assets.

Recommended hardware contract (validated):

- Board: ESP32 DevKit V1 30-pin, classic ESP32.
- Float switch input: GPIO32, internal pull-up enabled, float switch closes to GND when active.
- Pump Relay 1 output: GPIO26, default inactive at boot, polarity configurable.
- Pump Relay 2 output: GPIO27, default inactive at boot, polarity configurable.
- DS18B20 data: GPIO33, powered mode, external 4.7 kOhm pull-up to 3.3 V.
- Cooling relay output: GPIO25, default inactive at boot, polarity configurable.
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
- **Stable baseline**: Treat v1.1 relay, timer, cooling, and APSTA behavior as stable; do not rewrite firmware logic unless a UI state bug requires it.
- **Soak preservation**: Preserve the passed `13:38:10` Wi-Fi/APSTA hardware soak behavior.
- **UI verification**: Screenshot-verify UI changes and keep the embedded frontend memory/performance safe for ESP32.
- **UI workflow**: Use `frontend-ui-ux`, `ui-ux-pro-max`, and `impeccable` during UI design and review. Apply their product-dashboard guidance within this project's no-CDN, no-framework-migration, embedded-asset constraints.
- **Bookkeeping debt**: The eight deferred scanner rows are bookkeeping debt only, not unfinished firmware work.

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
| Make `hardware_map` the single source for safe role options | Later UI/API phases need enum-backed GPIO choices instead of freeform numeric pins | Validated in Phase 6 |
| Store pending hardware map separately from active map | GPIO changes need reboot and wiring confirmation before becoming runtime-active | Validated in Phase 6 |
| Keep Relay 2, DS18B20, and cooling relay read-only/inactive in Phase 6 | Runtime behavior belongs to later scoped phases | Validated as Phase 6 staging; activated in Phases 7-8 |
| Use separate pump relay channels selected by binary float state | Each timer now maps to a physical output and the previous channel must stop immediately on float change | Validated in Phase 7 and hardware UAT |
| Keep cooling fail-safe OFF on missing or unreadable DS18B20 data | Sensor failure must not energize cooling indefinitely | Validated in Phase 8 and hardware UAT |
| Keep GPIO edits pending until reboot confirmation | Runtime pin remapping during operation is unsafe for relay wiring | Validated in Phase 9 and Hardware/Install UAT |
| Instrument bounded serial diagnostics instead of scheduled reboot | The final soak showed stable uptime; diagnostics preserve evidence for future regressions | Validated by the 13:38:10 soak |

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
*Last updated: 2026-06-02 after starting v1.2 milestone planning*
