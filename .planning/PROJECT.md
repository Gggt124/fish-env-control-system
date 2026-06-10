# Fish Pump Relay Timer Control

## What This Is

Firmware for an ESP32 DevKit V1 30-pin board that controls two fish-pump relay channels from one binary float switch and a separate DS18B20 cooling relay. The local web dashboard is a professional, accessible offline control surface for timer settings, pump operation, cooling settings, guarded hardware mapping, Wi-Fi setup, and runtime diagnostics — with clear operator hierarchy, responsive mobile layout, and consistent state communication.

The control logic is intentionally simple and hardware-real: the float switch is an ON/OFF contact, not a continuous water-level sensor. Float ON selects Timer 1/Relay 1, Float OFF selects Timer 2/Relay 2, and only the selected pump relay follows its timer's ON/OFF phase.

## Core Value

The pump must switch reliably between Timer 1 and Timer 2 based on the float switch and drive the relay safely according to the selected timer's ON/OFF cycle.

## Milestone Status

**Latest shipped:** v1.5 TFT Display Integration on 2026-06-06

## Current Milestone: v1.7 Authentication & Recovery

**Goal:** Improve access control by adding persistent sessions, user credential management, and a recovery mechanism for forgotten credentials.

**Target features:**
- AUTH-01: "Remember Me" persistent session to bypass login on trusted devices.
- AUTH-02: User interface to change Username & Password.
- AUTH-03: Credential Recovery mechanism (e.g., hardware button reset or fallback) to prevent lockout.

## Requirements

### Validated

- ✓ Device boots as ESP-IDF firmware on classic ESP32 target — existing
- ✓ Local web server serves embedded HTML/CSS/JS without external dependencies — existing
- ✓ Login/session flow protects dashboard, status, Wi-Fi, and API routes — existing
- ✓ SoftAP fallback and Wi-Fi STA setup are available through the local UI — existing
- ✓ Wi-Fi credentials and optional static IP configuration persist in NVS — existing
- ✓ Device status JSON reports system, Wi-Fi, heap, uptime, and service state — existing
- ✓ Pump control core initializes GPIO26 relay output inactive during boot and leaves control stopped — v1.0
- ✓ Pump control core defines GPIO32 active-low binary float input with debounce for timer selection — v1.0
- ✓ Pump control core implements Timer 1/Timer 2 ON/OFF defaults, bounds, and relay phase behavior — v1.0
- ✓ ESP-IDF build succeeds with pump control integrated — v1.0
- ✓ Timer 1/Timer 2 ON/OFF settings persist in NVS through the `pump_cfg` namespace — v1.0
- ✓ Pump boot behavior loads persisted settings before pump initialization and defaults safely when missing — v1.0
- ✓ Auto-start is persisted at the firmware layer and disabled saved settings suppress boot start — v1.0
- ✓ Authenticated `/api/pump/config` returns and saves validated pump settings while keeping GPIO/debounce read-only — v1.0
- ✓ Authenticated `/api/pump/status` returns machine-friendly pump runtime state — v1.0
- ✓ Authenticated `/api/pump/start` and `/api/pump/stop` provide idempotent runtime controls with same-origin POST protection — v1.0
- ✓ User can configure Timer 1 and Timer 2 ON/OFF durations from the web UI — v1.0
- ✓ User can start and stop pump control from the web UI — v1.0
- ✓ User can choose whether pump control auto-starts on boot from authenticated API and web UI controls — v1.0
- ✓ Dashboard displays active timer, current phase, countdown, float state, relay state, and auto-start state — v1.0
- ✓ Firmware preserves AP/Wi-Fi setup access while pump control is running — v1.0
- ✓ Manual hardware test confirms float OFF selects Timer 1 and float ON selects Timer 2 — v1.0
- ✓ Manual hardware test confirms relay follows ON/OFF phase and Stop forces inactive — v1.0
- ✓ Manual reboot test confirms timer settings and auto-start preference persist — v1.0
- ✓ Manual access test confirms Wi-Fi setup, SoftAP fallback, login, and status pages still work while pump control is present — v1.0
- ✓ Firmware-defined safe GPIO options exist for float input, pump Relay 1, pump Relay 2, DS18B20 data, and cooling relay — v1.1
- ✓ Conservative ESP32 DevKit V1 defaults are GPIO32, GPIO26, GPIO27, GPIO33, and GPIO25 for the hardware roles — v1.1
- ✓ Relay 1, Relay 2, and cooling relay polarity are represented independently in persistent settings — v1.1
- ✓ DS18B20 powered-mode wiring assumptions are documented with 3.3 V rail and 4.7 kOhm pull-up — v1.1
- ✓ Active and pending GPIO maps, relay polarity, cooling settings, and timer start phases are stored through documented NVS schemas — v1.1
- ✓ Pump control uses two separate timer/relay channels selected by binary float state — v1.1
- ✓ Float state changes restart the selected timer cycle and force the previously selected relay OFF — v1.1
- ✓ Temperature cooling control uses DS18B20 and a dedicated relay separate from pump relays — v1.1
- ✓ Cooling control provides explicit enable, auto-enable preference, safe override modes, and compressor protection — v1.1
- ✓ Web Hardware/Install GPIO map exposes firmware-defined safe enum options and pending reboot behavior — v1.1
- ✓ Baseline UI audit, Thai-first shared state language, offline ESP32 checklist, and scoped implementation briefs are established before frontend edits — v1.2
- ✓ Owner dashboard clearly presents pump state, selected timer/relay, countdown, float switch, and cooling as separate operational channels — v1.2
- ✓ Consistent app shell navigation with mobile drawer across all authenticated pages — v1.2
- ✓ Login presents clear credential entry, loading, error, and recovery guidance — v1.2
- ✓ Hardware/Install distinguishes active, pending, and reboot-required GPIO state with DS18B20 pull-up guidance — v1.2
- ✓ Wi-Fi scan/connect flow presents loading, error, empty, disconnected, and connected states — v1.2
- ✓ Keyboard focus visibility and assistive-technology status announcements across all pages — v1.2
- ✓ Professional CSS overhaul with HSL color system, accessible contrast, no CDN or external assets — v1.2
- ✓ Full visual regression validated with screenshots, accessibility audit, build/footprint gate, and device-backed hardware regression — v1.2
- ✓ UI-17: Only list changed GPIO pins in the Pending Reboot Map display — v1.3
- ✓ UI-18: GPIO map unsaved warning disappears and button disables on value reversion — v1.3
- ✓ UI-19: Wi-Fi disconnect button simplified to "Disconnect" / "Disconnecting..." — v1.3
- ✓ UI-20: Wi-Fi disconnect requires confirmation dialog — v1.3
- ✓ UI-21: Pump and Cooling Settings save buttons disabled by default — v1.3
- ✓ UI-22: Pump and Cooling Settings forms unsaved warning and button disable on value reversion — v1.3
- ✓ UI-23: Logout menu requires confirmation dialog — v1.3
- ✓ UI-24: "Force OFF" cooling mode button styled red danger button (btn-danger) — v1.3
- ✓ WIFI-UI-01: Hidden credentials panel when no network selected, styled Empty State Card — v1.4
- ✓ WIFI-UI-02: Clicking scan network fades out empty state and fades in credentials panel — v1.4
- ✓ WIFI-UI-03: Clicking Cancel button restores empty state card — v1.4
- ✓ WIFI-UI-04: Style system uses native CSS variables and strictly avoids GPU-heavy backdrop blur — v1.4
- ✓ CODE-REV-01: Review C firmware source files for warnings, logical bugs, and race conditions — v1.4
- ✓ CODE-REV-02: Review JS/HTML assets for console errors, formatting, and leftover debug statements — v1.4
- ✓ CODE-REV-03: Safely resolve all identified issues while maintaining stable operation — v1.4
- ✓ TFT-01: Initialize SPI bus (VSPI) and native `esp_lcd` driver for ILI9341 display. — v1.5
- ✓ TFT-02: Implement a lightweight display rendering module for custom fonts and shapes. — v1.5
- ✓ TFT-03: Render the landscape dashboard showing pump state, active timer, countdown, temperature, cooling relay state, float switch, and Wi-Fi IP. — v1.5
- ✓ TFT-04: Update the screen periodically (e.g., every 500ms or on state change) without blocking the main event loop or watchdog. — v1.5
- ✓ UI-01: Implement Single Page Application (SPA) architecture for smoother transitions. — v1.6
- ✓ UI-02: Update styles with HSL color system, system fonts, and inline SVG icons (Zero External Dependencies). — v1.6
- ✓ UI-03: Redesign Pump Control, Timer Settings, and Cooling Control dashboards to a modern card-based layout. — v1.6
- ✓ UI-04: Overhaul Wi-Fi Setup page with stepper, loading states, and modal popups. — v1.6
- ✓ UI-05: Redesign System Status and Hardware/Install pages for better readability and UX. — v1.6
- ✓ UI-06: Add UI feedback mechanisms (loading spinners, debouncing, modal confirmation). — v1.6

### Active

- [ ] AUTH-01: Implement "Remember Me" persistent session system.
- [ ] AUTH-02: Implement change Username & Password functionality.
- [ ] AUTH-03: Implement Credential Recovery mechanism.

### Out of Scope

- Continuous 0-100% water-level measurement — the real float switch is binary only.
- Multiple float switches or analog level sensors — current design has one binary float switch.
- Relay pin assignment as a hidden hard-code with no configurability — module polarity and wiring need to be explicit.
- OTA update, MQTT/cloud, charts, WebSocket, HTTPS, and multi-user management — not needed for the local controller milestone.
- Production security hardening beyond the existing local prototype model — can be handled in a later hardening pass.
- Event log and export features — deferred until after current-state operator clarity is validated.
- Frontend framework migration, CDN, remote fonts, or remote icon assets — embedded UI must remain offline-capable.

## Current State

**Shipped:** v1.5 TFT Display Integration on 2026-06-06

The firmware is a complete local ESP32 pump and cooling controller with:
- GPIO32 float input selecting GPIO26 Relay 1 or GPIO27 Relay 2 timer channels
- Dedicated GPIO33 DS18B20 cooling input and GPIO25 cooling relay control
- NVS-backed pump, cooling, polarity, and active/pending hardware-map settings
- Professional, accessible Thai-language owner dashboard with operator-first hierarchy
- Consistent mobile-responsive app shell with drawer navigation
- Hardware/Install page with active/pending GPIO map and DS18B20 pull-up guidance
- Wi-Fi setup page polished with a CSS-styled Empty State Card and sequential 0.2s fade-in/fade-out panel switching (with instant transition for prefers-reduced-motion)
- SoftAP fallback, STA configuration, captive DNS, and bounded long-uptime diagnostics
- Interactive forms dirty checking, Thai confirmation dialogs, and simplified buttons to improve UX
- Duration-based Test ON countdown timer that pauses during active compressor protection lockout
- Isolated Auto mode demand state using s_auto_demand to prevent stuck cooling relay on transitions
- Code reviewed, warning-free build, aligned task watchdog configuration and comments (10s watchdog timeout fed every 5s)
- Real-board hardware UAT and a `13:38:10` soak with no reboot, watchdog trip, or monotonic heap loss

**Codebase:** ESP-IDF C firmware with embedded HTML, CSS, JS, CMake, and docs. Build-valid with ESP-IDF 6.0.1.
**Partition usage:** 44% free in dual OTA app slots (`0x1F0000` each on 4MB flash).
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
- **Boot behavior**: Auto-start is disabled by default but must be user-toggleable and persisted.
- **Local operation**: UI must work without internet because SoftAP setup mode has no external connectivity.
- **Existing foundation**: Preserve Wi-Fi setup, SoftAP fallback, login/session, captive DNS, and status routes while adding pump control.
- **Validation**: `idf.py build` remains the main automated validation gate; hardware behavior needs manual flash/device testing.
- **Stable baseline**: Treat v1.2 relay, timer, cooling, UI, and APSTA behavior as stable.
- **Soak preservation**: Preserve the passed `13:38:10` Wi-Fi/APSTA hardware soak behavior.

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Build real firmware rather than only UI simulation | User selected hardware control as the target milestone | ✓ Good |
| Use ESP32 DevKit V1 30-pin classic ESP32 | User confirmed board variant | ✓ Good |
| Recommend GPIO32 for float switch input with pull-up to GND | GPIO32 is a conservative input-capable pin with internal pull-up support and avoids boot strapping pins | ✓ Good |
| Recommend GPIO26 for relay output | GPIO26 is a general-purpose output and avoids common boot/programming pins | ✓ Good |
| Make relay polarity configurable | Relay modules are often active-low, so firmware must not assume one electrical polarity forever | ✓ Good |
| Auto-start pump control disabled by default, with persisted user override | Safer boot behavior after hardware validation; user can enable via UI | ✓ Good |
| Default Timer 1 is ON 20s / OFF 1:00 and Timer 2 is ON 10s / OFF 3:00 | User corrected defaults from the reference flow document | ✓ Good |
| Keep pump APIs under `/api/pump/*` | Separates pump control contracts from existing system/Wi-Fi status routes | ✓ Good |
| Make `/dashboard` the pump control surface | Operator needs timer setup, Start/Stop, and live runtime state before system diagnostics | ✓ Good |
| Hardware validation on real ESP32 DevKit V1 with GPIO32/GPIO26 wiring | Confirmed float switching, relay phase behavior, Stop safety, and reboot persistence on actual device | ✓ Good |
| Make `hardware_map` the single source for safe role options | Later UI/API phases need enum-backed GPIO choices instead of freeform numeric pins | ✓ Good |
| Store pending hardware map separately from active map | GPIO changes need reboot and wiring confirmation before becoming runtime-active | ✓ Good |
| Keep Relay 2, DS18B20, and cooling relay read-only/inactive in Phase 6 | Runtime behavior belongs to later scoped phases | ✓ Good |
| Use separate pump relay channels selected by binary float state | Each timer now maps to a physical output and the previous channel must stop immediately on float change | ✓ Good |
| Keep cooling fail-safe OFF on missing or unreadable DS18B20 data | Sensor failure must not energize cooling indefinitely | ✓ Good |
| Keep GPIO edits pending until reboot confirmation | Runtime pin remapping during operation is unsafe for relay wiring | ✓ Good |
| Instrument bounded serial diagnostics instead of scheduled reboot | The final soak showed stable uptime; diagnostics preserve evidence for future regressions | ✓ Good |
| Audit and define shared UI state language before editing embedded assets | A no-edit baseline makes visual, accessibility, offline, and footprint changes traceable | ✓ Good |
| CSS-only overhaul within offline embedded constraints | No CDN, web fonts, or frameworks — keeps UI offline-capable and ESP32-safe | ✓ Good |
| Separate pump and cooling operational channels on dashboard | Operator needs to distinguish daily pump control from cooling system at a glance | ✓ Good |
| Active vs pending GPIO map visual separation | Installer needs to see what is live vs what requires reboot before wiring changes | ✓ Good |
| Use esp_timer_get_time() to calculate elapsed time in milliseconds for the test timer rather than a static deadline | Allows the Test ON countdown to be selectively paused during active compressor protection lockout | ✓ Good |
| Isolate Auto mode demand tracking using a dedicated s_auto_demand variable | Ensures mode transitions (like from Test ON back to Auto) immediately evaluate conditions without being biased by the physical relay's prior state | ✓ Good |
| Replace CPU-heavy backdrop blur panel with Empty State Card and sequential fade transitions | Solves low-power GPU repaint issues and improves UX visual clarity | ✓ Good |
| Align task watchdog timeout default to 10s and feed loop at 5s | Resolves configuration comment drift and provides conservative CPU starvation protection | ✓ Good |

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
*Last updated: 2026-06-11 for active milestone v1.7 initiation*
