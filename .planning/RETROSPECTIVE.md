# Project Retrospective

*A living document updated after each milestone. Lessons feed forward into future planning.*

## Milestone: v1.2 — Owner UI Polish And Hardware Readiness

**Shipped:** 2026-06-04
**Phases:** 4 | **Plans:** 13

### What Was Built
- Baseline UI audit with evidence-backed findings and Thai-first shared state language before any source edits.
- Global accessible keyboard focus primitives, login error announcements, and mobile slide-out drawer navigation.
- Owner dashboard hierarchy separating pump runtime/timer/relay from cooling operational channel.
- Professional light-theme CSS system with HSL color palette, accessible contrast, and cleaned-up AI-generated artifacts.
- Hardware/Install active vs pending GPIO map visual separation with DS18B20 4.7 kΩ pull-up guidance.
- Wi-Fi scan/connect state flow improvements covering loading, error, empty, disconnected, and connected states.
- Full visual regression validation: browser screenshots, accessibility audit, impeccable/ui-ux-pro-max review, build/footprint gate, and device-backed hardware regression.

### What Worked
- The no-edit Phase 11 baseline audit made all subsequent UI changes traceable and gave clear acceptance criteria.
- Using `impeccable` and `ui-ux-pro-max` as design gates forced professional-quality outcomes without subjective debate.
- CSS-only overhaul within offline constraints proved sufficient — no framework or CDN was needed.
- Device-backed hardware regression in Phase 14 confirmed no runtime impact from UI changes.

### What Was Inefficient
- Phase 14 validation required 4 separate plans (browser evidence, accessibility, build, hardware) where some could have been combined.
- Three quick tasks during the milestone (audit fixes, health repair, dark theme) added scope not originally in the v1.2 requirements.

### Patterns Established
- Audit before implement: baseline evidence pass → shared state language → implementation briefs → source edits.
- CSS overhaul stays offline-safe: HSL tokens, system font stack, no CDN or remote assets.
- Active vs pending hardware maps: visual separation reinforced as a UI pattern, not just a firmware contract.

### Key Lessons
1. A no-edit baseline audit phase pays for itself by establishing clear before/after evidence and scoped implementation briefs.
2. Professional CSS aesthetics are achievable within offline embedded constraints using HSL color systems and careful typography.
3. Mobile drawer navigation is essential for ESP32 operator dashboards where users connect from phones.

### Cost Observations
- Model mix: not recorded.
- Sessions: not recorded.
- Notable: the 2-day milestone was efficient due to the Phase 11 audit establishing clear scope boundaries.

---

## Milestone: v1.1 — Dual Relay Cooling And Install UI

**Shipped:** 2026-06-02
**Phases:** 5 | **Plans:** 15 | **Tasks:** 62

### What Was Built
- Added a typed ESP32 hardware contract with active/pending GPIO maps and safe defaults.
- Replaced the single pump relay model with two float-selected timer/relay channels.
- Added DS18B20 cooling control with safe-off faults, hysteresis, lockout, and bounded Test ON.
- Added authenticated hardware/cooling APIs, owner dashboard controls, and Hardware/Install UI.
- Stabilized APSTA recovery, asynchronous scans, stale HTTP client cleanup, and serial soak diagnostics.

### What Worked
- Scoped phases kept hardware-map, pump, cooling, API, and UI behavior independently verifiable.
- Real-board UAT caught boot-map promotion, cooling lockout, and DS18B20 rediscovery issues before archive.
- The final `13:38:10` soak provided enough evidence to reject scheduled reboot as a default workaround.

### What Was Inefficient
- Long-uptime diagnostics were added after web refresh failures appeared, requiring extra flash-and-soak cycles.
- Nyquist `VALIDATION.md` documents were not created during Phases 7-10 and remain documentation debt.
- The GSD open-artifact scanner does not recognize retained quick-task directory metadata consistently.

### Patterns Established
- Relay GPIO edits stay pending until reboot confirmation.
- HTTP response send failures propagate so ESP-IDF closes stale client sessions.
- Wi-Fi scans are owned by `wifi_manager` with overlap rejection and late-callback cleanup.
- Board diagnostics remain bounded serial snapshots rather than scheduled reboots.

### Key Lessons
1. Instrument socket counts, scan state, heap minima, stack margins, reset reason, and watchdog state before extended field tests.
2. Treat DS18B20 `85.0 C` as a transient power-on value and install the documented external `4.7 kOhm` pull-up.
3. Prefer evidence-driven recovery fixes over periodic reboot when heap, sockets, watchdog, and radio state can be measured.

### Cost Observations
- Model mix: not recorded.
- Sessions: not recorded.
- Notable: the soak logger and bounded serial snapshots reduced guesswork during the final stability pass.

---

## Cross-Milestone Trends

### Process Evolution

| Milestone | Sessions | Phases | Key Change |
|-----------|----------|--------|------------|
| v1.1 | not recorded | 5 | Added real-board UAT plus a 13-hour whole-device soak before archive |
| v1.2 | not recorded | 4 | Added no-edit baseline audit phase before UI edits; used impeccable/ui-ux-pro-max as design gates |

### Cumulative Quality

| Milestone | Tests | Coverage | Zero-Dep Additions |
|-----------|-------|----------|-------------------|
| v1.1 | Build, syntax check, 10 UAT cases, 13-hour soak | Hardware and long-uptime evidence | Offline embedded UI retained |
| v1.2 | Build, footprint gate, browser screenshots, accessibility audit, device-backed hardware regression | 20/20 requirements, 4/4 Nyquist compliant | Professional CSS overhaul, mobile drawer, accessible focus — still zero external deps |

### Top Lessons (Verified Across Milestones)

1. Keep relay boot behavior fail-safe and validate on the physical board.
2. Keep the local UI independent from external runtime assets.
3. Audit-before-implement eliminates rework — establish baseline evidence and acceptance criteria before source edits.

