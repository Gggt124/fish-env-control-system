---
phase: 03-authenticated-pump-control-api
phase_number: "03"
phase_name: authenticated-pump-control-api
status: clean
depth: standard
files_reviewed: 1
scope_source: SUMMARY.md
reviewed_files:
  - main/web_server.c
findings:
  critical: 0
  warning: 0
  info: 0
  total: 0
updated: 2026-05-20
supersedes_review_commit: b7688a1
fix_commit: 76abdaf
---

# Phase 03 Code Review: Authenticated Pump Control API

## Summary

Reviewed Phase 3 source scope from the phase summary artifacts:

- `main/web_server.c`

No issues found at standard review depth. The prior warning findings from the first review are resolved in fix commit `76abdaf`.

## Resolved Prior Findings

### WR-001: Timer JSON validation casts unbounded doubles to `uint32_t`

**Status:** Resolved

`api_pump_required_u32()` now rejects negative, out-of-range, and non-integral JSON numbers before assigning the validated value to `uint32_t`. The cast occurs only after the configured timer range has been checked, so unbounded values such as `1e100` are rejected without relying on implementation-defined or undefined conversion behavior.

### WR-002: Same-origin check uses substring matching for safety-critical POST routes

**Status:** Resolved

`is_same_origin()` now parses the request origin host and compares it exactly against the AP IP, STA IP, or mDNS host. The pump mutation routes also call it with `allow_missing_header=false`, so `/api/pump/config`, `/api/pump/start`, and `/api/pump/stop` reject POSTs that omit both `Origin` and `Referer`.

## Positive Checks

- All five Phase 3 routes are registered in `web_server_start()`: `GET/POST /api/pump/config`, `GET /api/pump/status`, `POST /api/pump/start`, and `POST /api/pump/stop`.
- Pump API handlers consistently require authentication before serving protected data or mutating runtime state.
- Pump config, start, and stop POST handlers use strict origin checking.
- Timer duration validation checks the configured min/max bounds before casting to `uint32_t`.
- Start is idempotent and does not reset the running timer state when already running.
- Stop verifies the relay is inactive before returning success.
- Config save persists settings before runtime apply, then restarts only when the controller was running before the save.

## Verification Notes

- Review depth: standard.
- Scope source: Phase 3 `*-SUMMARY.md` artifacts.
- Static review checks covered route registration, auth gates, strict pump POST origin gates, and timer JSON validation.
- The fix commit was build-validated with `.\scripts\build.ps1`; no additional build was run during this review rerun.
- Hardware flashing/manual validation remains Phase 5 scope.
