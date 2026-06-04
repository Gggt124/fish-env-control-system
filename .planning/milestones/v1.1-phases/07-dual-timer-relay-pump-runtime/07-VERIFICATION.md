---
phase: 07-dual-timer-relay-pump-runtime
status: passed
verified: 2026-05-23
build: passed
---

# Phase 7 Verification

## Result

Passed for source/build validation. Hardware behavior still requires manual flashing and relay/float testing.

## Checks Run

- `rg "relay1_gpio|relay2_gpio|relay1_energized|relay2_energized|active_relay|timer1_start|timer2_start|fault" components/pump_control/pump_control.h components/pump_control/CMakeLists.txt`
- `rg "Float ON|PUMP_CONTROL_TIMER_1|PUMP_CONTROL_TIMER_2|PUMP_CONTROL_PHASE_OFF|start_phase|relay1|relay2" components/pump_control/pump_control.c`
- `rg "gpio_set_level|force_both_relays_inactive|mark_fault|pump_control_stop" components/pump_control/pump_control.c`
- `rg "pump_relay1_gpio|pump_relay2_gpio|relay1_active_low|relay2_active_low|timer1_start_phase|timer2_start_phase" main/app_main.c main/web_server.c`
- `rg "active_relay|relay1_energized|relay2_energized|relay_energized|fault" main/web_server.c`
- `rg "timer1_start_phase|timer2_start_phase" main/static/dashboard.html main/static/app.js main/web_server.c`
- `rg "Float ON.*Timer 1|Float OFF.*Timer 2|Relay 1|Relay 2|พร้อมเริ่ม" main/static/dashboard.html main/static/app.js`
- `rg "relay1_energized|relay2_energized|active_relay" main/static/dashboard.html main/static/app.js main/web_server.c`
- `rg "http://|https://|cdn|Tailwind|Google Fonts|Material Symbols" main/static` returned no matches.
- `.\scripts\build.ps1`
- `Test-Path build\fish_pump_relay_timer_control.bin` returned `True`.

## Requirement Coverage

- **PUMP-01:** Runtime uses binary float switch state and debounces confirmed state changes.
- **PUMP-02:** Float ON selects Timer 1 / Relay 1.
- **PUMP-03:** Float OFF selects Timer 2 / Relay 2.
- **PUMP-04:** Timer 1 and Timer 2 start phase are configurable and persisted.
- **PUMP-05:** Confirmed float transitions force both relays OFF before starting the selected channel from start phase.
- **PUMP-06:** Stopped state keeps relays OFF while reporting float and ready channel preview.

## Manual Hardware Validation Still Needed

- Flash firmware to ESP32 DevKit V1.
- Wire float switch to GPIO32 as active-low switch to GND.
- Wire pump Relay 1 to GPIO26 and pump Relay 2 to GPIO27 with the configured active-low polarity.
- Confirm Relay 1 and Relay 2 never energize at the same time.
- Confirm dashboard shows float/ready state before pressing Start.
