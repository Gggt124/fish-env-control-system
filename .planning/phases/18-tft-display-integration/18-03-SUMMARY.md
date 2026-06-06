---
phase: 18-tft-display-integration
plan: "03"
subsystem: ui
tags: [freertos, task-stack, debug, config, esp-idf]

# Dependency graph
requires:
  - phase: 18-tft-display-integration
    provides: "TFT driver initialization, dashboard skeleton, and background update task that previously crashed silently on entry"
provides:
  - "Hardened tft_display_task with 8KB stack and static 640-byte pixel chunk buffer"
  - "Observable xTaskCreate lifecycle (log on success/failure)"
  - "FreeRTOS stack overflow detection (method 2 / canary) enabled project-wide"
affects: [18-tft-display-integration]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Static file-scope pixel chunk buffer (640B) for tft_fill_rect to keep SPI draw buffers off any task stack"
    - "Capture-and-log xTaskCreate return value for observable task creation"

key-files:
  created: []
  modified: [main/tft_display.c, sdkconfig.defaults]

key-decisions:
  - "Move 640-byte chunk_buf to file-scope static (was: stack-local in tft_fill_rect). Safe because all draw paths serialize via s_trans_done_sem and the boot-splash main-task path runs to completion before the new task starts."
  - "Set xTaskCreate stack to 8192 bytes — 2.7x the previous 3072, with comfortable headroom over the ~3KB combined task-local + format-buffer + draw-buffer budget."
  - "Capture xTaskCreate return in BaseType_t and log both success and failure paths so any future heap-exhaustion or stack-allocation failure surfaces in the serial monitor."
  - "CONFIG_FREERTOS_CHECK_STACKOVERFLOW=2 (canary) — development-time panic-on-overflow safety net; document that this must be reduced for unattended production deployment."

patterns-established:
  - "Pattern: Always capture xTaskCreate return value and log both branches. Silent failure on task creation is the single hardest class of bug to diagnose from logs alone."

requirements-completed: [TFT-04]

# Metrics
duration: 5min
completed: 2026-06-06
---

# Phase 18 Plan 03: tft_display_task Stack Hardening Summary

**Fix the blocker gap (UAT test 3) where the TFT background update task was created but never executed its first statement: 3072-byte task stack was overflowing under the deep call chain through `tft_display_draw_dashboard_skeleton` -> `tft_fill_rect` with a 640-byte stack-local buffer.**

## Performance

- **Duration:** ~5 min (source edits + build verification)
- **Tasks:** 2 of 3 (Task 3 is hardware human-verify checkpoint, pending user flash)
- **Files modified:** 2

## Accomplishments

- Moved the 640-byte `chunk_buf[320]` in `tft_fill_rect` to file-scope static storage. Eliminates the largest single stack consumer in the draw path and keeps the new tft_display_task stack well under its 8192-byte budget.
- Bumped `xTaskCreate` stack from 3072 to 8192 bytes for the TFT background update task.
- Added a `BaseType_t created` capture around the `xTaskCreate` call with `ESP_LOGI` on success and `ESP_LOGE` on failure, so any future task-creation failure (heap exhaustion, etc.) is visible in the serial log instead of being silently dropped.
- Added a pre-create `ESP_LOGI` so the boot path is observable.
- Enabled `CONFIG_FREERTOS_CHECK_STACKOVERFLOW=2` (canary method) project-wide. Future stack overflows will panic visibly with a Guru Meditation instead of silently corrupting memory.

## Task Commits

1. **Task 1: Harden tft_display_task stack, reduce stack pressure, and instrument task creation** - `7cb56de` (fix)
2. **Task 2: Enable FreeRTOS stack overflow detection (method 2) in sdkconfig.defaults** - `f5d4346` (build)
3. **Task 3: Checkpoint - Human Verification of splash-to-dashboard transition** - Pending user flash.

## Files Modified

- `main/tft_display.c`:
  - `tft_fill_rect` (`chunk_buf[320]`): stack-local -> file-scope static
  - `tft_display_start_task`: log pre-create, capture xTaskCreate return, log success/failure, bump stack 3072 -> 8192
- `sdkconfig.defaults`:
  - Added `CONFIG_FREERTOS_CHECK_STACKOVERFLOW=2` with explanatory comment

## Decisions Made

- The static `chunk_buf` is safe because (a) the boot-splash main-task path runs to completion before the dashboard task is created (so no concurrent writers during boot), and (b) the dashboard task is the only writer after that, with all draws serialized through `s_trans_done_sem`. No concurrent access.
- Kept the file-scope `s_tft_cache` pattern in `tft_display_task` unchanged (already static, already a single-writer).
- Did NOT touch `tft_display_task`'s body (line 291 ESP_LOGI) — that first log is now the success indicator proving the task actually runs.

## Deviations from Plan

None. The plan was executed exactly as specified. Build size grew from `0x11dfd0` to `0x11f090` (about 4KB), consistent with the additional FreeRTOS stack-overflow instrumentation overhead.

## Issues Encountered

None at the source level. Task 3 (hardware verification) is pending — the user must flash the rebuilt firmware and confirm the dashboard now appears.

## User Setup Required

User must flash the new firmware on the ESP32 board and confirm the splash-to-dashboard transition works. After flashing:

1. Watch the serial log for the new lines:
   - `Creating TFT display background update task (stack=8192, prio=4)` (boot context)
   - `TFT display background update task started` (first statement of task body)
   - `tft_display_task created successfully` (post-create success)
2. Confirm the TFT transitions from the "Booting..." splash to the dual-column landscape dashboard within ~5 seconds of power-on.
3. Leave idle for 2 minutes and confirm no panics, watchdog resets, or display freezes.
4. Open the web UI (http://192.168.4.1 or STA IP) and confirm it still works (no regression to non-TFT subsystems).

Resume signal: type "approved" once the dashboard is visible and the new log line is present. If the dashboard is still missing, describe the symptoms and serial log. If the dashboard is visible but visually still wrong (mirrored/cyan/white-band), type "visual-still-broken" so the executor proceeds to plan 18-04 (mirror + gap fix).

## Next Phase Readiness

- Task 1 of plan 18-04 (mirror-flag flip + set_gap) is ready to execute and depends only on this plan's source edits being on the branch.
- 18-04 is independent of whether the user's hardware passes the human-verify — the source fix addresses a separate root cause (orientation/gap), and the conditional Task 3 (ST7789 fallback) is gated by the user's hardware result.

---
*Phase: 18-tft-display-integration*
*Completed: 2026-06-06*

## Self-Check: PASSED
