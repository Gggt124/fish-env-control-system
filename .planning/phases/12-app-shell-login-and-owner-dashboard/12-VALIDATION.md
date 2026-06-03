---
phase: 12
slug: app-shell-login-and-owner-dashboard
status: approved
nyquist_compliant: true
wave_0_complete: true
created: 2026-06-03
---

# Phase 12 - Validation Strategy

> Per-phase validation contract for the App Shell Login and Owner Dashboard phase.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | Source `rg` checks, ESP-IDF build, and manual browser testing |
| **Config file** | `main/static/`, `main/CMakeLists.txt` |
| **Quick run command** | `rg -n ":focus-visible" main/static/style.css` |
| **Full suite command** | `idf.py build` |
| **Estimated runtime** | <10 seconds for source checks, ~1 minute for build |

---

## Sampling Rate

- **After every task commit:** Run the task's planned artifact and source `rg` verification command.
- **After every plan wave:** Verify screenshot/report or brief completeness for the completed deliverables.
- **Before `$gsd-verify-work`:** Run the full source check suite and manually review screenshot evidence.

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 12-01-01 | 01 | 1 | UI-08, A11Y-02 | T-12-01 | Focus visibility is maintained | source check | `rg -n ":focus-visible" main/static/style.css` | N/A command | passed |
| 12-01-02 | 01 | 1 | UI-08, A11Y-02 | | | build | `idf.py build` | N/A command | passed |
| 12-02-01 | 02 | 2 | UI-09, UI-14 | | | source check | `rg -n "translateX" main/static/style.css` | N/A command | passed |
| 12-02-02 | 02 | 2 | UI-09, UI-14 | | | source check | `rg -n "toggleMobileNav" main/static/app.js` | N/A command | passed |
| 12-02-03 | 02 | 2 | UI-09, UI-14 | | | build | `idf.py build` | N/A command | passed |
| 12-03-01 | 03 | 3 | UI-10, UI-15 | | | source check | `rg -n "border-left" main/static/style.css` | N/A command | passed |
| 12-03-02 | 03 | 3 | UI-10, UI-15 | | | build | `idf.py build` | N/A command | passed |

---

## Wave 0 Requirements

Existing command-based infrastructure covers Phase 12.

No package-managed test framework is required because:

- Browser screenshots and state capture are evidence artifacts, not automated app tests.
- Firmware build validation is already centralized.

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Test Accessibility (Focus Rings & ARIA) | A11Y-02 | Visual focus requires browser rendering | Run Test 1 from `12-UAT.md` |
| Test App Shell / Mobile Drawer | UI-14, UI-15 | Drawer requires browser rendering and resize | Run Test 2 from `12-UAT.md` |
| Test Dashboard Visual Hierarchy | UI-08, UI-09, UI-10 | Visual hierarchy requires human verification | Run Test 3 from `12-UAT.md` |

---

## Validation Sign-Off

- [x] All anticipated tasks have automated artifact checks or documented manual evidence.
- [x] Sampling continuity: no three consecutive tasks without a quick verification command.
- [x] Existing infrastructure covers Wave 0; no new files are required.
- [x] No watch-mode flags.
- [x] Source-check feedback latency is <10 seconds.
- [x] `nyquist_compliant: true` set in frontmatter.

**Approval:** approved for phase 12
