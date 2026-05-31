---
status: complete
date: 2026-05-31
commit: a4aa645
---

# Quick Task 260531-nag Summary

## Completed

- Added an explicit `แสดงรหัสผ่าน` checkbox below the Wi-Fi password field.
- Enabled the checkbox only after selecting a network.
- Reset the checkbox and password field back to hidden when selection clears.

## Validation

- `node --check main\static\app.js` passed.
- `.\scripts\build.ps1` generated `build\fish_pump_relay_timer_control.bin`.
- `.\scripts\flash.ps1 -Port COM5` flashed successfully to `ESP32-D0WD-V3`.

## Notes

- Manual mobile verification remains: select a network, type a password, and
  confirm the checkbox reveals and hides the value before connecting.
