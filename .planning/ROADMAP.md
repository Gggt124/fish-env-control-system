# Roadmap: Fish Pump Relay Timer Control

**Created:** 2026-05-18
**Mode:** Vertical MVP

## Milestones

- ✅ **v1.0 MVP** — Phases 1-5 (shipped 2026-05-20) — `.planning/milestones/v1.0-ROADMAP.md`
- ✅ **v1.1 Dual Relay Cooling And Install UI** — Phases 6-10 (shipped 2026-06-02) — `.planning/milestones/v1.1-ROADMAP.md`
- ✅ **v1.2 Owner UI Polish And Hardware Readiness** — Phases 11-14 (shipped 2026-06-04) — `.planning/milestones/v1.2-ROADMAP.md`
- ✅ **v1.3 UI Details Refinement** — Phase 15 (shipped 2026-06-05) — `.planning/milestones/v1.3-ROADMAP.md`
- ⏳ **v1.4 Wi-Fi UI Polish and Code Review** — Phases 16-17 (active)

## Phases

<details>
<summary>✅ v1.0 MVP (Phases 1-5) — SHIPPED 2026-05-20</summary>

- [x] Phase 1: Hardware-Safe Pump Control Core (3/3 plans) — completed 2026-05-19
- [x] Phase 2: Pump Settings Persistence And Boot Behavior (3/3 plans) — completed 2026-05-19
- [x] Phase 3: Authenticated Pump Control API (3/3 plans) — completed 2026-05-20
- [x] Phase 4: Web Pump Control UI (3/3 plans) — completed 2026-05-20
- [x] Phase 5: Hardware Validation And Regression Pass (1/1 plan) — completed 2026-05-20

</details>

<details>
<summary>✅ v1.1 Dual Relay Cooling And Install UI (Phases 6-10) — SHIPPED 2026-06-02</summary>

- [x] Phase 6: Hardware Contract And Persistent Map Foundation (3/3 plans) — completed 2026-05-22
- [x] Phase 7: Dual Timer Relay Pump Runtime (3/3 plans) — completed 2026-05-23
- [x] Phase 8: DS18B20 Cooling Runtime (3/3 plans) — completed 2026-05-23
- [x] Phase 9: Authenticated Hardware And Cooling APIs (3/3 plans) — completed 2026-05-23
- [x] Phase 10: Owner Dashboard, Hardware Install UI, And Validation (3/3 plans) — completed 2026-06-02

</details>

<details>
<summary>✅ v1.2 Owner UI Polish And Hardware Readiness (Phases 11-14) — SHIPPED 2026-06-04</summary>

- [x] Phase 11: Baseline UI Audit And State Language (3/3 plans) — completed 2026-06-02
- [x] Phase 12: App Shell Login And Owner Dashboard (3/3 plans) — completed 2026-06-03
- [x] Phase 13: Install Setup, Status UX, and Professional UI Overhaul (3/3 plans) — completed 2026-06-03
- [x] Phase 14: Full UI Visual Regression Validation (4/4 plans) — completed 2026-06-03

</details>

<details>
<summary>✅ v1.3 UI Details Refinement (Phase 15) — SHIPPED 2026-06-05</summary>

- [x] Phase 15: UI Refinements (3/3 plans) — completed 2026-06-05

</details>

### Active Milestone: v1.4 Wi-Fi UI Polish and Code Review

- [x] **Phase 16: Wi-Fi Setup UI Polish** - Clean empty state card and smooth form transition on the Wi-Fi setup page (completed 2026-06-05)
- [ ] **Phase 17: Code Quality Review & Stability Validation** - Review firmware source and web assets to resolve warnings, console errors, and ensure runtime stability

## Phase Details

### Phase 16: Wi-Fi Setup UI Polish
**Goal**: Users can easily configure Wi-Fi using a polished, non-blurry interface with a clear empty state and smooth transitions.
**Depends on**: Phase 15
**Requirements**: WIFI-UI-01, WIFI-UI-02, WIFI-UI-03, WIFI-UI-04
**Success Criteria** (what must be TRUE):
  1. When opening the Wi-Fi page, the user sees a styled Empty State Card in place of the input form instructing them to select a network from the list on the left.
  2. Clicking a network in the scan list smoothly fades in the Wi-Fi credentials input form and hides the Empty State Card.
  3. Clicking the "Cancel" (ยกเลิก) button clears the selected network, hides the input form, and restores the Empty State Card.
  4. The Wi-Fi configuration UI operates smoothly and fits the styling/theme design system without CSS backdrop blur performance issues.
**Plans**: [16-01-PLAN.md](phases/16-wi-fi-setup-ui-polish/16-01-PLAN.md)
**UI hint**: yes

### Phase 17: Code Quality Review & Stability Validation
**Goal**: The firmware runs with high stability, clean code structure, and zero compile warnings or javascript console errors, maintaining the verified pump, cooling, and Wi-Fi soak stability.
**Depends on**: Phase 16
**Requirements**: CODE-REV-01, CODE-REV-02, CODE-REV-03
**Success Criteria** (what must be TRUE):
  1. The firmware builds successfully (`idf.py build`) without compile warnings.
  2. The web interface loads in the browser and functions without JavaScript console errors.
  3. The controller operates stably under manual testing and runs pump and cooling control without unexpected crashes, watchdog resets, or connection drops.
**Plans**: TBD

## Progress

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Hardware-Safe Pump Control Core | 3/3 | Complete | 2026-05-19 |
| 2. Pump Settings Persistence And Boot Behavior | 3/3 | Complete | 2026-05-19 |
| 3. Authenticated Pump Control API | 3/3 | Complete | 2026-05-20 |
| 4. Web Pump Control UI | 3/3 | Complete | 2026-05-20 |
| 5. Hardware Validation And Regression Pass | 1/1 | Complete | 2026-05-20 |
| 6. Hardware Contract And Persistent Map Foundation | 3/3 | Complete | 2026-05-22 |
| 7. Dual Timer Relay Pump Runtime | 3/3 | Complete | 2026-05-23 |
| 8. DS18B20 Cooling Runtime | 3/3 | Complete | 2026-05-23 |
| 9. Authenticated Hardware And Cooling APIs | 3/3 | Complete | 2026-05-23 |
| 10. Owner Dashboard, Hardware Install UI, And Validation | 3/3 | Complete | 2026-06-02 |
| 11. Baseline UI Audit And State Language | 3/3 | Complete | 2026-06-02 |
| 12. App Shell Login And Owner Dashboard | 3/3 | Complete | 2026-06-03 |
| 13. Install Setup, Status UX, and Professional UI Overhaul | 3/3 | Complete | 2026-06-03 |
| 14. Full UI Visual Regression Validation | 4/4 | Complete | 2026-06-03 |
| 15. UI Refinements | 3/3 | Complete | 2026-06-05 |
| 16. Wi-Fi Setup UI Polish | 1/1 | Complete   | 2026-06-05 |
| 17. Code Quality Review & Stability Validation | 0/0 | Not started | - |

---
*Roadmap created: 2026-05-18*
*Last updated: 2026-06-05 for active milestone v1.4*
