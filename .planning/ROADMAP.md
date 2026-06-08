# Roadmap: Milestone v1.6 Modern Web UI Optimization

**4 phases** | **10 requirements mapped** | All covered âœ“

| # | Phase | Status | Goal | Requirements | Success Criteria |
|---|-------|--------|------|--------------|------------------|
| 18 | Modern App Shell & Architecture Foundation | Not Started | Establish SPA layout, CSS variables, and typography. | ARCH-UI-01, ARCH-UI-02, ARCH-UI-03 | 3 |
| 19 | Dashboard & Control Implementation | Not Started | Redesign Pump, Timer, and Cooling controls to card-based layout. | DASH-UI-01, DASH-UI-02 | 3 |
| 20 | System Status & Hardware/Install Pages | Not Started | Overhaul the status view and GPIO configuration interface. | SYS-UI-02, SYS-UI-03 | 3 |
| 21 | Wi-Fi Setup & UX Polish | Not Started | Overhaul Wi-Fi Setup stepper and integrate loading/modal feedback across the app. | SYS-UI-01, UX-UI-01, UX-UI-02 | 3 |

### Phase Details

**Phase 18: Modern App Shell & Architecture Foundation**
Goal: Establish SPA layout, CSS variables, and typography.
Requirements: ARCH-UI-01, ARCH-UI-02, ARCH-UI-03
Success criteria:
1. Base HTML structure includes sidebar, mobile header, and main content wrapper.
2. CSS variables (HSL colors, radius, shadows) and system font stack are applied.
3. No external dependencies (CDNs) are loaded; inline SVGs are used.

**Plans:** 3 plans

Plans:
- [x] 18-01-PLAN.md â€” CSS Design System Refactor
- [x] 18-02-PLAN.md â€” SPA Shell HTML and JS Routing Setup
- [x] 18-03-PLAN.md â€” HTML Content Migration and C Server Integration

**Phase 19: Dashboard & Control Implementation**
Goal: Redesign Pump, Timer, and Cooling controls to card-based layout.
Requirements: DASH-UI-01, DASH-UI-02
Success criteria:
1. Pump and Timer settings are visually separated in modern cards.
2. Cooling control is distinctly separate from pump controls.
3. Data grid layouts cleanly display values without overlapping.


**Plans:** 2 plans

Plans:
- [x] 19-01-PLAN.md ï¿½ Dashboard Grid Layout & iOS Switch
- [x] 19-02-PLAN.md ï¿½ Circular Progress Rings

**Phase 20: System Status & Hardware/Install Pages**
Goal: Overhaul the status view and GPIO configuration interface.
Requirements: SYS-UI-02, SYS-UI-03
Success criteria:
1. System status displays chip, memory (with progress bar), and uptime cleanly.
2. Hardware/Install page distinguishes active and pending configurations.
3. Responsive design ensures these pages are readable on mobile.

**Plans:** 1 plans

Plans:
- [ ] 20-01-PLAN.md — Overhaul system status and GPIO configuration interfaces

**Phase 21: Wi-Fi Setup & UX Polish**
Goal: Overhaul Wi-Fi Setup stepper and integrate loading/modal feedback across the app.
Requirements: SYS-UI-01, UX-UI-01, UX-UI-02
Success criteria:
1. Wi-Fi setup features a 3-step numbered stepper and network list.
2. Action buttons display loading spinners while waiting for API responses.
3. Important actions trigger modal confirmations instead of browser alerts.


