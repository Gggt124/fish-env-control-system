# Requirements: Fish Pump Relay Timer Control

**Defined:** 2026-05-22
**Milestone:** v1.1 Dual Relay Cooling And Install UI
**Core Value:** The pump must switch reliably between Timer 1 and Timer 2 based on the float switch and drive the relay safely according to the selected timer's ON/OFF cycle.

## v1.1 Requirements

### Hardware Contract

- [x] **HW-01**: Owner can rely on firmware-defined safe GPIO options for float input, Relay 1, Relay 2, DS18B20 data, and cooling relay roles.
- [x] **HW-02**: Owner can use conservative default GPIO recommendations for ESP32 DevKit V1 30-pin without conflicting with flash, programming, input-only, or boot-strapping pins.
- [x] **HW-03**: Owner can configure relay polarity independently for pump Relay 1, pump Relay 2, and cooling relay.
- [x] **HW-04**: Installer can wire a DS18B20 using documented powered-mode assumptions, pull-up resistor expectations, voltage rail, and connector labeling.
- [x] **HW-05**: Firmware can store active GPIO map, pending GPIO map, relay polarity, cooling settings, and timer start phase values in a documented NVS schema.

### Pump Channels

- [ ] **PUMP-01**: User can run pump control with two separate timer/relay channels selected by binary float state.
- [ ] **PUMP-02**: User sees `Float ON` select Timer 1 and Relay 1, with Relay 2 forced OFF.
- [ ] **PUMP-03**: User sees `Float OFF` select Timer 2 and Relay 2, with Relay 1 forced OFF.
- [ ] **PUMP-04**: User can configure independent Timer 1 and Timer 2 start phases as `ON` or `OFF`, defaulting both timers to `ON`.
- [ ] **PUMP-05**: When float state changes, firmware stops the previous timer, forces its relay OFF immediately, and starts the newly selected timer from its configured start phase.
- [ ] **PUMP-06**: When pump control is disabled, Relay 1 and Relay 2 remain OFF while float status continues to be read and displayed.

### Cooling Control

- [ ] **COOL-01**: User can enable a DS18B20-based cooling channel that controls a dedicated cooling relay separate from pump relays.
- [ ] **COOL-02**: User can configure cooling threshold and hysteresis with defaults of 30 C threshold and 1 C hysteresis.
- [ ] **COOL-03**: Firmware turns the cooling relay ON at or above the threshold and OFF after temperature falls below threshold minus hysteresis.
- [ ] **COOL-04**: If the DS18B20 is missing, disconnected, or unreadable, firmware forces the cooling relay OFF and exposes sensor fault status.
- [ ] **COOL-05**: User can choose whether cooling auto-enables on boot, defaulting auto-enable to OFF.
- [ ] **COOL-06**: User can use cooling modes `Auto`, `Force OFF`, and `Test ON`, where `Test ON` defaults to 10 seconds, does not persist across reboot, and returns to the previous mode after timeout.
- [ ] **COOL-07**: Firmware enforces compressor-protection minimum off-time for cooling relay operation by default, including override/test behavior.

### Web UI And API

- [ ] **UI-01**: Owner dashboard remains focused on daily operation and shows pump enabled state, float state, active timer/relay, countdown, temperature, cooling relay state, threshold, auto-enable setting, and sensor fault state.
- [ ] **UI-02**: Owner-accessible Hardware/Install page shows wiring-oriented visualization, GPIO summary table, and a secondary technical pinout view.
- [ ] **UI-03**: User can change GPIO mapping only through safe dropdown enum options, not freeform numeric input.
- [ ] **UI-04**: User can save GPIO changes as pending persistent values that do not become active until reboot.
- [ ] **UI-05**: User sees active GPIO values, pending GPIO values, and reboot-required status when pending values differ from active values.
- [ ] **UI-06**: User must confirm a reboot/wiring safety checkbox before saving GPIO map changes.
- [ ] **UI-07**: Authenticated APIs expose pump channel, cooling, and hardware map state with active vs pending GPIO fields and reboot-required status.

## Future Requirements

### Monitoring And Troubleshooting

- **MON-01**: User can view an event log of float changes, timer phase changes, relay changes, cooling changes, and start/stop events.
- **MON-02**: User can export pump configuration, cooling configuration, hardware mapping, and status for troubleshooting.

### Optimization

- **OPT-01**: Firmware binary size and flash usage are optimized if v1.1 features create partition pressure.

## Out of Scope

| Feature | Reason |
|---------|--------|
| Continuous 0-100% water-level measurement | The real input is a binary float switch only. |
| Multiple float switches or analog level sensors | v1.1 keeps one binary float input and changes only the relay/channel behavior. |
| Cloud/MQTT/remote telemetry | Local operation remains the priority and SoftAP mode has no internet dependency. |
| OTA implementation | Dual OTA partitions exist, but update flow is not needed for this milestone. |
| Multi-user roles | Existing local admin session model remains sufficient for v1.1. |
| Charts or historical analytics | Deferred until after event logging/export is scoped. |
| Freeform GPIO entry | Unsafe for installers and conflicts with firmware-defined safe enum guardrails. |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| HW-01 | Phase 6 | Complete |
| HW-02 | Phase 6 | Complete |
| HW-03 | Phase 6 | Complete |
| HW-04 | Phase 6 | Complete |
| HW-05 | Phase 6 | Complete |
| PUMP-01 | Phase 7 | Pending |
| PUMP-02 | Phase 7 | Pending |
| PUMP-03 | Phase 7 | Pending |
| PUMP-04 | Phase 7 | Pending |
| PUMP-05 | Phase 7 | Pending |
| PUMP-06 | Phase 7 | Pending |
| COOL-01 | Phase 8 | Pending |
| COOL-02 | Phase 8 | Pending |
| COOL-03 | Phase 8 | Pending |
| COOL-04 | Phase 8 | Pending |
| COOL-05 | Phase 8 | Pending |
| COOL-06 | Phase 8 | Pending |
| COOL-07 | Phase 8 | Pending |
| UI-01 | Phase 10 | Pending |
| UI-02 | Phase 10 | Pending |
| UI-03 | Phase 9 | Pending |
| UI-04 | Phase 9 | Pending |
| UI-05 | Phase 9 | Pending |
| UI-06 | Phase 9 | Pending |
| UI-07 | Phase 9 | Pending |

**Coverage:**
- v1.1 requirements: 25 total
- Mapped to phases: 25
- Unmapped: 0

---
*Requirements defined: 2026-05-22*
*Last updated: 2026-05-22 after Phase 6 verification*
