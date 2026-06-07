---
phase: 18
slug: tft-display-integration
status: draft
nyquist_compliant: true
wave_0_complete: true
created: 2026-06-06
---

# Phase 18 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | ESP-IDF Build System (compiler validation) |
| **Config file** | `sdkconfig` |
| **Quick run command** | `idf.py build` |
| **Full suite command** | `idf.py build` |
| **Estimated runtime** | ~30 seconds (incremental) |

---

## Sampling Rate

- **After every task commit:** Run `idf.py build`
- **After every plan wave:** Run `idf.py build`
- **Before `/gsd-verify-work`:** Full build must pass successfully
- **Max feedback latency:** 30 seconds

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 18-01-01 | 01 | 1 | TFT-01 | — | N/A | build | `idf.py build` | N/A | ⬜ pending |
| 18-01-02 | 01 | 1 | TFT-02 | — | N/A | build | `idf.py build` | N/A | ⬜ pending |
| 18-01-03 | 01 | 2 | TFT-03 | — | N/A | build | `idf.py build` | N/A | ⬜ pending |
| 18-01-04 | 01 | 2 | TFT-04 | — | N/A | build | `idf.py build` | N/A | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

Existing build infrastructure covers all compilation checks. No additional test framework installation is required.

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Display Initialization | TFT-01 | Hardware SPI signal / display control | Flash firmware, boot device, and confirm serial logs show successful display initialization. |
| Shapes and Text Draw | TFT-02 | Graphic rendering correctness on physical screen | Flash test pattern (e.g. test shapes and characters) and visually verify lines, rectangles, and letters render without distortion. |
| Landscape Dashboard UI | TFT-03 | Visual layout alignment and spacing | Verify the dual-column landscape dashboard renders status items (PUMP, TEMP, TIMER, WiFi) aligned with correct HSL color codes (Green/Gray). |
| Real-time Countdown | TFT-04 | Timer ticking behavior and non-blocking task | Verify that the countdown ticks down every 1 second, and Wi-Fi/Web server remains responsive (no watchdog resets). |

---

## Validation Sign-Off

- [x] All tasks have `<automated>` build verify
- [x] Sampling continuity: no 3 consecutive tasks without automated build check
- [x] No watch-mode flags
- [x] Feedback latency < 30s
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
