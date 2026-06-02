---
phase: 08-ds18b20-cooling-runtime
status: passed
verified: 2026-06-02
requirements:
  - COOL-01
  - COOL-02
  - COOL-03
  - COOL-04
  - COOL-05
  - COOL-06
  - COOL-07
---

# Phase 08 Verification: DS18B20 Cooling Runtime

## Result

status: passed

Phase 8 achieved its goal: the firmware reads a powered-mode DS18B20, controls a
dedicated cooling relay independently from pump relays, persists safe cooling
settings, exposes runtime status, fails safe when sensor data is unavailable,
and enforces compressor-aware mode behavior.

## Evidence

| Requirement | Evidence | Result |
|---|---|---|
| COOL-01 separate cooling channel | `components/cooling_control` owns DS18B20 polling and cooling relay GPIO state independently from `pump_control`. | PASS |
| COOL-02 threshold and hysteresis | Cooling config persists `threshold_x10` and `hyst_x10`; runtime compares temperature against threshold and threshold minus hysteresis. | PASS |
| COOL-03 relay behavior | `update_control_locked()` energizes only when Auto demand is valid and de-energizes below the hysteresis boundary. | PASS |
| COOL-04 sensor fail-safe | Invalid/unavailable reads clear trusted temperature state and force automatic cooling relay OFF. Phase 10 hardware UAT Test 9 passed. | PASS |
| COOL-05 safe boot default | Cooling auto-enable defaults OFF and persists through `cool_cfg`. | PASS |
| COOL-06 runtime modes | Auto, Force OFF, and bounded runtime-only Test ON are implemented; Phase 10 hardware UAT Test 4 and Test 9 passed. | PASS |
| COOL-07 compressor protection | Minimum off-time applies at boot/reinit and after ON-to-OFF transitions, including Test ON lockout. | PASS |
| Live sensor soak | `logs/soak-wifi-scan-final-20260601-223025.log` captured more than 13 hours of valid cooling diagnostics with no persistent cooling fault. | PASS |
| Transient recovery | One DS18B20 `85.0 C` power-on value was rejected, rediscovered in about 0.1 seconds, and followed by valid readings without energizing the cooling relay from invalid data. | PASS |
| Firmware build | `.\scripts\build.ps1` passed on 2026-06-02 and generated `build\fish_pump_relay_timer_control.bin`. | PASS |

## Automated Checks

- `rg "COOLING_CONTROL_FAILED_READS_TO_FAULT|COOLING_CONTROL_SUCCESSES_TO_CLEAR_FAULT|temperature_in_supported_range|record_read_failure_locked|record_read_success_locked" components/cooling_control`
- `rg "COOLING_CONTROL_MODE_TEST_ON|compressor|min_off|lockout" components/cooling_control main`
- `rg "/api/cooling/status|/api/cooling/config|/api/cooling/mode" main/web_server.c main/static/app.js`
- `.\scripts\build.ps1`

## Human Verification

- Phase 10 hardware UAT Test 4 passed: cooling config and runtime mode controls work.
- Phase 10 hardware UAT Test 9 passed: missing/unreadable sensor behavior, valid
  sensor behavior, Force OFF, Test ON, and lockout-aware operation are safe.

## Hardware Follow-Up

The test setup produced one recoverable DS18B20 power-on value while the external
pull-up resistor was not yet installed. Add a 4.7 kOhm pull-up from DQ to 3.3 V
before the next hardware validation cycle. This is installation follow-up, not a
firmware release blocker.

## Gaps

None.
