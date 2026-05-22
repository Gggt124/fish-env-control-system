# Roadmap: Fish Pump Relay Timer Control

**Created:** 2026-05-18  
**Mode:** Vertical MVP
**Current Milestone:** v1.1 Dual Relay Cooling And Install UI

## Milestones

- ✅ **v1.0 MVP** — Phases 1-5 (shipped 2026-05-20) — `.planning/milestones/v1.0-ROADMAP.md`
- 🔄 **v1.1 Dual Relay Cooling And Install UI** — Phases 6-10 (planned 2026-05-22)

## Phases

<details>
<summary>✅ v1.0 MVP (Phases 1-5) — SHIPPED 2026-05-20</summary>

- [x] Phase 1: Hardware-Safe Pump Control Core (3/3 plans) — completed 2026-05-19
- [x] Phase 2: Pump Settings Persistence And Boot Behavior (3/3 plans) — completed 2026-05-19
- [x] Phase 3: Authenticated Pump Control API (3/3 plans) — completed 2026-05-20
- [x] Phase 4: Web Pump Control UI (3/3 plans) — completed 2026-05-20
- [x] Phase 5: Hardware Validation And Regression Pass (1/1 plan) — completed 2026-05-20

</details>

### v1.1 Dual Relay Cooling And Install UI

#### Phase 6: Hardware Contract And Persistent Map Foundation

**Goal:** Define and implement the safe hardware contract before changing runtime control behavior.

**Requirements:** HW-01, HW-02, HW-03, HW-04, HW-05

**Success criteria:**
1. Firmware exposes role-based safe GPIO enums for float input, Relay 1, Relay 2, DS18B20 data, and cooling relay.
2. Conservative ESP32 DevKit V1 defaults are documented and available from firmware/API constants.
3. Relay polarity can be represented independently for all three relay outputs.
4. DS18B20 wiring assumptions and compressor protection defaults are documented before cooling implementation.
5. NVS schema supports active GPIO map, pending GPIO map, relay polarity, cooling settings, and timer start phases.

**Plans:** 3 plans

Plans:

**Wave 1**

- [ ] 06-01: Hardware map component, safe GPIO defaults, and DS18B20 wiring contract

**Wave 2 (blocked on Wave 1 completion)**
- [ ] 06-02: Active/pending NVS hardware map, independent polarity, cooling defaults, and timer start-phase schema

**Wave 3 (blocked on Waves 1-2 completion)**
- [ ] 06-03: Boot/config/status integration, compatibility handoff, and ESP-IDF build validation

#### Phase 7: Dual Timer Relay Pump Runtime

**Goal:** Replace the single relay-selected-timer model with two timer/relay channels controlled by binary float state.

**Requirements:** PUMP-01, PUMP-02, PUMP-03, PUMP-04, PUMP-05, PUMP-06

**Success criteria:**
1. Float ON selects Timer 1/Relay 1 and forces Relay 2 OFF.
2. Float OFF selects Timer 2/Relay 2 and forces Relay 1 OFF.
3. Float transitions immediately stop the previous channel and restart the selected timer from its configured start phase.
4. Pump disabled state keeps both pump relays OFF while still reporting float state.
5. ESP-IDF build passes with the new pump channel model.

#### Phase 8: DS18B20 Cooling Runtime

**Goal:** Add a separate temperature cooling channel with safe fault behavior and compressor-aware relay control.

**Requirements:** COOL-01, COOL-02, COOL-03, COOL-04, COOL-05, COOL-06, COOL-07

**Success criteria:**
1. DS18B20 readings drive cooling relay state through threshold and hysteresis rules.
2. Missing, disconnected, or unreadable sensor state forces cooling relay OFF and is visible through status.
3. Cooling enable and auto-enable settings persist with safe OFF defaults.
4. Auto, Force OFF, and Test ON modes behave as specified, including test timeout and reboot behavior.
5. Minimum off-time compressor protection is enabled by default and applies to automatic and test operation.

#### Phase 9: Authenticated Hardware And Cooling APIs

**Goal:** Expose the new pump, cooling, and hardware-map contracts through authenticated local APIs.

**Requirements:** UI-03, UI-04, UI-05, UI-06, UI-07

**Success criteria:**
1. Authenticated APIs return active GPIO values, pending GPIO values, and reboot-required status.
2. GPIO update APIs accept only safe enum values and require explicit confirmation semantics.
3. Cooling configuration APIs validate threshold, hysteresis, auto-enable, mode, and test timeout inputs.
4. Pump configuration APIs support two relay channels and independent timer start phases.
5. Same-origin POST protection and existing session auth remain intact for new mutation routes.

#### Phase 10: Owner Dashboard, Hardware Install UI, And Validation

**Goal:** Deliver the owner-facing operation surface and installer-friendly hardware flow, then validate the full v1.1 behavior.

**Requirements:** UI-01, UI-02

**Success criteria:**
1. Dashboard shows pump enabled state, float state, active timer/relay, countdown, temperature, cooling relay state, threshold, auto-enable setting, and sensor fault state.
2. Hardware/Install page presents wiring visualization first, GPIO summary next, and technical pinout as a secondary view.
3. GPIO changes are saved as pending values with reboot-required messaging and confirmation guardrails.
4. UI works without CDN or internet access in SoftAP mode.
5. Build and manual hardware validation cover dual pump relays, DS18B20 fault handling, cooling relay behavior, reboot persistence, and Wi-Fi/setup regression.

## Progress

| Phase | Milestone | Plans Complete | Status | Completed |
|-------|-----------|---------------|--------|-----------|
| 1. Hardware-Safe Pump Control Core | v1.0 | 3/3 | Complete | 2026-05-19 |
| 2. Pump Settings Persistence And Boot Behavior | v1.0 | 3/3 | Complete | 2026-05-19 |
| 3. Authenticated Pump Control API | v1.0 | 3/3 | Complete | 2026-05-20 |
| 4. Web Pump Control UI | v1.0 | 3/3 | Complete | 2026-05-20 |
| 5. Hardware Validation And Regression Pass | v1.0 | 1/1 | Complete | 2026-05-20 |
| 6. Hardware Contract And Persistent Map Foundation | v1.1 | 0/3 | Ready to execute | — |
| 7. Dual Timer Relay Pump Runtime | v1.1 | 0/0 | Pending | — |
| 8. DS18B20 Cooling Runtime | v1.1 | 0/0 | Pending | — |
| 9. Authenticated Hardware And Cooling APIs | v1.1 | 0/0 | Pending | — |
| 10. Owner Dashboard, Hardware Install UI, And Validation | v1.1 | 0/0 | Pending | — |

---
*Roadmap created: 2026-05-18*
*Last updated: 2026-05-22 after v1.1 milestone approval*
