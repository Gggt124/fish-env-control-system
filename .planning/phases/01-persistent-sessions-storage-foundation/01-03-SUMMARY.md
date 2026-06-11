---
phase: 01-persistent-sessions-storage-foundation
plan: 03
subsystem: session
tags:
  - ui
  - cookie
  - remember-me
dependency_graph:
  requires:
    - 01-02
  provides:
    - Persistent sessions across browser restarts
  affects:
    - main/static/index.html
    - main/static/app.js
    - main/web_server.c
tech_stack:
  added: []
  patterns:
    - Session Cookies
key_files:
  created: []
  modified:
    - main/static/index.html
    - main/static/app.js
    - main/web_server.c
key_decisions:
  - Stored Remember Me state dynamically and set cookie Max-Age on backend conditionally.
metrics:
  duration_minutes: 15
  tasks_completed: 2
  lines_of_code_changed: 35
  completed_at: 2026-06-11T14:52:00+07:00
---

# Phase 01 Plan 03: Persistent Sessions Storage Foundation Summary

**Integrate the stateless JWT session backend with the web server and add UI for persistent "Remember Me" sessions.**

## Implementation Details

- **Task 1 ("Remember Me" UI and Client Logic):** Added a "จำการเข้าสู่ระบบ (Remember Me)" checkbox to the login form in `main/static/index.html` and extracted its value to include in the `/api/login` POST payload in `main/static/app.js`.
- **Task 2 (Web Server Cookie Management and Session Integration):** Updated `handle_api_login` in `main/web_server.c` to parse the `remember` flag from JSON or form data. If true, appended `Max-Age=31536000` to the `Set-Cookie` header to persist the session. Removed the `session_destroy` call in `handle_api_logout` since JWT sessions are stateless.

## Deviations from Plan

**1. [Rule 3 - Broken File Reference] Auto-fixed HTML filename**
- **Found during:** Task 1
- **Issue:** The plan referenced `main/static/login.html`, but the repository uses a single `index.html` containing all views.
- **Fix:** Added the Remember Me checkbox to the login view within `main/static/index.html`.
- **Files modified:** `main/static/index.html`
- **Commit:** pending

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED
