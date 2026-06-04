---
phase: 14-full-ui-visual-regression-validation
plan: 03
subsystem: build-footprint
tags:
  - build
  - footprint
  - esp-idf
key-files:
  created:
    - .planning/phases/14-full-ui-visual-regression-validation/evidence/footprint.md
metrics:
  static_bytes: 166718
  binary_bytes: 1122240
  app_slot_free_bytes: 909376
---

# Plan 14-03 Summary

## Completed

- Confirmed no remote frontend dependencies in `main/static`.
- Recorded embedded static byte inventory and Phase 11 growth comparison.
- Ran ESP-IDF build through `.\scripts\build.ps1` with escalation after sandboxed subprocess failure.
- Recorded binary size and OTA app partition headroom.

## Commits

| Commit | Description |
|--------|-------------|
| pending | Phase 14 evidence commit |

## Deviations

- The first sandboxed build produced a Windows subprocess permission traceback while returning process exit code 0. The escalated build passed.

## Self-Check

PASSED. VER-02 is supported by build-backed evidence.
