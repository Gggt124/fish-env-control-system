---
quick_id: 260520-siy
status: complete
date: 2026-05-20
commit: b7e88c7
---

# Quick Task 260520-siy Summary

## Result

Switched the project from a 1MB factory app partition to a 4MB-flash-compatible dual OTA partition layout.

## Changes

- Added an `otadata` partition at `0xE000` sized `0x2000`.
- Moved `phy_init` to `0x10000`.
- Replaced the single `factory` app partition with:
  - `ota_0` at `0x20000`, size `0x1F0000`
  - `ota_1` at `0x210000`, size `0x1F0000`
- Updated README and AGENTS documentation for the dual-OTA layout.

## Validation

- `.\scripts\build.ps1` passed.
- ESP-IDF generated the partition table with `ota_0` and `ota_1` at 1984K each.
- Build size check passed: binary `0xf38a0`, smallest app partition `0x1f0000`, free `0xfc760` bytes (51%).
- `Test-Path build\fish_pump_relay_timer_control.bin` returned `True`.

## Notes

- This change makes the flash layout OTA-ready but does not add OTA update logic or UI.
- `idf.py flash` now writes `ota_data_initial.bin` and the app to `0x20000`; the project flash wrapper handles this through ESP-IDF.

