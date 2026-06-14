---
phase: 02-credential-management-ui-logic
plan: 05
subsystem: web_server
tags:
  - fix
  - api
  - auth
dependency_graph:
  requires:
    - Phase 02 Plan 04
  provides:
    - Change Password Error Handling Fix
  affects:
    - main/web_server.c
tech_stack:
  added: []
  patterns:
    - HTTP 400 for validation errors to bypass 401 global logout
key_files:
  created: []
  modified:
    - main/web_server.c
key_decisions:
  - Use 400 Bad Request for invalid current password during credential change to avoid triggering the global 401 logout interceptor.
metrics:
  duration_minutes: 2
  tasks_completed: 1
  files_modified: 1
---

# Phase 02 Plan 05: Change Password Error Handling Fix Summary

Changed the response code for incorrect current password from `401 Unauthorized` to `400 Bad Request` to prevent the frontend's global interceptor from improperly logging the user out.

## Execution Notes

The implementation for this plan was found to be already present in the codebase via commit `a30c800`. The code in `main/web_server.c` for `handle_api_auth_credentials()` already correctly returns `400 Bad Request` when the current password does not match. Therefore, no new source code changes were necessary.

## Deviations from Plan

None - plan was already fulfilled.

## Self-Check: PASSED
- `main/web_server.c` correctly handles incorrect credentials with HTTP 400.
