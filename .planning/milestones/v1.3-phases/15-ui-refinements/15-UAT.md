---
status: diagnosed
phase: 15-ui-refinements
source: [15-01-SUMMARY.md]
started: 2026-06-05T01:28:44Z
updated: 2026-06-05T02:03:00Z
---

## Current Test

[testing complete]

## Tests

### 1. Hardware GPIO Mapping Dirty Checks
expected: |
  Navigate to Hardware page, modify any dropdown select value, and observe the unsaved warning. Revert the dropdown selection back to its original value.
  Expected outcome: Unsaved warning "Unsaved GPIO map changes" appears on change and disappears immediately on reverting back. The save button is disabled when dropdowns match baseline.
result: pass

### 2. Pump & Cooling Settings Dirty Checks
expected: |
  Open the Owner Dashboard. Check the Pump Settings and Cooling Settings save buttons. Modify any input fields in both forms, and then revert them back to baseline values.
  Expected outcome: Save buttons are disabled by default. Editing fields shows "มีการแก้ไขที่ยังไม่ได้บันทึก" and enables the save buttons. Reverting inputs to baseline hides the warning and disables the save buttons.
result: pass

### 3. Red Danger Button styling & Active Mode Logic
expected: |
  Look at the Cooling Settings form on the dashboard. Trigger different cooling modes (Auto, Force OFF, Test ON).
  Expected outcome: The "Force OFF" button is styled red. The active cooling mode button is disabled (e.g. if auto is active, the Auto button is disabled; if Force OFF is active, Force OFF is disabled).
result: issue
reported: "test on ถ้าติด lockout ให้มัน pending อย่าพึ่งนับเวลา ไม่งั้นเวลา ทำงานของ test on จะเป็น teston - lockout // ระหว่าง pending ก็ สลับ auto หรือ test on ได้ตามปกติ"
severity: major

### 4. Sidebar Logout Confirmation
expected: |
  Click the logout link in the sidebar menu.
  Expected outcome: A native confirm box prompts in Thai: "คุณต้องการออกจากระบบใช่หรือไม่?". Click Cancel to stay on the page. Click OK to log out and redirect to `/login`.
result: pass

### 5. Wi-Fi Disconnect Flow
expected: |
  Navigate to the Wi-Fi settings page and click the disconnect button.
  Expected outcome: Button displays "Disconnect". On click, prompts "คุณต้องการตัดการเชื่อมต่อ Wi-Fi ใช่หรือไม่?". Cancel aborts. OK disables the button, changes text to "Disconnecting...", and triggers disconnect before showing success/error toast.
result: pass

## Summary

total: 5
passed: 4
issues: 1
pending: 0
skipped: 0

## Gaps

- truth: "Test ON mode respects compressor protection lockout without subtracting lockout duration from Test ON runtime"
  status: failed
  reason: "User reported: test on ถ้าติด lockout ให้มัน pending อย่าพึ่งนับเวลา ไม่งั้นเวลา ทำงานของ test on จะเป็น teston - lockout // ระหว่าง pending ก็ สลับ auto หรือ test on ได้ตามปกติ"
  severity: major
  test: 3
  root_cause: "Test ON mode tracks its remaining duration using a fixed absolute deadline timestamp s_test_deadline_ms. When compressor protection lockout is active, the countdown continues to decrement in real-time as now_ms increments, resulting in the lockout duration being subtracted from Test ON runtime."
  artifacts:
    - path: "components/cooling_control/cooling_control.c"
      issue: "s_test_deadline_ms decrements in real-time regardless of lockout state"
  missing:
    - "Replace s_test_deadline_ms with s_test_remaining_ms and s_last_test_update_ms to track elapsed time"
    - "Only decrement s_test_remaining_ms in update_control_locked when lockout is inactive"
    - "Initialize s_test_remaining_ms and s_last_test_update_ms on starting Test ON mode"
  debug_session: ".planning/debug/test-on-lockout-pending.md"
