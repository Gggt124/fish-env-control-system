---
status: complete
phase: 03-hardware-recovery-anti-lockout
source: [03-01-SUMMARY.md, 03-02-SUMMARY.md, 03-03-SUMMARY.md, 03-04-SUMMARY.md, 03-05-SUMMARY.md]
started: 2026-06-13T22:21:00Z
updated: 2026-06-14T16:10:00Z
---

## Current Test

[testing complete]

## Tests

### 1. Cold Start Smoke Test
expected: |
  Kill any running server/service. Clear ephemeral state (temp DBs, caches, lock files). Start the application from scratch. Server boots without errors, any seed/migration completes, and a primary query (health check, homepage load, or basic API call) returns live data.
result: pass

### 2. Verify Staging Confirmation UI Banner
expected: |
  - Setup: Change the admin credentials (password).
  - Action: Open the dashboard, status, or Wi-Fi configuration page in your browser.
  - Expectation:
    - A yellow warning banner is displayed at the top: **"โหมดทดลองใช้การตั้งค่าใหม่: ระบบจะคืนค่าเดิมและรีบูตใน 3 นาทีหากไม่กดยืนยัน"** (New configuration trial mode: system will revert and reboot in 3 minutes if not confirmed).
    - The banner contains a **ยืนยัน (Confirm)** button and a **ยกเลิก (Cancel)** button.
    - Clicking **ยืนยัน (Confirm)** sends a `POST /api/confirm` request, makes the configuration permanent, and hides the banner.
    - Clicking **ยกเลิก (Cancel)** sends a `DELETE /api/confirm` request, rolls back the staging parameters, and triggers a delayed reboot.
    - Wi-Fi connections do not use staging and are applied immediately without a confirmation banner.
result: pass

### 3. Verify Hardware Button Recovery AP
expected: |
  - Action: Press and hold the BOOT/Recovery button (GPIO 0) or the external button (GPIO 4) for **2 to 5 seconds**.
  - Expectation:
    - The onboard blue LED (GPIO 2) and external LED (GPIO 5) start blinking slowly (500ms intervals).
    - Release the button when the slow blink starts.
    - The LEDs remain **solid ON**.
    - The SoftAP `FishPump-Setup` becomes visible on a Wi-Fi scan and can be connected to at `http://192.168.4.1`.
result: pass

### 4. Verify Hardware Button Factory Reset
expected: |
  - Action: Press and hold the BOOT/Recovery button (GPIO 0) or the external button (GPIO 4) for **more than 5 seconds**.
  - Expectation:
    - The onboard blue LED and external LED transition from slow blinking to **fast blinking (100ms intervals)**.
    - Release the button.
    - The system logs the credentials reset to default (`admin`/`admin123`).
    - Active memory sessions are invalidated (forcing re-login).
    - The SoftAP remains active (no reboot).
result: pass

### 5. Verify Auto-Rollback on Connection Failure (Anti-Lockout)
expected: |
  - Action: Configure a staged credential update and do not click confirm, or let it timeout.
  - Expectation:
    - The device boots, waits for confirmation, and if not confirmed within 3 minutes, automatically rolls back the staged credentials and reboots.
result: pass

## Summary

total: 5
passed: 5
issues: 0
pending: 0
skipped: 0

## Gaps

None. All UAT gaps have been closed successfully.
