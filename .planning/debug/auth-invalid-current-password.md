# Debug Session: auth-invalid-current-password

## Status: resolved

## Focus
Investigate issue: auth-invalid-current-password

**Summary:** User reported: ลองใส่รหัสผ่านผิด แล้วกดเปลี่ยน มันเป็นแบบนี้ loading state ค้างแล้วก็เด้งมาหน้า login อีก

## Symptoms
expected: In the Change Password modal, entering an incorrect current password shows an error message and does not change the credentials.
actual: User reported: ลองใส่รหัสผ่านผิด แล้วกดเปลี่ยน มันเป็นแบบนี้ loading state ค้างแล้วก็เด้งมาหน้า login อีก
errors: None reported
reproduction: Test 4 in UAT
timeline: Discovered during UAT

## Eliminated
- Frontend validation bug (the frontend sends the request properly).
- Modal error handling (the modal code has logic to handle `invalid_credentials` error).

## Evidence
- `main/web_server.c` in `handle_api_auth_credentials` returns a `401 Unauthorized` HTTP status when the `current_password` provided by the user doesn't match `stored_pass`.
- `main/static/app.js` has a global interceptor in `apiPost()` and `apiGet()` that checks `if (xhr.status === 401 && window.location.pathname !== '/login')`.
- If the status is 401, it immediately calls `handleUnauthorized()`.
- `handleUnauthorized()` clears the session cookie and navigates the user to `/login`.
- Because the callback is never called, the loading state in the modal is never removed.

## Resolution
Root cause found: Returning 401 on an invalid current password during the "Change Password" flow triggers the global 401 auth interceptor in the frontend, leading to an immediate redirect to login.

Suggested fix: In `main/web_server.c:handle_api_auth_credentials`, change the response status for invalid current password from `"401 Unauthorized"` to `"400 Bad Request"`. This allows the frontend to receive the `{"ok":false,"error":"invalid_credentials"}` payload and handle it gracefully within the modal.
