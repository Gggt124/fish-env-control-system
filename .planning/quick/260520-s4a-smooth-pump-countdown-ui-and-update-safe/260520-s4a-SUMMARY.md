---
quick_id: 260520-s4a
status: complete
date: 2026-05-20
commit: c56ad65
---

# Quick Task 260520-s4a Summary

## Result

Completed the dashboard countdown smoothing and default-setting changes requested after hardware validation.

## Changes

- Reworked dashboard pump countdown display from integer decrementing to a deadline-based countdown.
- Added local ON/OFF phase rollover prediction from the loaded timer config so phase/countdown/relay labels update immediately when a timer phase ends.
- Increased pump status polling cadence to 750 ms and guarded against overlapping status requests so external float switch changes appear faster without piling up requests.
- Stabilized countdown text width with tabular numeric styling.
- Changed default pump auto-start to off.
- Changed default login password to `admin123`.
- Changed default session max age to `0` and updated session validation so `0` means no automatic timeout.
- Updated README, AGENTS, component docs, and state notes for the new defaults.

## Validation

- `node --check main/static/app.js` passed.
- `.\scripts\build.ps1` passed with ESP-IDF 6.1 and generated `build/fish_pump_relay_timer_control.bin`.
- `Test-Path build\fish_pump_relay_timer_control.bin` returned `True`.

## Notes

- Build warning remains: app partition is nearly full with about 5% free space.
- Existing NVS pump settings still override the new auto-start default until settings are cleared or saved again.
- User reported real hardware validation passed before this quick task.

