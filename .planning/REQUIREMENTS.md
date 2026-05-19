# Requirements: Fish Pump Relay Timer Control

**Defined:** 2026-05-18  
**Core Value:** The pump must switch reliably between Timer 1 and Timer 2 based on the float switch and drive the relay safely according to the selected timer's ON/OFF cycle.

## v1 Requirements

### Hardware Configuration

- [x] **HW-01**: Firmware uses ESP32 DevKit V1 30-pin / classic ESP32 as the target board.
- [x] **HW-02**: Firmware reads one binary float switch input with internal pull-up enabled and switch-to-GND treated as active.
- [x] **HW-03**: Firmware uses GPIO32 as the default float switch input unless changed in source configuration.
- [x] **HW-04**: Firmware drives one relay output GPIO and initializes it inactive before pump control starts.
- [x] **HW-05**: Firmware uses GPIO26 as the default relay output unless changed in source configuration.
- [x] **HW-06**: Firmware supports configurable relay polarity so active-high and active-low relay modules can be handled safely.

### Timer Configuration

- [ ] **TIME-01**: User can configure Timer 1 ON duration in minutes and seconds from the web UI.
- [ ] **TIME-02**: User can configure Timer 1 OFF duration in minutes and seconds from the web UI.
- [ ] **TIME-03**: User can configure Timer 2 ON duration in minutes and seconds from the web UI.
- [ ] **TIME-04**: User can configure Timer 2 OFF duration in minutes and seconds from the web UI.
- [x] **TIME-05**: Default Timer 1 duration is ON 20 seconds and OFF 1 minute.
- [x] **TIME-06**: Default Timer 2 duration is ON 10 seconds and OFF 3 minutes.
- [x] **TIME-07**: Firmware rejects invalid timer settings that could cause zero-duration or rapid relay chatter.
- [ ] **TIME-08**: Timer settings persist in NVS and survive reboot.

### Pump Runtime

- [ ] **RUN-01**: User can start pump control from the web UI.
- [ ] **RUN-02**: User can stop pump control from the web UI.
- [x] **RUN-03**: Stop immediately de-energizes the relay and leaves the controller in a stopped state.
- [ ] **RUN-04**: Pump control auto-starts on boot by default after saved configuration loads.
- [ ] **RUN-05**: User can disable or enable auto-start from the web UI, and the setting persists in NVS.
- [x] **RUN-06**: Float switch inactive/OFF selects Timer 1.
- [x] **RUN-07**: Float switch active/ON selects Timer 2.
- [x] **RUN-08**: When the selected timer changes because float state changes, the newly selected timer starts at its ON phase.
- [x] **RUN-09**: Active timer alternates between ON and OFF phases using its configured durations.
- [x] **RUN-10**: Relay is energized only during the active timer's ON phase.
- [x] **RUN-11**: Relay is de-energized during OFF phase, stopped state, invalid config, and initialization.

### Web API

- [ ] **API-01**: Authenticated API returns pump configuration including timer durations, auto-start setting, relay polarity, and configured GPIO defaults.
- [ ] **API-02**: Authenticated API saves pump configuration after validating timer durations and polarity.
- [ ] **API-03**: Authenticated API starts pump control.
- [ ] **API-04**: Authenticated API stops pump control.
- [ ] **API-05**: Authenticated API returns pump status including running state, float state, active timer, current phase, countdown, relay state, and auto-start state.
- [ ] **API-06**: Pump APIs follow the existing session auth and same-origin POST protection patterns.

### Web UI

- [ ] **UI-01**: Dashboard or pump page shows Timer 1 and Timer 2 ON/OFF settings.
- [ ] **UI-02**: UI lets the user save timer settings and auto-start preference.
- [ ] **UI-03**: UI provides Start and Stop controls.
- [ ] **UI-04**: UI displays active timer, current phase, and countdown as MM:SS.
- [ ] **UI-05**: UI displays float switch state as binary ON/OFF or high/low, not as real 0-100% water level.
- [ ] **UI-06**: UI displays relay state as energized/de-energized or open/closed.
- [ ] **UI-07**: UI remains usable without internet and uses no CDN dependencies.

### Validation

- [x] **VAL-01**: `idf.py build` succeeds after pump control changes.
- [ ] **VAL-02**: Manual hardware test confirms float OFF selects Timer 1 and float ON selects Timer 2.
- [ ] **VAL-03**: Manual hardware test confirms relay output follows ON/OFF phase and stop forces relay inactive.
- [ ] **VAL-04**: Manual reboot test confirms timer settings and auto-start preference persist.
- [ ] **VAL-05**: Manual access test confirms Wi-Fi setup, SoftAP fallback, login, and status pages still work while pump control is present.

## v2 Requirements

### Hardware Configuration

- **HW2-01**: User can configure float GPIO from the web UI.
- **HW2-02**: User can configure relay GPIO from the web UI.
- **HW2-03**: User can configure float debounce duration from the web UI.

### Monitoring

- **MON-01**: User can view a recent event log of float changes, timer phase changes, relay changes, and start/stop events.
- **MON-02**: User can export current pump configuration and status for troubleshooting.

### Safety

- **SAFE-01**: User can run an explicit relay test mode with timeout and warning.
- **SAFE-02**: Firmware can enforce maximum continuous relay ON duration.

## Out of Scope

Explicitly excluded. Documented to prevent scope creep.

| Feature | Reason |
|---------|--------|
| Continuous water-level measurement | Current hardware is a binary float switch, not an analog level sensor |
| Multiple float switches | User design specifies one float switch |
| Sensor telemetry charts | Not needed for the first real relay-control milestone |
| OTA updates | Existing project intentionally excludes OTA in this phase |
| MQTT/cloud integration | Device is local-first and must work in AP mode without internet |
| HTTPS/multi-user production auth | Existing security model is local prototype only |
| SPIFFS/LittleFS storage | Existing static UI uses embedded files and NVS is enough for settings |

## Traceability

Which phases cover which requirements. Updated during roadmap creation.

| Requirement | Phase | Status |
|-------------|-------|--------|
| HW-01 | Phase 1 | Complete |
| HW-02 | Phase 1 | Complete |
| HW-03 | Phase 1 | Complete |
| HW-04 | Phase 1 | Complete |
| HW-05 | Phase 1 | Complete |
| HW-06 | Phase 1 | Complete |
| TIME-01 | Phase 4 | Pending |
| TIME-02 | Phase 4 | Pending |
| TIME-03 | Phase 4 | Pending |
| TIME-04 | Phase 4 | Pending |
| TIME-05 | Phase 1 | Complete |
| TIME-06 | Phase 1 | Complete |
| TIME-07 | Phase 1 | Complete |
| TIME-08 | Phase 2 | Pending |
| RUN-01 | Phase 3 | Pending |
| RUN-02 | Phase 3 | Pending |
| RUN-03 | Phase 1 | Complete |
| RUN-04 | Phase 2 | Pending |
| RUN-05 | Phase 2 | Pending |
| RUN-06 | Phase 1 | Complete |
| RUN-07 | Phase 1 | Complete |
| RUN-08 | Phase 1 | Complete |
| RUN-09 | Phase 1 | Complete |
| RUN-10 | Phase 1 | Complete |
| RUN-11 | Phase 1 | Complete |
| API-01 | Phase 3 | Pending |
| API-02 | Phase 3 | Pending |
| API-03 | Phase 3 | Pending |
| API-04 | Phase 3 | Pending |
| API-05 | Phase 3 | Pending |
| API-06 | Phase 3 | Pending |
| UI-01 | Phase 4 | Pending |
| UI-02 | Phase 4 | Pending |
| UI-03 | Phase 4 | Pending |
| UI-04 | Phase 4 | Pending |
| UI-05 | Phase 4 | Pending |
| UI-06 | Phase 4 | Pending |
| UI-07 | Phase 4 | Pending |
| VAL-01 | Phase 1 | Complete |
| VAL-02 | Phase 5 | Pending |
| VAL-03 | Phase 5 | Pending |
| VAL-04 | Phase 5 | Pending |
| VAL-05 | Phase 5 | Pending |

**Coverage:**
- v1 requirements: 42 total
- Mapped to phases: 42
- Unmapped: 0

---
*Requirements defined: 2026-05-18*
*Last updated: 2026-05-18 after roadmap creation*
