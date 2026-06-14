---
phase: 03-hardware-recovery-anti-lockout
plan: 05
subsystem: "Web Server & Frontend"
tags:
  - ux
  - anti-lockout
  - staging
dependencies:
  requires: ["03-01", "03-02", "03-03", "03-04"]
  provides: ["Seamless Staging Confirmation"]
  affects: ["main/static/app.js", "main/web_server.c"]
tech_stack:
  added: []
  patterns: ["Silent Validation", "Graceful Degradation"]
key_files:
  created: []
  modified:
    - main/static/app.js
    - main/web_server.c
decisions:
  - "Replaced blocking alert() calls with non-blocking showToast() for confirmation/rollback flows."
  - "Removed unconditional session invalidation upon staging confirmation to prevent double redirects."
  - "Skipped staging for Wi-Fi profiles that match existing credentials in NVS."
metrics:
  duration_minutes: 5
  completed_date: "2026-06-14"
---

# Phase 03 Plan 05: Close UAT gaps for Staging Confirmation

Close UAT gaps for Staging Confirmation UI Banner by fixing UX issues in the confirmation flow and avoiding redundant staging for known Wi-Fi profiles.

## Summary

The confirmation and rollback workflows were refined to improve user experience. First, blocking \lert()\ popups in the frontend were substituted with non-intrusive \showToast()\ notifications combined with an explicit reload timer. Second, the backend logic unconditionally terminating sessions after applying configuration settings was modified so authenticated administrators wouldn't be force-logged out of the interface just for verifying changes. Lastly, saving an identical Wi-Fi profile now correctly bypasses the staging/reboot flow entirely, returning immediate success when the credentials already exist exactly as provided.

## Key Changes

1. **Replaced blocking alerts:** The \confirmStaging\ and \cancelStaging\ functions in \pp.js\ now use \showToast\ and a 1.5s delay before reloading.
2. **Session Persistence:** \handle_api_confirm_post()\ in \web_server.c\ no longer blindly calls \session_invalidate_all()\, preserving the user's active session.
3. **Smart Wi-Fi Staging:** \handle_api_wifi_profiles_save()\ now inspects existing NVS Wi-Fi profiles. If the new profile matches an already known SSID/password exactly, staging is bypassed and no reboot is triggered.

## Deviations from Plan

None - plan executed exactly as written.

## Self-Check: PASSED
FOUND: main/static/app.js
FOUND: main/web_server.c
