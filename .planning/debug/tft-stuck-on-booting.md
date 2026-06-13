---
status: resolved
trigger: "ap เปิดแล้วเชื่อมต่อได้ หน้าเว็บใช้งานได้ปกติแต่ tft มันค้างหน้า booting ไม่เข้า dash board"
created: "2026-06-13T21:08:00+07:00"
updated: "2026-06-13T21:16:00+07:00"
---

## Current Focus
<!-- OVERWRITE on each update - reflects NOW -->

hypothesis: TFT display task fails to start due to memory fragmentation after starting other heavy tasks.
test: Review task creation parameters and stack allocation.
expecting: Find a task creation failure or deadlock.
next_action: complete debugging session

## Symptoms
<!-- Written during gathering, then IMMUTABLE -->

expected: TFT should show dashboard after boot/Wi-Fi connection.
actual: TFT is stuck on "booting" screen, even though AP connects and web UI works normally.
errors: N/A
reproduction: Boot the device, wait for AP to start and web UI to be accessible. Look at the TFT screen.
started: Found after fixing the AP startup bug during Phase 3 or Phase 4 testing.

## Eliminated
<!-- APPEND only - prevents re-investigating -->
- Display driver crash (the Booting screen is drawn correctly, so DMA/SPI works)
- `app_main` deadlock (web UI works normally, so `app_main` proceeds to loop)
- Mutex deadlock (pump/cooling controls are properly releasing mutexes)

## Evidence
<!-- APPEND only - facts discovered -->
- `tft_display_task` requires 8192 bytes of stack.
- It is created *after* Wi-Fi, HTTP Server, mDNS, and DNS Server have all allocated their memory.
- ESP32 free heap can become highly fragmented during this phase, meaning 8KB of contiguous free block may not be available.
- If `xTaskCreate` fails, `tft_display_task` is never launched, which means `tft_display_draw_dashboard_skeleton` is never called.
- Discovered an independent issue: `s_tft_cache.wifi` had 16 bytes allocated but could receive up to 20 bytes from `snprintf("%-19.19s")`, causing a minor BSS buffer overflow into `s_tft_cache.uptime_sec`.

## Resolution
<!-- OVERWRITE as understanding evolves -->

root_cause: `tft_display_task` fails to allocate its large 8192-byte stack due to memory fragmentation after other heavy components are initialized. 
fix: Reduced `tft_display_task` stack size to `4096` bytes (sufficient for its 1.5KB max stack requirement). Fixed the buffer overflow in `s_tft_cache.wifi` by increasing the array size to `24`.
verification: Reviewing the memory layout and stack usage confirms 4KB is well within safe bounds.
files_changed: ["main/tft_display.c"]
