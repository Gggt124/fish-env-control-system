---
status: diagnosed
phase: 02-credential-management-ui-logic
source: [02-01-SUMMARY.md, 02-02-SUMMARY.md, 02-03-SUMMARY.md, 02-04-SUMMARY.md]
started: 2026-06-12T17:20:02+07:00
updated: 2026-06-12T18:38:20+07:00
---

## Current Test

[testing complete]

## Tests

### 1. Cold Start Smoke Test
expected: Kill any running server/service. Clear ephemeral state (temp DBs, caches, lock files). Start the application from scratch. Server boots without errors, any seed/migration completes, and a primary query (health check, homepage load, or basic API call) returns live data.
result: pass

### 2. Change Password Modal and Flow
expected: On the dashboard, clicking "Change Password" opens a modal. Submitting the correct current password and a new username/password succeeds, logs the user out immediately, and redirects to the login page.
result: pass

### 3. Login with New Credentials
expected: After changing the password, logging in with the old credentials fails. Logging in with the new credentials succeeds and loads the dashboard.
result: pass

### 4. Invalid Current Password
expected: In the Change Password modal, entering an incorrect current password shows an error message and does not change the credentials.
result: issue
reported: "ลองใส่รหัสผ่านผิด แล้วกดเปลี่ยน มันเป็นแบบนี้ loading state ค้างแล้วก็เด้งมาหน้า login อีก"
severity: major

## Summary

total: 4
passed: 3
issues: 1
pending: 0
skipped: 0

## Gaps

- truth: "In the Change Password modal, entering an incorrect current password shows an error message and does not change the credentials."
  status: failed
  reason: "User reported: ลองใส่รหัสผ่านผิด แล้วกดเปลี่ยน มันเป็นแบบนี้ loading state ค้างแล้วก็เด้งมาหน้า login อีก"
  severity: major
  test: 4
  root_cause: "When a user enters an incorrect current password, backend returns 401 Unauthorized, which triggers frontend's global interceptor to clear session and redirect to /login."
  artifacts:
    - path: "main/web_server.c"
      issue: "Returns 401 Unauthorized for invalid_credentials"
    - path: "main/static/app.js"
      issue: "Global 401 interceptor catches this and aborts modal flow"
  missing:
    - "Change HTTP status code from 401 to 400 in backend when current password does not match"
  debug_session: ".planning/debug/auth-invalid-current-password.md"

