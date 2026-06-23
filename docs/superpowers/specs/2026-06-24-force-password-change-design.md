# Force Password Change on First Login (ESP32)

## Purpose
The system currently uses a hardcoded default password (`admin123`). This leaves the device vulnerable to unauthorized access if the user never changes the password. This design forces the user to change the default password upon their first successful login.

## Backend Architecture (ESP-IDF)

### State Management
- `nvs_store_get_credentials()` returns the stored password.
- If the current password matches `APP_TEMPLATE_DEFAULT_PASSWORD` ("admin123"), the system is in the `require_password_change` state.

### `POST /api/login`
- When a user logs in successfully, the backend checks if the system is in the `require_password_change` state.
- If true, the response will be: `{"ok": true, "require_password_change": true, "username": "..."}`.
- A valid session token is still issued so the user can call the change password API.

### Security Middleware (Auth Check)
- For all protected API routes (e.g., `/api/status`, `/api/wifi/scan`), the server checks the session.
- We will modify the auth middleware in `web_server.c` to also check if the system requires a password change.
- If `require_password_change` is true, any API *except* `/api/login`, `/api/logout`, and `/api/change-password` will return HTTP 403 Forbidden with JSON payload: `{"ok": false, "error": "force_password_change"}`.

### `POST /api/change-password` (New Endpoint)
- **Requires Auth**: Yes
- **Request Body**: `{"current_password": "...", "new_password": "..."}`
- **Validation**:
  - `current_password` must match the stored password.
  - `new_password` must be at least 8 characters, valid UTF-8, and not equal to `APP_TEMPLATE_DEFAULT_PASSWORD`.
- **Action**: Updates the credentials in NVS.
- **Response**: `{"ok": true}`.

## Frontend Architecture

### Central API Handler (`app.js`)
- Modify `apiGet()` and `apiPost()` error handlers.
- If the server returns a 403 status with `error === "force_password_change"`, the handler will trigger a global UI function `showForcePasswordModal()`.

### Login Flow (SPA via `app.js`)
- Upon successful login, check the response payload for `require_password_change`.
- If true, navigate to the Dashboard view (`navigateTo('/dashboard')`). When the dashboard logic attempts to fetch `/api/status`, it will receive the 403 `force_password_change` error, triggering the modal via the central API handler. The user remains on the same physical page (`index.html`).

### UI Component: Force Password Change Modal
- A new HTML modal added directly to `index.html`.
- **Behavior**:
  - Un-dismissible (no "X" button, clicking the backdrop doesn't close it).
  - Contains fields: "รหัสผ่านปัจจุบัน" (Current Password), "รหัสผ่านใหม่" (New Password), "ยืนยันรหัสผ่านใหม่" (Confirm New Password).
  - Buttons: "เปลี่ยนรหัสผ่าน" (Submit) and "ออกจากระบบ" (Logout).
- **Styling**: Uses the existing plain CSS design system, matching the current modern/glassmorphism aesthetic.

## Edge Cases & Error Handling
- **Short Password**: Handled by the backend API (returns 400) and frontend validation.
- **Bypassing UI**: If an attacker intercepts the session and tries to call `/api/wifi/connect` directly via curl, the backend middleware will block it with 403 because the password is still the default.
