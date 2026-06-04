---
phase: 11-baseline-ui-audit-and-state-language
plan: 02
subsystem: ui-contract
tags: [state-language, accessibility, responsive, offline, esp32]

requires:
  - phase: 11-baseline-ui-audit-and-state-language
    provides: Source-backed audit findings and explicit evidence limitations from plan 01.
provides:
  - Canonical loading, success, error, empty, disabled, unavailable, pending reboot, and disconnected vocabulary.
  - ESP32-adapted product UI checklist with keep, adapt, and exclude decisions.
  - Accessibility, responsive, offline, and footprint implementation gates.
affects: [phase-12, phase-13, phase-14, embedded-ui, tft-future]

tech-stack:
  added: []
  patterns: [thai-first state copy, contextual-status-before-toast, reasoned-disabled-state]

key-files:
  created:
    - .planning/phases/11-baseline-ui-audit-and-state-language/11-STATE-LANGUAGE.md
    - .planning/phases/11-baseline-ui-audit-and-state-language/11-ESP32-UI-CHECKLIST.md
  modified: []

key-decisions:
  - "Disabled controls must resolve to processing, reasoned unavailable, or pending reboot rather than an unexplained dimmed state."
  - "Safety, recovery, and pending-reboot messages stay contextual; toast is only an additional short confirmation."
  - "Frontend asset and firmware sizes are audit references, not hard budgets."

patterns-established:
  - "Canonical state rows specify copy, visual treatment, action, announcement behavior, and downstream ownership."
  - "ESP32 adaptation keeps local HTML/CSS/JavaScript and excludes network-dependent or footprint-heavy UI advice."

requirements-completed: [UI-13, A11Y-02, A11Y-03]

duration: 3 min
completed: 2026-06-02
---

# Phase 11 Plan 02: Shared UI State Language Summary

**Thai-first embedded UI state contract with accessibility semantics, responsive checks, and ESP32-specific keep/adapt/exclude guidance**

## Performance

- **Duration:** 3 min
- **Started:** 2026-06-02T11:32:20Z
- **Completed:** 2026-06-02T11:35:23Z
- **Tasks:** 1
- **Files modified:** 2

## Accomplishments

- Defined all eight canonical UI states and the three locked availability
  conditions: `processing`, reasoned `unavailable`, and `pending reboot`.
- Added Thai-first copy patterns, visual treatment, allowed action, assistive
  announcement, and ownership guidance for every state.
- Adapted product-dashboard guidance to the ESP32 footprint and offline
  boundary with explicit keep, adapt, and exclude decisions.

## Task Commits

Each task was committed atomically:

1. **Task 1: Define canonical states and the ESP32-adapted checklist** - `08b5964` (docs)

## Files Created/Modified

- `11-STATE-LANGUAGE.md` - Reusable state vocabulary and presentation rules.
- `11-ESP32-UI-CHECKLIST.md` - Implementation and Phase 14 validation checklist.

## Decisions Made

- Safety and recovery text stays contextual instead of relying on transient
  toast messages.
- Mobile controls target at least `44x44px`; every interactive control receives
  a visible `2-4px` blue `:focus-visible` ring.
- `159,436` static bytes and `1,111,616` firmware bytes are comparison
  references, not arbitrary hard caps.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Phase 12 and Phase 13 can implement one shared state vocabulary.
- The checklist preserves accessibility, responsive, offline, and footprint
  requirements without importing unsuitable dependencies.
- Ready for `11-03-PLAN.md`.

## Self-Check: PASSED

- All eight canonical state names present.
- Three availability conditions and at least three unavailable reasons present.
- Keep, adapt, and exclude decisions present.
- Accessibility, responsive, offline, and footprint gates present.
- `git diff --name-only -- main/static` returned no files.

---
*Phase: 11-baseline-ui-audit-and-state-language*
*Completed: 2026-06-02*
