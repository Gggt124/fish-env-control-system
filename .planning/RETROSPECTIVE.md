# Project Retrospective

*A living document updated after each milestone. Lessons feed forward into future planning.*

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

### Cumulative Quality

| Milestone | Tests | Coverage | Zero-Dep Additions |
|-----------|-------|----------|-------------------|
| v1.1 | Build, syntax check, 10 UAT cases, 13-hour soak | Hardware and long-uptime evidence | Offline embedded UI retained |

### Top Lessons (Verified Across Milestones)

1. Keep relay boot behavior fail-safe and validate on the physical board.
2. Keep the local UI independent from external runtime assets.
