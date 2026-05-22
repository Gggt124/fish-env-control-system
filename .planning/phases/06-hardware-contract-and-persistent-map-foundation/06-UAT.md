---
status: complete
phase: 06-hardware-contract-and-persistent-map-foundation
source:
  - 06-01-SUMMARY.md
  - 06-02-SUMMARY.md
  - 06-03-SUMMARY.md
started: 2026-05-22T14:19:27.2854308+07:00
updated: 2026-05-22T17:07:46.8667162+07:00
---

## Current Test

[testing complete]

## Tests

### 1. Hardware Contract Documentation
expected: Opening the hardware documentation shows the ESP32 DevKit V1 wiring contract with safe defaults for float GPIO32, pump Relay 1 GPIO26, pump Relay 2 GPIO27, DS18B20 GPIO33, cooling relay GPIO25, relay polarity warnings, powered-mode DS18B20 wiring, and the external 4.7 kOhm pull-up requirement.
result: pass

### 2. Build Produces Firmware With Hardware Foundation
expected: Running the project build completes successfully and produces build/fish_pump_relay_timer_control.bin with the hardware_map component included.
result: pass

### 3. Boot Uses Safe Active Map Defaults
expected: On boot with no valid persisted hardware map, the firmware starts normally, logs the active hardware map/default status, and uses float GPIO32 plus pump Relay 1 GPIO26 for the existing single-relay runtime.
result: pass

### 4. Pump API Exposes Read-Only Hardware Fields
expected: The existing pump config/status API responses still work and include read-only hardware foundation fields such as float GPIO, Relay 1/Relay 2 GPIOs and polarity, timer start phases, DS18B20 GPIO, cooling relay GPIO, cooling polarity, cooling mode, and pending/reboot-required status.
result: pass

### 5. Invalid Persisted Settings Fail Safe
expected: If persisted pump settings or the active hardware map are invalid, the firmware suppresses pump auto-start for the current boot and falls back to safe defaults rather than energizing relay hardware.
result: pass

### 6. Phase 6 Runtime Boundary Is Preserved
expected: Relay 2, DS18B20, and cooling relay values are stored or reported only; Phase 6 does not drive those outputs or add dual-relay, sensor, or cooling runtime behavior.
result: pass

## Summary

total: 6
passed: 6
issues: 0
pending: 0
skipped: 0
blocked: 0

## Gaps

[none yet]
