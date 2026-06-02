# Requirements: Fish Pump Relay Timer Control

**Defined:** 2026-06-02
**Milestone:** v1.2 Owner UI Polish And Hardware Readiness
**Core Value:** The pump must switch reliably between Timer 1 and Timer 2 based on the float switch and drive the relay safely according to the selected timer's ON/OFF cycle.

## v1.2 Requirements

### Owner Dashboard

- [ ] **UI-08**: Owner can identify the main pump state, selected timer and relay, current phase, countdown, and float switch state clearly from the dashboard.
- [ ] **UI-09**: Owner can identify temperature, cooling mode, cooling relay state, and sensor fault state as a separate operational channel from the pump relays.
- [ ] **UI-10**: Owner can distinguish daily operation controls from configuration settings and secondary diagnostics without scanning equal-weight panels.

### App Shell And Login

- [ ] **UI-14**: User can understand login purpose, credential entry, loading, and authentication error recovery clearly on desktop and narrow mobile screens.
- [ ] **UI-15**: Authenticated user can navigate dashboard, Hardware/Install, Wi-Fi, Status, and logout consistently from the app shell on desktop and narrow mobile screens.

### Hardware Install

- [ ] **HW-06**: Installer can see explicit guidance to add an external `4.7 kOhm` pull-up resistor between DS18B20 DQ and `3.3 V` before the next hardware cycle.
- [ ] **UI-11**: Installer can distinguish active GPIO values, pending GPIO values, and reboot-required state clearly on the Hardware/Install page.
- [ ] **UI-12**: Installer can use the Hardware/Install flow on a narrow mobile screen while retaining safe dropdown options and the reboot/wiring confirmation checkbox.

### Setup States

- [ ] **SETUP-01**: User can identify loading, error, empty, disconnected, and connected states during Wi-Fi scan and connect flows.
- [ ] **SETUP-02**: User can identify loading, success, error, disabled, and pending-reboot states on dashboard and Hardware/Install operations.

### System Status

- [ ] **UI-16**: User can scan system, memory, uptime, Wi-Fi station, access point, and service diagnostics clearly on desktop and narrow mobile screens.

### Accessibility

- [ ] **A11Y-01**: User can use login, app shell, dashboard, Hardware/Install, Wi-Fi, and Status pages on a narrow mobile viewport without losing access to primary content or actions.
- [ ] **A11Y-02**: Keyboard user can navigate interactive controls with visible focus treatment and understandable labels or instructions.
- [ ] **A11Y-03**: Assistive-technology user can recognize dynamic status and error messages programmatically.

### Design System

- [ ] **UI-13**: User experiences a consistent embedded product UI shaped by the existing `DESIGN.md` system and reviewed with `frontend-ui-ux`, `ui-ux-pro-max`, and `impeccable` guidance adapted to offline ESP32 constraints.

### Verification

- [ ] **VER-01**: Maintainer can review screenshots of affected UI pages at desktop and narrow mobile viewports before milestone close.
- [ ] **VER-02**: Maintainer can build valid ESP-IDF firmware after embedded asset changes without introducing unsuitable frontend dependencies or unsafe ESP32 footprint growth.
- [ ] **VER-03**: Maintainer can review the UI with an `impeccable` critique/audit/polish pass and the applicable `ui-ux-pro-max` checklist before milestone close.
- [ ] **VER-04**: Maintainer can review representative loading, error, empty, disabled, and pending-reboot screenshots before milestone close.
- [ ] **REG-01**: Owner can continue using the stable v1.1 relay, timer, cooling, Wi-Fi, and APSTA flows without behavioral regression.

## Future Requirements

### Monitoring And Troubleshooting

- **MON-01**: User can view an event log of float changes, timer phase changes, relay changes, cooling changes, and start/stop events.
- **MON-02**: User can export pump configuration, cooling configuration, hardware mapping, and status for troubleshooting.

### Optimization

- **OPT-01**: Firmware binary size and flash usage are optimized if embedded UI polish creates partition pressure.

## Out of Scope

| Feature | Reason |
|---------|--------|
| Relay, timer, or cooling state-machine rewrite | v1.1 runtime behavior is stable and should change only if a verified UI state bug requires a targeted fix. |
| Wi-Fi/APSTA behavior rewrite | The passed `13:38:10` soak behavior is a protected baseline. |
| Eight deferred scanner rows as firmware scope | These rows are bookkeeping debt only, not unfinished firmware work. |
| Continuous 0-100% water-level measurement | The real input is a binary float switch only. |
| Multiple float switches or analog level sensors | Current hardware contract uses one binary float input. |
| Cloud/MQTT/remote telemetry | Local operation remains the priority and SoftAP mode has no internet dependency. |
| OTA implementation | Dual OTA partitions exist, but update flow is not needed for this milestone. |
| Event log, export, charts, or historical analytics | Deferred until after current-state operator clarity is validated. |
| Frontend framework migration, CDN, remote fonts, or remote icon assets | Embedded UI must remain offline-capable and ESP32-safe. |

## Traceability

Traceability is populated during roadmap creation.

| Requirement | Phase | Status |
|-------------|-------|--------|
| UI-08 | Phase 12 | Pending |
| UI-09 | Phase 12 | Pending |
| UI-10 | Phase 12 | Pending |
| UI-14 | Phase 12 | Pending |
| UI-15 | Phase 12 | Pending |
| HW-06 | Phase 13 | Pending |
| UI-11 | Phase 13 | Pending |
| UI-12 | Phase 13 | Pending |
| SETUP-01 | Phase 13 | Pending |
| SETUP-02 | Phase 13 | Pending |
| UI-16 | Phase 13 | Pending |
| A11Y-01 | Phase 14 | Pending |
| A11Y-02 | Phase 11 | Pending |
| A11Y-03 | Phase 11 | Pending |
| UI-13 | Phase 11 | Pending |
| VER-01 | Phase 14 | Pending |
| VER-02 | Phase 14 | Pending |
| VER-03 | Phase 14 | Pending |
| VER-04 | Phase 14 | Pending |
| REG-01 | Phase 14 | Pending |

**Coverage:**
- v1.2 requirements: 20 total
- Mapped to phases: 20
- Unmapped: 0

---
*Requirements defined: 2026-06-02*
*Last updated: 2026-06-02 after v1.2 roadmap creation*
