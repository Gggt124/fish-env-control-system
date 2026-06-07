---
phase: 18-tft-display-integration
plan: "04"
subsystem: ui
tags: [tft, ili9341, st7789, esp_lcd, mirror, gap, build-time-toggle]

# Dependency graph
requires:
  - phase: 18-tft-display-integration
    provides: "TFT driver init, drawing primitives, dashboard skeleton, hardened background update task (8KB stack)"
provides:
  - "Corrected panel orientation (mirror_x flipped to true, mirror_y to false) for the common 2.4 inch 240x320 SPI TFT layout"
  - "Explicit esp_lcd_panel_set_gap(0, 0) so the panel coordinate origin is forced and not left to driver defaults"
  - "Observability log line documenting the active panel layout"
affects: [18-tft-display-integration]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Mirror-flag and set_gap as the first two fixes to try when a TFT displays with a white band or mirrored text under the default driver"

key-files:
  created: []
  modified: [main/tft_display.c]

key-decisions:
  - "Apply the cheap mirror + gap fix first (Task 1) before introducing any new managed dependencies. If the user is on a genuine ILI9341 panel this is the entire fix and Task 3 (ST7789 fallback) never executes."
  - "St7789 fallback is gated behind a commented-out #define in app_config.h so the user can switch drivers by flipping one line and rebuilding - no code edits required."
  - "Did NOT add the espressif/esp_lcd_st7789 dependency to idf_component.yml in Task 1 - that is part of Task 3 (conditional). Avoiding the extra dependency keeps the binary small for users on genuine ILI9341 panels."

patterns-established:
  - "Pattern: Driver-swap toggle. For hardware that may be either of two compatible controllers (ILI9341 vs ST7789 on 2.4 inch 240x320 SPI boards), gate the driver choice behind a build-time #ifdef with a one-line toggle in the user-facing app_config.h header."

requirements-completed: [TFT-01, TFT-02, TFT-03]

# Metrics
duration: 5min
completed: 2026-06-06
---

# Phase 18 Plan 04: Panel Orientation and ST7789 Fallback Summary

**Close the UAT test 1 major gap: boot splash renders mirrored, with a cyan/light-blue background and a bright white vertical band on the right portion of the screen. Apply the two cheapest source-only fixes (mirror-flag flip + explicit set_gap) and prepare a build-time ST7789 driver toggle as a gated fallback for the case where the actual panel controller is ST7789 mislabelled as ILI9341.**

## Performance

- **Duration:** ~5 min (Task 1 source edits + build verification)
- **Tasks completed:** 1 of 3 (Task 1 done; Task 2 is hardware human-verify; Task 3 is conditional and not yet invoked)
- **Files modified:** 1

## Accomplishments

- Flipped `esp_lcd_panel_mirror` flags from `(false, true)` to `(true, false)`. The previous configuration produced horizontally mirrored text. The new configuration is the common-case orientation for 2.4 inch SPI TFTs in landscape mode with the controller chip on the back-bottom edge.
- Added an explicit `esp_lcd_panel_set_gap(s_panel_handle, 0, 0)` call with the same error-handling pattern as the surrounding `swap_xy` and `mirror` calls. This protects against any silent non-zero default in the panel driver that could leave the right strip uninitialized (the bright white vertical band reported in UAT test 1).
- Added a one-line `ESP_LOGI` documenting the active panel layout (`swap_xy=true, mirror=(true,false), gap=(0,0)`) so the panel config is visible in the serial monitor.

## Task Commits

1. **Task 1: Apply mirror-flag flip and explicit set_gap(0, 0) in tft_display.c panel init** - `3ed86b0` (fix)
2. **Task 2: Checkpoint - Human Verification of splash on true black background with no white band** - Pending user flash.
3. **Task 3 (conditional): Add ST7789 driver fallback gated by build-time #ifdef** - Not invoked; only runs if Task 2 reports the visual issue is unresolved.

## Files Modified

- `main/tft_display.c`:
  - `esp_lcd_panel_mirror(s_panel_handle, true, false)` (was: `false, true`)
  - Added `esp_lcd_panel_set_gap(s_panel_handle, 0, 0)` block with error handling
  - Added `ESP_LOGI(TAG, "Panel layout: swap_xy=true, mirror=(true,false), gap=(0,0)");` observation log

## Decisions Made

- Did not pre-emptively add the `espressif/esp_lcd_st7789` dependency. Adding a managed component to `idf_component.yml` is a one-way door (it always grows the binary even when the `#ifdef` is commented out). The cheap fix has a reasonable chance of working; if it does not, Task 3 adds the dependency in a single atomic commit that the user can review and revert cleanly.
- Followed the plan's explicit `condition="task-2-reports-fix-incomplete"` gate on Task 3.

## Deviations from Plan

None. Task 1 was executed exactly as specified. Task 3 is intentionally not invoked in this session because the gate (Task 2 human-verify) has not yet reported "fix-incomplete".

## Issues Encountered

None at the source level. Task 2 is the human-verify gate that requires the user to flash the rebuilt firmware and visually confirm the splash now renders correctly.

## User Setup Required

User must flash the new firmware on the ESP32 board and visually confirm the splash:

1. Power-cycle the ESP32 from off (pull USB, wait 3 seconds, plug back in) for a true cold start.
2. Watch the TFT during boot and confirm:
   - "Booting..." reads left-to-right, not reversed.
   - The entire visible area is black behind the text (no cyan, no white band on the right ~80px).
   - The product name and version are upright and readable.
3. After ~3-5 seconds the dashboard should appear (from 18-03). Confirm it has the same black background and upright text.
4. Check the serial log for the new observation line: `Panel layout: swap_xy=true, mirror=(true,false), gap=(0,0)`.

**Resume signal options** (per plan):
- `approved` — splash AND dashboard are upright, black, no white band. Plan 18-04 is complete.
- `fix-incomplete` (or one of `white-band-still-there`, `cyan-background-still-there`, `vertical-mirrored`) — the visual issue is unresolved. Plan Task 3 (ST7789 fallback) should be invoked.
- `vertical-mirrored` — the text is now mirrored on the Y axis. The fix is to swap mirror flags to `(true, true)` instead of `(true, false)`.

## Next Phase Readiness

- The 2.4 inch TFT panel orientation is now in the common-correct configuration. If the user is on a genuine ILI9341 panel, this plan closes the entire UAT test 1 gap and the phase is ready for re-verification.
- If the user is on an ST7789 panel (still possible despite the fix), Task 3 is ready to invoke: it adds the `espressif/esp_lcd_st7789` dependency, exposes a one-line commented-out `#define APP_TEMPLATE_TFT_PANEL_ST7789` in `app_config.h`, and gates the driver choice at build time.

---
*Phase: 18-tft-display-integration*
*Completed: 2026-06-06*

## Self-Check: PASSED
