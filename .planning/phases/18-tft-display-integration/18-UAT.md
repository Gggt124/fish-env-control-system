---
status: diagnosed
phase: 18-tft-display-integration
source: [18-01-SUMMARY.md, 18-02-SUMMARY.md]
started: 2026-06-06T07:00:46+07:00
updated: 2026-06-06T18:43:34+07:00
---

## Current Test

[testing complete]

## Tests

### 1. Cold Start Smoke Test
expected: Power-cycle the ESP32 from off. Firmware boots without panic/abort, TFT backlight turns on, splash renders, and system transitions to dashboard. Serial log clean (no Guru Meditation, no watchdog reset).
result: pass

### 2. TFT Backlight Activation
expected: On boot, the TFT backlight (GPIO4) turns on immediately. The screen is visibly lit (not black/dark).
result: pass

### 3. Boot Splash Screen
expected: Immediately after boot, the screen shows a centered "Booting..." message in green (2x scale), with the product name in white and the version in gray, all on a black background.
result: pass

### 4. Transition To Dashboard Layout
expected: After init completes, the splash clears and the screen shows the dual-column landscape dashboard (320x240): vertical separator line, left column with pump/timer fields, right column with WiFi/uptime/cooling fields. All static labels visible and aligned.
result: issue
reported: "??????????????????? // ????????? ???? ip ?????????? sta ?????? ip ???????????? AP ?????????????????? ////???????????????????????? sta ????????? Ip ?????? ??????????????????????? ??? IP ??????"
severity: cosmetic

### 5. WiFi IP Updates
expected: The WiFi IP field on the dashboard shows the current STA IP when connected, or a clear "not connected"/AP IP indication otherwise. When you connect/disconnect Wi-Fi, the value updates within ~1 second without a full screen redraw or flicker.
result: pass

### 6. Pump State Updates (Float Switch / Timer)
expected: When the pump turns ON/OFF (via float switch toggle or timer phase change), the dashboard's pump state, active timer label, and phase update within ~1 second. Only the changed fields redraw — no flicker on unchanged fields.
result: issue
reported: "?????????????? status ?? stop ??? relay ??????????????? ??????  phase ??????????? on"
severity: major

### 7. Countdown Timer Ticks Smoothly
expected: The pump countdown (remaining time) decrements visibly every 1 second when a timer phase is active. Digits update cleanly without flicker, ghosting, or skipped seconds.
result: pass

### 8. Uptime Counter Increments
expected: The uptime field increments every 1 second (HH:MM:SS or similar). Counter ticks smoothly and does not glitch.
result: pass

### 9. Cooling Temperature & Relay State Updates
expected: The cooling temperature (DS18B20) and cooling relay state are displayed and refresh as the sensor reading / relay state changes. Lockout/Mode fields reflect actual cooling controller state.
result: pass

### 10. Long-Run Stability (No Watchdog / No Crashes)
expected: Let the device run for a few minutes with the dashboard active. No watchdog panics, no resets, no display freezes. Serial monitor stays clean. Other subsystems (web server, Wi-Fi, pump control) continue to function normally â€” TFT task does not starve them.
result: pass

## Summary

total: 10
passed: 8
issues: 2
pending: 0
skipped: 0
blocked: 0

## Gaps\r\n\r\n[none yet]
- truth: "After init completes, the splash clears and the screen shows the dual-column landscape dashboard (320x240): vertical separator line, left column with pump/timer fields, right column with WiFi/uptime/cooling fields. All static labels visible and aligned."
  status: failed
  reason: "User reported: ??????????????????? // ????????? ???? ip ?????????? sta ?????? ip ???????????? AP ?????????????????? ////???????????????????????? sta ????????? Ip ?????? ??????????????????????? ??? IP ??????"
  severity: cosmetic
  test: 4
  root_cause: "The wifi string is formatted with %-13.13s limitation in 	ft_display.c, cutting off strings longer than 13 characters like 'AP:192.168.4.1' (14 chars) or long STA IP addresses."
  artifacts:
    - path: "main/tft_display.c"
      issue: "snprintf(wifi_formatted, sizeof(wifi_formatted), "%-13.13s", wifi_str); is too short for AP IP or long STA IPs."
  missing:
    - "Increase format specifier width from 13 to 15 (e.g., %-15.15s and ensure array is large enough, like char wifi_formatted[16];) to accommodate longer IP formats."
- truth: "When the pump turns ON/OFF (via float switch toggle or timer phase change), the dashboard's pump state, active timer label, and phase update within ~1 second. Only the changed fields redraw — no flicker on unchanged fields."
  status: failed
  reason: "User reported: ?????????????? status ?? stop ??? relay ??????????????? ??????  phase ??????????? on"
  severity: major
  test: 6
  root_cause: "When pump_running is false (status is STOPPED), the pump.phase variable from backend might still reflect the active timer's current cycle (e.g., ON phase), causing the TFT to show 'ON' phase while the relay itself is completely stopped."
  artifacts:
    - path: "main/tft_display.c"
      issue: "The Phase field UI logic (line 405-417) blindly renders the pump.phase value without considering if the pump is actually RUNNING (pump_running)."
  missing:
    - "Override the Phase display string to 'IDLE' or '-' when !pump_running (i.e. Pump Status is STOPPED) so it accurately reflects that the relay output is inactive."