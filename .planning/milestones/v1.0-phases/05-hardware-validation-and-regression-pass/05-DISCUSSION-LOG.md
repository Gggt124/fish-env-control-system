# Phase 5: Hardware Validation And Regression Pass - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md - this log preserves the alternatives considered.

**Date:** 2026-05-20
**Phase:** 5-Hardware Validation And Regression Pass
**Areas discussed:** Hardware wiring, hardware validation results, Wi-Fi and web regression

---

## Hardware Wiring

| Option | Description | Selected |
|--------|-------------|----------|
| GPIO32 float / GPIO26 relay | Use the current firmware defaults from `components/app_config/app_config.h`. GPIO32 is float input; GPIO26 is relay output. | x |
| Change GPIO pins before test | Defer validation until source defaults are changed. | |

**User's choice:** Test with current firmware defaults.
**Notes:** The user asked which GPIOs are currently used before testing. Source check confirmed GPIO32 for float input and GPIO26 for relay output, both active-low by default.

---

## Relay Voltage And Module Use

| Option | Description | Selected |
|--------|-------------|----------|
| Use relay module input from GPIO26 | Treat ESP32 GPIO26 as a 3.3V control signal into a relay module, not as relay coil power. | x |
| Drive bare relay directly | Not acceptable for ESP32 GPIO safety. Requires driver circuitry if not using a module. | |

**User's choice:** User wired the relay and reported it works.
**Notes:** Guidance given: GPIO26 is 3.3V logic only; many relay modules use 5V VCC with common GND and active-low IN.

---

## Hardware Validation Results

| Option | Description | Selected |
|--------|-------------|----------|
| Manual hardware passed | Float switching, relay phase, Stop, and reboot persistence all work on device. | x |
| Hardware issue found | Document failure and route to fix/debug before closing validation. | |

**User's choice:** `1234 ผ่านหมด`.
**Notes:** The user confirmed Float OFF -> Timer 1, Float ON -> Timer 2, relay follows ON/OFF and Stop forces inactive, and reboot persistence all passed.

---

## Wi-Fi And Web Regression

| Option | Description | Selected |
|--------|-------------|----------|
| Regression passed | SoftAP, login, dashboard, status, Wi-Fi page, Wi-Fi scan/connect, and pump UI remain reachable. | x |
| Regression issue found | Document failure and route to fix/debug before closing validation. | |

**User's choice:** `ผ่าน`.
**Notes:** The user confirmed the remaining Wi-Fi/web regression check passed after the hardware validation checks.

---

## the agent's Discretion

- Planner/verifier may choose exact checklist and verification note formatting.
- Planner/verifier should keep the phase validation-only and avoid adding new capabilities.

## Deferred Ideas

None.
