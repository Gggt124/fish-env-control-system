# Roadmap: Fish Pump Relay Timer Control

**Created:** 2026-05-18  
**Mode:** Vertical MVP  
**Requirements:** 42 v1 requirements mapped  

## Overview

This roadmap extends the existing Wi-Fi/web dashboard foundation into a real ESP32 pump relay controller. Phases are ordered so each step preserves a buildable firmware state and keeps hardware safety visible.

## Phases

### Phase 1: Hardware-Safe Pump Control Core

**Goal:** Add the pump-control component with safe GPIO defaults, relay inactive initialization, timer state machine, and build validation.  
**Mode:** mvp  
**UI hint:** no

**Requirements:** HW-01, HW-02, HW-03, HW-04, HW-05, HW-06, TIME-05, TIME-06, TIME-07, RUN-03, RUN-06, RUN-07, RUN-08, RUN-09, RUN-10, RUN-11, VAL-01

**Success Criteria**:
1. Firmware builds with a new pump-control component included.
2. GPIO32 is configured as active-low float input with pull-up.
3. GPIO26 is configured as relay output and starts inactive.
4. Timer selection and ON/OFF phase transitions match the flow-document v6 logic.
5. Stop or invalid configuration forces relay inactive.

### Phase 2: Pump Settings Persistence And Boot Behavior

**Goal:** Persist timer settings, relay polarity, and auto-start behavior in NVS, with defaults matching the user-approved timing.  
**Mode:** mvp  
**UI hint:** no

**Requirements:** TIME-08, RUN-04, RUN-05

**Success Criteria**:
1. Timer and control settings load from NVS with defaults when missing.
2. Timer 1 defaults to ON 20s / OFF 1:00.
3. Timer 2 defaults to ON 10s / OFF 3:00.
4. Auto-start defaults disabled but can be persisted as enabled.
5. Reboot behavior follows the saved auto-start value.

### Phase 3: Authenticated Pump Control API

**Goal:** Expose authenticated JSON APIs for pump config, pump status, start, and stop while preserving existing auth and same-origin patterns.  
**Mode:** mvp  
**UI hint:** no

**Requirements:** RUN-01, RUN-02, API-01, API-02, API-03, API-04, API-05, API-06

**Success Criteria**:
1. Authenticated clients can read pump configuration.
2. Authenticated clients can save validated pump configuration.
3. Authenticated clients can start and stop pump control.
4. Status API reports running state, float state, active timer, phase, countdown, relay state, and auto-start.
5. Unauthenticated pump API access is rejected consistently with existing protected APIs.

### Phase 4: Web Pump Control UI

**Goal:** Add the local web UI for timer setup, auto-start, start/stop, and live pump status without external dependencies.  
**Mode:** mvp  
**UI hint:** yes

**Requirements:** TIME-01, TIME-02, TIME-03, TIME-04, UI-01, UI-02, UI-03, UI-04, UI-05, UI-06, UI-07

**Success Criteria**:
1. User can edit Timer 1 and Timer 2 ON/OFF durations in minutes and seconds.
2. User can save timer settings and auto-start preference from the UI.
3. User can start and stop pump control from the UI.
4. UI displays active timer, phase, countdown, binary float state, relay state, and running state.
5. UI works through the existing local web server without CDN dependencies.

### Phase 5: Hardware Validation And Regression Pass

**Goal:** Validate the complete controller on ESP32 hardware and confirm existing Wi-Fi/setup behavior still works.  
**Mode:** mvp  
**UI hint:** no

**Requirements:** VAL-02, VAL-03, VAL-04, VAL-05

**Success Criteria**:
1. Manual hardware test confirms float OFF selects Timer 1 and float ON selects Timer 2.
2. Manual hardware test confirms relay follows ON/OFF phase and Stop forces inactive.
3. Reboot test confirms persisted timer and auto-start behavior.
4. Wi-Fi setup, SoftAP fallback, login, dashboard/status pages, and pump UI are reachable after pump control is added.
5. Any hardware findings are documented in project docs or phase verification notes.

## Coverage

| Phase | Requirement Count |
|-------|-------------------|
| Phase 1 | 17 |
| Phase 2 | 3 |
| Phase 3 | 8 |
| Phase 4 | 11 |
| Phase 5 | 4 |

All 42 v1 requirements are mapped to exactly one phase.

---
*Roadmap created: 2026-05-18*
