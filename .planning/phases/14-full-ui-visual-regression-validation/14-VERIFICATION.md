---
status: human_needed
phase: 14-full-ui-visual-regression-validation
updated: 2026-06-03
requirements:
  A11Y-01: partial
  VER-01: partial
  VER-02: passed
  VER-03: passed
  VER-04: partial
  REG-01: human_needed
---

# Phase 14 Verification

## Goal

Verify the finished v1.2 UI and firmware baseline with browser, accessibility, build, UI-review, and hardware-regression evidence before milestone close.

## Requirement Matrix

| Requirement | Status | Evidence | Notes |
|-------------|--------|----------|-------|
| A11Y-01 | partial | `evidence/accessibility-checks.md`, `evidence/browser-checks.md` | Source supports narrow mobile, but reachable device served stale static assets and failed drawer proof. |
| VER-01 | partial | `evidence/browser-checks.md`, `evidence/screenshots/desktop/debug-login-chrome.png` | Browser metrics cover required pages, but full screenshot set is blocked by in-app Browser screenshot timeout. |
| VER-02 | passed | `evidence/footprint.md` | ESP-IDF build passes; no remote dependencies; 45% app-slot headroom. |
| VER-03 | passed | `evidence/ui-review-closeout.md` | `impeccable` and `ui-ux-pro-max` closeout completed with no source-level blocker. |
| VER-04 | partial | `evidence/browser-checks.md` | Representative state hooks are source-inspected; sensor fault and pending reboot screenshots are not-run. |
| REG-01 | human_needed | `evidence/hardware-regression.md` | Hardware-impacting relay/timer/float/cooling/Wi-Fi/APSTA flows require manual device testing on current firmware. |

## Passed Criteria

- Current repository source builds successfully with ESP-IDF.
- Embedded frontend has no remote/CDN dependency hits.
- Firmware binary fits the OTA app partition with 909,376 bytes free.
- UI source review does not show unresolved AI-slop patterns or product-register visual blockers.

## Open Blockers

1. Flash or otherwise confirm the ESP32 is serving the freshly built static assets. The current reachable device served stale `/style.css` and `/app.js`.
2. Re-run 375px mobile drawer validation against the current firmware.
3. Capture the required desktop/mobile screenshots or document a working screenshot fallback.
4. Run manual hardware regression for pump relays, timers, float switch, cooling relay/sensor, Wi-Fi scan/connect/disconnect, and APSTA fallback.

## Verification Status

`human_needed`. Phase 14 should not be marked complete yet because REG-01 and screenshot requirements are not fully satisfied.
