# Milestones: Fish Pump Relay Timer Control

## v1.6 Modern Web UI Optimization (Shipped: 2026-06-10)

**Phases completed:** 5 phases, 10 plans, 2 tasks

**Key accomplishments:**

- (none recorded)

---

## v1.5 TFT Display Integration (Shipped: 2026-06-06)

**Phases completed:** 1 phases, 5 plans, 8 tasks

**Key accomplishments:**

- Native esp_lcd driver setup for ILI9341 with 8x16 VGA font drawing primitives and a DMA-synchronized boot splash screen.
- Landscape dual-column status dashboard layout and a periodic background FreeRTOS task with state-cached differential rendering to display live pump, timer, temperature, and network status.
- Fix the blocker gap (UAT test 3) where the TFT background update task was created but never executed its first statement: 3072-byte task stack was overflowing under the deep call chain through `tft_display_draw_dashboard_skeleton` -> `tft_fill_rect` with a 640-byte stack-local buffer.
- Close the UAT test 1 major gap: boot splash renders mirrored, with a cyan/light-blue background and a bright white vertical band on the right portion of the screen. Apply the two cheapest source-only fixes (mirror-flag flip + explicit set_gap) and prepare a build-time ST7789 driver toggle as a gated fallback for the case where the actual panel controller is ST7789 mislabelled as ILI9341.
- One-Liner:
- Status:

---

## v1.4 Wi-Fi UI Polish and Code Review (Shipped: 2026-06-05)

**Phases completed:** 2 phases, 2 plans, 6 tasks

**Key accomplishments:**

- Replaced the CPU-heavy backdrop blur panel overlay with a clean Empty State Card, implementing smooth sequential fade transitions and prefers-reduced-motion overrides.
- Verified that custom source code builds cleanly with zero compilation warnings, checked static frontend files for debug logs, corrected the watchdog timeout configuration to align with decision D-07, and confirmed stable hardware regression operation.

---

## v1.3 UI Details Refinement (Shipped: 2026-06-05)

**Phases completed:** 1 phases, 3 plans, 4 tasks
**Requirements:** 9/9 complete

**Known deferred items at close:** 10 scanner bookkeeping rows (see `.planning/STATE.md` Deferred Items)

**Key accomplishments:**

- Replaced absolute test deadline with duration-based countdown logic in cooling control, enabling the Test ON timer to pause during active compressor protection lockout.
- Isolated Auto mode demand state using s_auto_demand to prevent the cooling relay from getting stuck ON during transitions from Test ON to Auto mode.
- Added interactive forms dirty checking, Thai confirmation dialogs, and simplified disconnect buttons to improve UI/UX.

### Archives

- Roadmap: `.planning/milestones/v1.3-ROADMAP.md`
- Requirements: `.planning/milestones/v1.3-REQUIREMENTS.md`
- Audit: `.planning/milestones/v1.3-MILESTONE-AUDIT.md`

---

## v1.2 Owner UI Polish And Hardware Readiness (Shipped: 2026-06-04)

**Phases completed:** 4 phases, 13 plans
**Requirements:** 20/20 complete

**Git stats:** 37 commits, 17 source files changed, +583 / -175 lines (source), 2026-06-02 to 2026-06-03

**Key accomplishments:**

- Baseline UI audit and Thai-first shared state language established before any source edits
- Global accessible keyboard focus, login error announcements, and mobile drawer navigation
- Owner dashboard hierarchy separating pump runtime and cooling operational channels
- Professional light-theme CSS overhaul with HSL color system and accessible contrast
- Hardware/Install active vs pending GPIO separation with DS18B20 4.7 kΩ pull-up guidance
- Wi-Fi scan/connect state flow improvements for loading, error, empty, and connected states
- Full visual regression validation with browser screenshots, accessibility audit, build/footprint gate, and device-backed hardware regression

### Archives

- Roadmap: `.planning/milestones/v1.2-ROADMAP.md`
- Requirements: `.planning/milestones/v1.2-REQUIREMENTS.md`
- Audit: `.planning/milestones/v1.2-MILESTONE-AUDIT.md`

---

## v1.1 Dual Relay Cooling And Install UI (Shipped: 2026-06-02)

**Phases completed:** 5 phases, 15 plans, 62 tasks

**Git stats:** 52 commits, 107 files changed, +13,652 / -381 lines, 2026-05-22 to 2026-06-02

**Known deferred items at close:** 8 scanner bookkeeping rows (see `.planning/STATE.md` Deferred Items) and one documented DS18B20 pull-up hardware follow-up

**Key accomplishments:**

- Typed ESP32 hardware role contract with safe GPIO defaults and DS18B20 powered-mode wiring documentation
- Validated NVS schemas for active/pending GPIO maps, independent relay polarity, timer start phases, and cooling defaults
- Boot and pump API integration for the hardware map foundation while preserving single-relay runtime behavior
- Dual-relay ESP-IDF pump runtime with debounced float selection, start phases, stopped preview, and fail-safe relay writes
- Boot and authenticated pump APIs now initialize, report, persist, and safely apply the dual-relay runtime contract
- Embedded dashboard now configures timer start phases and clearly displays ready/running state for Relay 1 and Relay 2
- ESP-IDF DS18B20 cooling sensor foundation with managed 1-Wire dependency and fail-safe fault thresholds
- Cooling relay state machine with Auto hysteresis, Force OFF, Test ON timeout, and compressor lockout protection
- Boot-started cooling runtime with authenticated `/api/cooling/status` and build-validated DS18B20 relay control
- Authenticated hardware map APIs now expose active/pending GPIO state and safe dropdown options
- Authenticated cooling config and runtime mode APIs now sit on top of the Phase 8 cooling runtime
- Pump config compatibility, Phase 9 documentation, and final build validation are complete
- Owner dashboard cooling operation surface is implemented
- Protected Hardware/Install page and pending GPIO flow are implemented
- Phase 10 source validation, docs, and build closeout are complete

---

## v1.0 MVP — SHIPPED 2026-05-20

**Phases:** 1-5 | **Plans:** 13 | **Tasks:** 44 | **Commits:** 68

### Key Accomplishments

1. **Hardware-safe pump control core** — GPIO32 float input, GPIO26 relay output, timer state machine, and safe relay inactive initialization.
2. **NVS persistence** — Timer settings, relay polarity, and auto-start survive reboot with missing/invalid default handling.
3. **Authenticated pump APIs** — `/api/pump/config`, `/api/pump/status`, `/api/pump/start`, `/api/pump/stop` with session auth and same-origin POST protection.
4. **Local web dashboard** — Thai-language timer setup, Start/Stop, live status polling, countdown, and no CDN dependencies.
5. **Hardware validation** — Float switching, relay phase behavior, Stop safety, reboot persistence, and Wi-Fi regression all passed on real ESP32 hardware.

### Stats

- **Files changed:** 88
- **Lines:** +9,034 / -323
- **Timeline:** 2026-05-18 → 2026-05-20 (3 days)
- **Firmware binary:** `build/fish_pump_relay_timer_control.bin`
- **Partition usage:** 49% of app slot

### Archives

- Roadmap: `.planning/milestones/v1.0-ROADMAP.md`
- Requirements: `.planning/milestones/v1.0-REQUIREMENTS.md`
- Audit: `.planning/v1.0-MILESTONE-AUDIT.md`

### Known Deferred Items

- Automated hardware-in-loop testing (no test rig)
- Automated visual/browser verification (Playwright unavailable)
- Nyquist validation artifacts for all phases

---

*For current project status, see `.planning/PROJECT.md` and `.planning/ROADMAP.md`*
