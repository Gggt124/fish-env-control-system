---
status: complete
phase: 03-hardware-recovery-anti-lockout
source: [03-01-SUMMARY.md, 03-02-SUMMARY.md, 03-03-SUMMARY.md, 03-04-SUMMARY.md]
started: 2026-06-13T22:21:00Z
updated: 2026-06-14T06:09:00Z
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
  - Setup: Change the admin credentials or save a new Wi-Fi connection profile.
  - Action: Open the dashboard, status, or Wi-Fi configuration page in your browser.
  - Expectation:
    - A yellow warning banner is displayed at the top: **"โหมดทดลองใช้การตั้งค่าใหม่: ระบบจะคืนค่าเดิมและรีบูตใน 3 นาทีหากไม่กดยืนยัน"** (New configuration trial mode: system will revert and reboot in 3 minutes if not confirmed).
    - The banner contains a **ยืนยัน (Confirm)** button and a **ยกเลิก (Cancel)** button.
    - Clicking **ยืนยัน (Confirm)** sends a `POST /api/confirm` request, makes the configuration permanent, and hides the banner.
    - Clicking **ยกเลิก (Cancel)** sends a `DELETE /api/confirm` request, rolls back the staging parameters, and triggers a delayed reboot.
result: issue
reported: "ปัญหา 1.เวลา ล็อคอินหลังจากเปลี่ยนรหัสแล้วขึ้น ให้ยืนยันตามปกติ แต่ กดยืนยันที่ปุ่มที่แถบแจ้งเตือนแล้วมันขึ้น ยืนยันของ brownser มาอีก ทำให้ UX แย่ลง /// 2.  ล็อคอินหลังจากเปลี่ยนรหัสแล้วขึ้น ให้ยืนยันตามปกติ แต่พอยืนยันเรียบร้อยแล้วมันก็เด้งไปหน้าล็อกอินอีกรอบ ทำให้โดยรวมมันเด้งไป 2 รอบเลย /// 3. กดเชื่อมต่อไวไฟที่บันทึก ไว้แล้วมันก็ยังขึ้นให้ยืนยัน แถมกดยืนยันแล้วไม่หายทันที ต้องรีโหลดหน้าจอ ถึงจะหาย น่าจะเพราะ ap ถูกปิดในทันทีเลยยืนยันไม่ได้ แต่ผมคิดว่าในด้าน UX มันควรจะไม่ต้องยืนยันถ้าเป็น wifi เดิมที่บันทึกแล้ว"
severity: major

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
  - Action: Configure a staged Wi-Fi connection with a wrong password and let the device reboot to validate. Do not click confirm.
  - Expectation:
    - The device boots, tries to connect to the staged profile, fails to get an IP address, and times out after 30 seconds.
    - The device automatically rolls back the staged credentials and reboots.
    - On the subsequent boot, the device reconnects to the previous known-good Wi-Fi profile.
result: pass

## Summary

total: 5
passed: 4
issues: 1
pending: 0
skipped: 0

## Gaps

- truth: "Verify Staging Confirmation UI Banner"
  status: diagnosed
  reason: "User reported: ปัญหา 1.เวลา ล็อคอินหลังจากเปลี่ยนรหัสแล้วขึ้น ให้ยืนยันตามปกติ แต่ กดยืนยันที่ปุ่มที่แถบแจ้งเตือนแล้วมันขึ้น ยืนยันของ brownser มาอีก ทำให้ UX แย่ลง /// 2.  ล็อคอินหลังจากเปลี่ยนรหัสแล้วขึ้น ให้ยืนยันตามปกติ แต่พอยืนยันเรียบร้อยแล้วมันก็เด้งไปหน้าล็อกอินอีกรอบ ทำให้โดยรวมมันเด้งไป 2 รอบเลย /// 3. กดเชื่อมต่อไวไฟที่บันทึก ไว้แล้วมันก็ยังขึ้นให้ยืนยัน แถมกดยืนยันแล้วไม่หายทันที ต้องรีโหลดหน้าจอ ถึงจะหาย น่าจะเพราะ ap ถูกปิดในทันทีเลยยืนยันไม่ได้ แต่ผมคิดว่าในด้าน UX มันควรจะไม่ต้องยืนยันถ้าเป็น wifi เดิมที่บันทึกแล้ว"
  severity: major
  test: 2
  root_cause: "1. `app.js` uses `alert()` instead of `showToast()`. 2. `web_server.c` unconditionally invalidates the session upon confirm. 3. `web_server.c` forces staging and reboot for known/saved Wi-Fi profiles without checking the saved list."
  artifacts:
    - path: "main/static/app.js"
      issue: "Uses `alert()` which creates a blocking browser popup."
    - path: "main/web_server.c"
      issue: "Forces session invalidation on confirm, and bypasses saved profile checks causing redundant staging."
  missing:
    - "Replace `alert()` with `showToast()` in `confirmStaging()`"
    - "Remove `session_invalidate_all()` from `handle_api_confirm_post()`"
    - "Add check for existing profiles in `handle_api_wifi_profiles_save()` to skip staging if SSID is already saved"
  debug_session: ".planning/debug/staging-ui-banner.md"
