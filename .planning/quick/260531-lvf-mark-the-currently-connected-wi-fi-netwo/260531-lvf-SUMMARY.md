---
status: complete
date: 2026-05-31
commit: 5c210d7
---

# Quick Task 260531-lvf Summary

## Completed

- Added `connected` to `/api/wifi/scan` entries for the active STA SSID.
- Rendered the active SSID as a disabled `เชื่อมต่ออยู่` status row instead of
  allowing another password flow.
- Short-circuited direct `/api/wifi/connect` calls for the SSID that is already
  active so they do not restart APSTA transitions.

## Validation

- `node --check main\static\app.js` passed.
- `.\scripts\build.ps1` generated `build\fish_pump_relay_timer_control.bin`.
- `.\scripts\flash.ps1 -Port COM5` flashed successfully to `ESP32-D0WD-V3`.

## Notes

- Manual browser verification remains: scan while STA is connected and confirm
  the active row shows `เชื่อมต่ออยู่` and cannot open the password panel.
