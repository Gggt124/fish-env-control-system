---
phase: 14-full-ui-visual-regression-validation
status: context-ready
created: 2026-06-03
---

# Phase 14 Context

## Goal

Close v1.2 with repeated full-UI quality review plus screenshot, accessibility, build, footprint, and stable-baseline regression evidence.

## Inputs

- `.planning/ROADMAP.md` Phase 14 success criteria.
- `.planning/phases/11-baseline-ui-audit-and-state-language/11-ESP32-UI-CHECKLIST.md`
- `.planning/phases/13-install-setup-status-ux-and-professional-ui-overhaul/13-UI-REVIEW.md`
- `.planning/quick/260603-88n-pre-phase-14-audit-fixes-for-blockers-an/260603-88n-SUMMARY.md`

## Required Validation Surfaces

- Login
- Dashboard
- Hardware/Install
- Wi-Fi
- Status
- App shell navigation and logout path

## Evidence Required

- Desktop and narrow-mobile screenshot/browser evidence for affected pages.
- Accessibility checks for labels, focus visibility, touch target size, and dynamic state announcements.
- `.\scripts\build.ps1` or equivalent ESP-IDF build gate.
- Embedded asset/firmware footprint check against partition capacity.
- Manual hardware regression notes for relay, timer, cooling, Wi-Fi, and APSTA flows.

## Constraints

- Keep the frontend offline-capable: no CDN, no web fonts, no frontend package dependency.
- Do not claim hardware behavior completion without real ESP32 flash/device evidence.
