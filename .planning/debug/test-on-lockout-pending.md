---
status: resolved
trigger: "test on ถ้าติด lockout ให้มัน pending อย่าพึ่งนับเวลา ไม่งั้นเวลา ทำงานของ test on จะเป็น teston - lockout // ระหว่าง pending ก็ สลับ auto หรือ test on ได้ตามปกติ"
created: 2026-06-05T01:54:35Z
updated: 2026-06-05T12:57:00Z
---

## Current Focus

hypothesis: check current cooling/lockout timer logic in main/web_server.c, main/app_main.c, and components/
test: search for cooling state and timer decrement logic
expecting: find where Test ON duration is decremented during lockout
next_action: Return ROOT CAUSE FOUND structured output to orchestrator

## Symptoms

expected: Test ON mode respects compressor protection lockout without subtracting lockout duration from Test ON runtime. When lockout is active, the Test ON state should be pending and the countdown should not decrement.
actual: test on ถ้าติด lockout ให้มัน pending อย่าพึ่งนับเวลา ไม่งั้นเวลา ทำงานของ test on จะเป็น teston - lockout // ระหว่าง pending ก็ สลับ auto หรือ test on ได้ตามปกติ
errors: None reported
reproduction: Trigger Test ON mode during cooling lockout period in UAT Test 3
started: Discovered during UAT

## Eliminated

## Evidence

- timestamp: 2026-06-05T02:01:00+07:00
  checked: components/cooling_control/cooling_control.c
  found: In `cooling_control_start_test()`, `s_test_deadline_ms` is set as an absolute timestamp `now_ms + (test_timeout_sec * 1000)`.
  implication: The Test ON duration counts down continuously based on real-time elapsed, regardless of system state.
- timestamp: 2026-06-05T02:01:30+07:00
  checked: components/cooling_control/cooling_control.c
  found: In `update_control_locked()`, if `s_lockout_active` is true, the relay is blocked from energizing, but there is no logic to shift/extend `s_test_deadline_ms` or pause the countdown.
  implication: The Test ON duration decrements during compressor lockout, reducing actual test runtime by the lockout period.

## Resolution

root_cause: Test ON mode tracks its remaining duration using a fixed absolute deadline `s_test_deadline_ms` set at test start (`now_ms + test_timeout_sec`). When the compressor protection lockout is active (`s_lockout_active`), `update_control_locked()` blocks the relay from energizing, but the absolute deadline `s_test_deadline_ms` continues to tick down in real-time. This effectively subtracts the lockout duration from the active Test ON duration.
fix: Modify the state machine to track elapsed Test ON active time or dynamically extend `s_test_deadline_ms` by the lockout duration (shifting it forward) whenever the control loop is updated while Test ON is active and lockout blocks the relay, thereby keeping the test duration pending.
verification: Manual testing and validation in UAT environment.
files_changed: []
