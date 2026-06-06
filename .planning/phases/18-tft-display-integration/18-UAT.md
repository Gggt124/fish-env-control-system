---
status: diagnosed
phase: 18-tft-display-integration
source: [18-01-SUMMARY.md, 18-02-SUMMARY.md]
started: 2026-06-06T07:00:46+07:00
updated: 2026-06-06T07:30:00+07:00
---

## Current Test

[testing complete]

## Tests

### 1. Cold Start Smoke Test
expected: Power-cycle the ESP32 from off. Firmware boots without panic/abort, TFT backlight turns on, splash renders, and system transitions to dashboard. Serial log clean (no Guru Meditation, no watchdog reset).
result: issue
reported: "Tried it, got these problems (from photos): splash text 'Booting...' / 'Fish Pump Relay Timer Co...' / 'v0.1.0' renders but is mirrored/reversed; background is cyan/light-blue instead of black; bright white vertical band on the right portion of the screen. Backlight is on; no visible panic — boot completed."
severity: major

### 2. TFT Backlight Activation
expected: On boot, the TFT backlight (GPIO4) turns on immediately. The screen is visibly lit (not black/dark).
result: pass

### 3. Boot Splash Screen
expected: Immediately after boot, the screen shows a centered "Booting..." message in green (2x scale), with the product name in white and the version in gray, all on a black background.
result: issue
reported: "Shows 'Booting...' but with the same rendering issues as the photo (mirrored text, cyan background, white band on right) — AND the device is stuck on the splash, never transitions to other screens."
severity: blocker

### 4. Transition To Dashboard Layout
expected: After init completes, the splash clears and the screen shows the dual-column landscape dashboard (320x240): vertical separator line, left column with pump/timer fields, right column with WiFi/uptime/cooling fields. All static labels visible and aligned.
result: blocked
blocked_by: other
reason: "Boot hangs on splash screen — dashboard never appears (see gap on test 3)."

### 5. WiFi IP Updates
expected: The WiFi IP field on the dashboard shows the current STA IP when connected, or a clear "not connected"/AP IP indication otherwise. When you connect/disconnect Wi-Fi, the value updates within ~1 second without a full screen redraw or flicker.
result: blocked
blocked_by: other
reason: "Dashboard never renders — boot hangs on splash (see gap on test 3)."

### 6. Pump State Updates (Float Switch / Timer)
expected: When the pump turns ON/OFF (via float switch toggle or timer phase change), the dashboard's pump state, active timer label, and phase update within ~1 second. Only the changed fields redraw — no flicker on unchanged fields.
result: blocked
blocked_by: other
reason: "Dashboard never renders — boot hangs on splash (see gap on test 3)."

### 7. Countdown Timer Ticks Smoothly
expected: The pump countdown (remaining time) decrements visibly every 1 second when a timer phase is active. Digits update cleanly without flicker, ghosting, or skipped seconds.
result: blocked
blocked_by: other
reason: "Dashboard never renders — boot hangs on splash (see gap on test 3)."

### 8. Uptime Counter Increments
expected: The uptime field increments every 1 second (HH:MM:SS or similar). Counter ticks smoothly and does not glitch.
result: blocked
blocked_by: other
reason: "Dashboard never renders — boot hangs on splash (see gap on test 3)."

### 9. Cooling Temperature & Relay State Updates
expected: The cooling temperature (DS18B20) and cooling relay state are displayed and refresh as the sensor reading / relay state changes. Lockout/Mode fields reflect actual cooling controller state.
result: blocked
blocked_by: other
reason: "Dashboard never renders — boot hangs on splash (see gap on test 3)."

### 10. Long-Run Stability (No Watchdog / No Crashes)
expected: Let the device run for a few minutes with the dashboard active. No watchdog panics, no resets, no display freezes. Serial monitor stays clean. Other subsystems (web server, Wi-Fi, pump control) continue to function normally — TFT task does not starve them.
result: pass

## Summary

total: 10
passed: 2
issues: 2
pending: 0
skipped: 0
blocked: 6

## Gaps

- truth: "Cold start: device boots cleanly and renders the boot splash on the TFT correctly (centered, black background, upright text)"
  status: failed
  reason: "User reported (with photos): splash renders but text is mirrored/reversed; background is cyan instead of black; bright white vertical band fills the right portion of the screen. Backlight is on. Boot completed without visible panic."
  severity: major
  test: 1
  symptoms:
    - "Mirrored / reversed text on splash (visible chars: 'Booting...', product name, 'v0.1.0' — readable when mirrored)"
    - "Background appears cyan/light-blue, not black"
    - "Bright white vertical band on the right ~30% of the screen"
  root_cause: |
    Combined panel orientation + panel-variant / window-offset mismatch.
    1) Mirror flags wrong for this physical board: tft_display.c:131 calls
       esp_lcd_panel_mirror(s_panel_handle, false, true) — needs to be flipped
       (likely (true, false) or (true, true)) so X axis is no longer reversed.
    2) Visible cyan background + ~80px white band on the right strongly suggest
       either (a) the physical panel is actually ST7789 (very common on chinese
       2.4" 240x320 SPI boards marketed as ILI9341) rather than ILI9341, OR
       (b) the panel needs explicit X/Y gap offsets (esp_lcd_panel_set_gap).
       Code currently installs ili9341 driver only; never tries st7789. The
       untouched right strip is panel RAM that was never addressed/cleared
       because the active window calculated from swap_xy+mirror is offset
       from the visible display area.
  artifacts:
    - path: "main/tft_display.c"
      line: 131
      issue: "esp_lcd_panel_mirror(false, true) — X axis flipped wrong way for this board"
    - path: "main/tft_display.c"
      line: 102
      issue: "esp_lcd_new_panel_ili9341 may be the wrong driver for the actual panel — needs to be checked, possibly try st7789 driver via espressif/esp_lcd_st7789"
    - path: "main/tft_display.c"
      line: 125-135
      issue: "No esp_lcd_panel_set_gap call — some 240x320 variants need a 0/80 offset"
  missing:
    - "Try toggling esp_lcd_panel_mirror flags ((true,false) and (true,true)) to find the correct orientation"
    - "Add ST7789 driver dependency (espressif/esp_lcd_st7789) as alternative and detect/select the right driver, OR confirm panel chip from physical markings"
    - "Add esp_lcd_panel_set_gap() if offsets are needed for the active panel variant"
    - "Black background on splash should fill the entire visible area after orientation is correct"
  debug_session: "Inline diagnosis from serial log + source review (see UAT chat 2026-06-06)"

- truth: "Boot completes through TFT init and the firmware transitions from the splash to the dashboard within a few seconds"
  status: failed
  reason: "User reported: device shows 'Booting...' splash but is stuck — never transitions to the dashboard or any other screen. Splash also has the rendering bugs from test 1 (mirrored text, cyan background, white band on the right)."
  severity: blocker
  test: 3
  symptoms:
    - "Device hangs on splash screen indefinitely — dashboard skeleton never appears"
    - "Serial log shows the firmware itself reaches 'System Ready' and STA gets IP 192.168.1.39 — the firmware is NOT hung, only the screen never advances past the splash"
    - "Critical missing log: tft_display.c:292 should print 'TFT display background update task started' as the first thing the task does — this log is absent in the serial trace, proving the task body never executes its first statement"
  root_cause: |
    The TFT update task is created via xTaskCreate in tft_display.c:475 with only
    3072 bytes of stack, then immediately crashes or fails to start before
    reaching its first ESP_LOGI on line 292. Strongest hypothesis: stack
    overflow on task entry.
    Stack budget for tft_display_task:
      ~1.5KB FreeRTOS/IDF task overhead (saved context, task control block use)
      ~640 bytes for uint16_t chunk_buf[320] in tft_fill_rect (called from
        tft_display_draw_dashboard_skeleton on line 295)
      ~256-512 bytes for ESP_LOGI vprintf format buffering
      ~120 bytes for local snprintf char buffers in the while loop
      ~ pump_control_status_t + cooling_control_status_t struct locals
      Total: ~3KB+ vs 3072 allocated → overflow likely on entry or first deep
      call into tft_fill_rect.
    Secondary risk: xTaskCreate return value is not checked, so if creation
    itself fails (e.g. out-of-heap because Wi-Fi/web stack consumed it earlier),
    we get the same silent failure.
    Tertiary risk: even if stack is fine, holding s_trans_done_sem across
    many sequential draws under concurrent Wi-Fi/SPI activity could deadlock
    if the on_color_trans_done callback ever races — but this is less likely
    since splash render already worked from main task.
  artifacts:
    - path: "main/tft_display.c"
      line: 475
      issue: "xTaskCreate uses 3072-byte stack which is too tight for this task; also return value not checked"
    - path: "main/tft_display.c"
      line: 291-295
      issue: "tft_display_task entry: ESP_LOGI then calls tft_display_draw_dashboard_skeleton (deep call chain into tft_fill_rect with 640-byte stack buffer)"
    - path: "main/tft_display.c"
      line: 263-289
      issue: "tft_display_draw_dashboard_skeleton uses tft_clear → tft_fill_rect → 640-byte stack allocation"
    - path: "main/app_main.c"
      line: 634
      issue: "tft_display_start_task() called but no log emitted on success/failure of task creation"
  missing:
    - "Increase tft_display_task stack to at least 6144 bytes (preferably 8192)"
    - "Check xTaskCreate return value and ESP_LOGE if it fails"
    - "Add ESP_LOGI before xTaskCreate in tft_display_start_task() for traceability"
    - "Consider moving uint16_t chunk_buf[320] in tft_fill_rect to static (out of stack) or heap-allocated"
    - "Consider enabling FreeRTOS stack overflow detection (CONFIG_FREERTOS_CHECK_STACKOVERFLOW=2) so future stack issues panic visibly instead of silently corrupting"
  debug_session: "Inline diagnosis from serial log + source review (see UAT chat 2026-06-06)"
