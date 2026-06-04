---
phase: 03-authenticated-pump-control-api
plan: 03
subsystem: api
tags: [esp-idf, http-server, route-registration, firmware-build]
requires:
  - phase: 03-01
    provides: pump config handlers
  - phase: 03-02
    provides: pump status, start, and stop handlers
provides:
  - Registered GET/POST /api/pump/config routes
  - Registered GET /api/pump/status route
  - Registered POST /api/pump/start and POST /api/pump/stop routes
  - ESP-IDF build validation evidence
affects: [phase-04-web-ui, phase-05-hardware-validation, pump-api]
tech-stack:
  added: []
  patterns: [route capacity check, static API contract check, ESP-IDF build gate]
key-files:
  created:
    - .planning/phases/03-authenticated-pump-control-api/03-03-SUMMARY.md
  modified:
    - main/web_server.c
key-decisions:
  - "APP_TEMPLATE_HTTP_MAX_URI_HANDLERS remains 18 because the final route table contains exactly 18 handlers."
  - "No static web UI files were changed in Phase 3; Phase 4 owns UI controls."
  - "No hardware flashing was performed; Phase 5 owns hardware validation."
patterns-established:
  - "New authenticated API routes are registered in web_server_start() after existing system/Wi-Fi API routes."
requirements-completed: [RUN-01, RUN-02, API-01, API-02, API-03, API-04, API-05, API-06]
duration: 14 min
completed: 2026-05-20
---

# Phase 03 Plan 03: Pump API Registration and Build Summary

**Registered authenticated pump API routes with static contract checks and ESP-IDF build validation**

## Performance

- **Duration:** 14 min
- **Started:** 2026-05-20T00:40:00Z
- **Completed:** 2026-05-20T00:54:00Z
- **Tasks:** 4 completed
- **Files modified:** 1 tracked source file plus generated build output

## Accomplishments

- Registered all five Phase 3 pump API route entries in `web_server_start()`.
- Confirmed route count is 18, matching `APP_TEMPLATE_HTTP_MAX_URI_HANDLERS`.
- Ran static contract checks for pump route names, auth gates, same-origin gates, required JSON fields, idempotency flags, status fields, and no static UI additions.
- Ran `.\scripts\build.ps1`; ESP-IDF build completed successfully and generated `build/fish_pump_relay_timer_control.bin`.

## Task Commits

1. **Task 1: Register pump API routes** - `bde3ee5` (feat)
2. **Task 2: Run API contract static checks** - no code commit; validation-only task
3. **Task 3: Build validate firmware** - no code commit; generated gitignored `build/` output
4. **Task 4: Summarize Phase 3 requirement coverage** - captured in this summary

## Files Created/Modified

- `main/web_server.c` - Registered pump API handlers.
- `build/fish_pump_relay_timer_control.bin` - Generated firmware binary from successful ESP-IDF build.
- `.planning/phases/03-authenticated-pump-control-api/03-03-SUMMARY.md` - Final route/build validation evidence.

## Decisions Made

- Kept `APP_TEMPLATE_HTTP_MAX_URI_HANDLERS` unchanged at 18 because the final route count does not exceed capacity.
- Left `main/static/` untouched so Phase 3 remains API-only.
- Did not flash or monitor hardware as this phase's validation gate is build-only.

## Deviations from Plan

None - plan executed exactly as written.

**Total deviations:** 0 auto-fixed.
**Impact:** No scope creep.

## Issues Encountered

The first build invocation exceeded the tool timeout while the ESP-IDF process continued in the background. After waiting for the process to finish, rerunning `.\scripts\build.ps1` completed successfully and confirmed the binary.

## Verification

- `rg "/api/pump/config|/api/pump/status|/api/pump/start|/api/pump/stop" main/web_server.c` passed.
- `rg "require_auth\\(req\\)" main/web_server.c` passed.
- `rg "is_same_origin\\(req\\)" main/web_server.c` passed.
- `rg "timer1_on_sec|timer1_off_sec|timer2_on_sec|timer2_off_sec|auto_start|relay_polarity" main/web_server.c` passed.
- `rg "float_state|active_timer|phase|countdown_sec|relay_energized|settings_status" main/web_server.c` passed.
- `rg "already_running|already_stopped" main/web_server.c` passed.
- `rg "/api/pump|timer1_on_sec|relay_polarity" main/static` returned no matches.
- `/api/status` inspection showed only existing system, Wi-Fi, memory, uptime, and DNS fields.
- `.\scripts\build.ps1` passed with ESP-IDF 6.1.
- `Test-Path build\fish_pump_relay_timer_control.bin` returned `True`.

## Requirement Coverage

- **RUN-01:** `POST /api/pump/start` starts pump control idempotently.
- **RUN-02:** `POST /api/pump/stop` stops pump control idempotently and verifies inactive relay state.
- **API-01:** `GET /api/pump/config` returns full editable config plus read-only hardware defaults.
- **API-02:** `POST /api/pump/config` validates and saves full replacement config before runtime apply.
- **API-03:** Start route returns `already_running` and status snapshot.
- **API-04:** Stop route returns `already_stopped` and status snapshot.
- **API-05:** `GET /api/pump/status` exposes machine-friendly runtime status.
- **API-06:** All pump routes require auth, and POST routes require same-origin checks.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Phase 4 can build the authenticated web UI against stable `/api/pump/*` JSON contracts. Phase 5 still owns hardware flash/manual validation.

## Self-Check: PASSED

All route registration, static contract, and ESP-IDF build validation checks passed.

---
*Phase: 03-authenticated-pump-control-api*
*Completed: 2026-05-20*
