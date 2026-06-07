# Phase 18: TFT Display Integration - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-06-06
**Phase:** 18-tft-display-integration
**Areas discussed:** Layout & Screen Refresh Rate

---

## Layout & Screen Refresh Rate

| Option | Description | Selected |
|--------|-------------|----------|
| Hybrid | Tick countdown every 1s, and update other states (relay, float, temp) immediately on change | ✓ |
| Fixed Interval (500ms) | Periodically update the entire screen every 500ms (high responsiveness but more CPU/SPI usage) | |
| Fixed Interval (1s) | Periodically update the entire screen every 1s (lower CPU/SPI, simple code) | |

**User's choice:** Hybrid: Tick countdown every 1s, and update other states (relay, float, temp) immediately on change.
**Notes:** The user wants smooth countdown ticking while maintaining immediate visual feedback when binary states (like relay and float switch) change.

---

## the agent's Discretion

- Backlight Power Management: The backlight will default to Always-ON (via GPIO4 or direct 3.3V connection).
- Display UI Language: The UI will default to English labels/numbers to save RAM and avoid rendering complex Thai fonts on the raw canvas.

## Deferred Ideas

- Touch screen interaction (reserved Touch CS GPIO13)
- SD card data logging (reserved SD CS GPIO12)
