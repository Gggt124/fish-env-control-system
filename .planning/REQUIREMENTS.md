# Requirements: Milestone v1.6 Modern Web UI Optimization

## v1 Requirements

### Architecture & Assets (ARCH-UI)
- [ ] **ARCH-UI-01**: Implement Single Page Application (SPA) layout approach to allow smooth transitions without full page reloads.
- [x] **ARCH-UI-02**: Enforce zero external dependencies by using system fonts and inline SVGs instead of external icon libraries or web fonts.
- [ ] **ARCH-UI-03**: Adopt a new HSL-based color scheme, soft box shadows, and card-based layout (Modern UI styling).

### Dashboard & Control (DASH-UI)
- [ ] **DASH-UI-01**: Redesign the Pump Control and Timer Settings to use the new card-based layout with clear informational grids.
- [ ] **DASH-UI-02**: Redesign the Cooling Control section to visually separate it from pump controls and display temperature and relay state clearly.

### System & Settings (SYS-UI)
- [ ] **SYS-UI-01**: Overhaul the Wi-Fi Setup page to include a numbered stepper, network scan list improvements, and connection loading states.
- [ ] **SYS-UI-02**: Redesign the System Status view to display chip information, memory usage progress bars, and uptime.
- [ ] **SYS-UI-03**: Redesign the Hardware/Install page with clear distinction between active and pending GPIO configurations.

### UX & Feedback (UX-UI)
- [ ] **UX-UI-01**: Integrate loading spinners on buttons (like Sync, Start/Stop, Connect) to indicate ongoing processes.
- [ ] **UX-UI-02**: Implement modal popups for confirmations and dialogs instead of relying on basic browser alerts or redirects.

## Future Requirements (Deferred)
- None

## Out of Scope
- Migrating the frontend to a framework (React, Vue, etc.) or introducing external CSS/JS libraries (must stay plain HTML/CSS/vanilla JS).
- Changing the underlying C firmware logic for pump, cooling, or Wi-Fi control.
- Altering the existing API endpoints unless absolutely necessary to support minimal JSON payloads.

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| ARCH-UI-01  | Phase 18 | Pending|
| ARCH-UI-02  | Phase 18 | Complete |
| ARCH-UI-03  | Phase 18 | Pending|
| DASH-UI-01  | Phase 19 | Pending|
| DASH-UI-02  | Phase 19 | Pending|
| SYS-UI-01   | Phase 21 | Pending|
| SYS-UI-02   | Phase 20 | Pending|
| SYS-UI-03   | Phase 20 | Pending|
| UX-UI-01    | Phase 21 | Pending|
| UX-UI-02    | Phase 21 | Pending|
