---
status: testing
phase: 10-owner-dashboard-hardware-install-ui-and-validation
source:
  - 10-01-SUMMARY.md
  - 10-02-SUMMARY.md
  - 10-03-SUMMARY.md
started: 2026-05-24T00:00:00.000Z
updated: 2026-05-25T00:00:00.000Z
---

## Current Test
<!-- OVERWRITE each test - shows where we are -->

number: 4
name: Cooling Config And Runtime Mode Controls Work
expected: |
  Saving threshold, hysteresis, boot auto-enable, and Test ON timeout updates the dashboard from the device. Auto and Force OFF change runtime mode. Test ON starts as a bounded runtime action and is not saved as a boot mode.
awaiting: user response

## Tests

### 1. Dashboard Loads As Daily Operation Surface
expected: After flashing and logging in, `/dashboard` opens without console-visible page breakage. It shows the pump operation area, cooling operation area, and links to Status, Wi-Fi, and Hardware/Install.
result: pass

### 2. Pump Runtime Status Is Owner-Readable
expected: The dashboard shows pump enabled/running state, float state, active timer, active relay, countdown, Relay 1 state, Relay 2 state, and Start/Stop controls. The countdown stays readable and does not resize the page awkwardly.
result: pass

### 3. Cooling Runtime Status Is Owner-Readable
expected: The dashboard shows temperature or `--` when invalid, cooling relay state, mode, sensor state, fault state, threshold, hysteresis, boot auto-enable, blocked reason, lockout countdown, and Test ON remaining time when applicable.
result: pass

### 4. Cooling Config And Runtime Mode Controls Work
expected: Saving threshold, hysteresis, boot auto-enable, and Test ON timeout updates the dashboard from the device. Auto and Force OFF change runtime mode. Test ON starts as a bounded runtime action and is not saved as a boot mode.
result: issue
reported: "เวลา บันทึก cooling ไม่ควร รี lockout time ใหม่"
severity: major

### 5. Hardware/Install Page Is Wiring-First
expected: `/hardware` opens after login and shows wiring summary first, active GPIO summary next, editable safe GPIO dropdowns, and technical pinout metadata after the primary install information.
result: [pending]

### 6. Hardware Map Save Uses Pending Reboot Guardrails
expected: GPIO fields are dropdowns only. Save is disabled until the reboot acknowledgement is checked. Saving a changed map shows pending GPIO values and a reboot-required message while active runtime GPIOs remain unchanged before reboot.
result: [pending]

### 7. Pending Hardware Map Applies Only After Reboot
expected: After rebooting with a saved pending map, the active GPIO summary reflects the new map and the reboot-required state clears. Runtime pump/cooling GPIO behavior follows the active map, not the old pending display.
result: [pending]

### 8. Pump Relay Hardware Behavior Matches Float State
expected: With safe relay polarity confirmed, Float ON selects Timer 1 / Relay 1 and Float OFF selects Timer 2 / Relay 2. Relay 1 and Relay 2 are never energized together, and Stop forces both pump relays OFF.
result: [pending]

### 9. Cooling Hardware Behavior Is Safe
expected: A missing or unreadable DS18B20 forces the cooling relay OFF and shows sensor fault. A valid sensor reading follows threshold/hysteresis behavior. Force OFF keeps the relay OFF, and Test ON is bounded and lockout-aware.
result: [pending]

### 10. Wi-Fi And Status Pages Still Work
expected: `/status` still reports system/Wi-Fi/device state. `/wifi` still scans networks, can connect STA with saved credentials, and AP fallback remains available if STA fails.
result: [pending]

## Summary

total: 10
passed: 3
issues: 1
pending: 6
skipped: 0
blocked: 0

## Gaps

- truth: "Saving cooling configuration updates settings without restarting the current lockout countdown."
  status: failed
  reason: "User reported: เวลา บันทึก cooling ไม่ควร รี lockout time ใหม่"
  severity: major
  test: 4
  root_cause: "/api/cooling/config reapplied settings with cooling_control_init(), which reset runtime state and called restart_lockout_locked() even when only ordinary cooling settings were saved."
  artifacts: []
  missing:
    - "Add an in-place cooling_control_apply_config() path for ordinary cooling setting saves."
    - "Use the in-place apply path from POST /api/cooling/config before falling back to full init."
    - "Preserve the existing lockout deadline, only capping it when the new min-off setting is shorter."
  debug_session: ""
