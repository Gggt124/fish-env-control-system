---
status: complete
quick_id: 260601-jk6
date: 2026-06-01
---

# Quick Task 260601-jk6 Summary

## Outcome

Expanded the active network soak into a whole-device soak:

- Enabled FreeRTOS runtime statistics with the 1 MHz ESP timer source.
- Added `[BOARD_DIAG]` every 30 seconds with wrap-safe delta CPU workload for both classic ESP32
  cores, task count, main-task watchdog subscription, reset reason, internal heap fragmentation,
  main-task stack margin, and an explicit `die_temp=unsupported` marker.
- Added `[PUMP_DIAG]` with logical float state, selected timer and relay, timer phase, countdown,
  fault state, and both pump relay states.
- Added `[COOLING_DIAG]` with the external DS18B20 temperature, sensor validity, cooling mode,
  demand, relay state, fault, lockout, timeout, threshold, and hysteresis.

Classic ESP32 internal die temperature is not read through private APIs. The external DS18B20 is
the supported field temperature source.

## Verification

- `.\scripts\build.ps1 -FullClean` passed.
- `git diff --check` passed before the final summary write.
- `.\scripts\flash.ps1 -Port COM5` flashed successfully and verified every written image hash.
- Firmware size: `0x10e170`; OTA app partition still has 46% free.
- Stopped the previous network-only logger PID `30912`; preserved
  `logs/soak-20260601-134702.log`.
- Started whole-device logger PID `11876`; current log:
  `logs/soak-hardware-20260601-141613.log`.
- Real board snapshots confirmed:
  - CPU workload samples became valid after warm-up: total `1.1-1.7%`, core 0 `2.1-3.3%`,
    core 1 `0.1%`.
  - Main TWDT subscription reports `ESP_OK`.
  - DS18B20 reports `31.62-31.88 C` with valid sensor status.
  - Pump relay 1, pump relay 2, and cooling relay report OFF in the observed stopped state.

## Closure Validation

- Final whole-device soak completed after at least `13:38:10`.
- CPU workload remained bounded at approximately `0.5-5.3%`; the main task
  watchdog subscription stayed `ESP_OK`; main stack HWM stayed at least `1796`.
- Internal heap stayed stable around `232 KB` with largest free block `110592`.
- No reboot, panic, brownout, watchdog trip, assertion, heap corruption, or
  stack overflow occurred.
- DS18B20 produced one transient `85.0 C` power-on value and recovered safely
  after rediscovery. Install the documented external `4.7 kOhm` DQ-to-3.3 V
  pull-up before the next hardware cycle.
