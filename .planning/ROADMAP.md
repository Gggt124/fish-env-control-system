# Roadmap: Fish Pump Relay Timer Control

**Created:** 2026-05-18
**Mode:** Vertical MVP

## Milestones

- ✅ **v1.0 MVP** — Phases 1-5 (shipped 2026-05-20) — `.planning/milestones/v1.0-ROADMAP.md`
- ✅ **v1.1 Dual Relay Cooling And Install UI** — Phases 6-10 (shipped 2026-06-02) — `.planning/milestones/v1.1-ROADMAP.md`
- ✅ **v1.2 Owner UI Polish And Hardware Readiness** — Phases 11-14 (shipped 2026-06-04) — `.planning/milestones/v1.2-ROADMAP.md`
- ✅ **v1.3 UI Details Refinement** — Phase 15 (shipped 2026-06-05) — `.planning/milestones/v1.3-ROADMAP.md`
- ✅ **v1.4 Wi-Fi UI Polish and Code Review** — Phases 16-17 (shipped 2026-06-06) — `.planning/milestones/v1.4-ROADMAP.md`

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

<details>
<summary>✅ v1.4 Wi-Fi UI Polish and Code Review (Phases 16-17) — SHIPPED 2026-06-06</summary>

- [x] Phase 16: Wi-Fi Setup UI Polish (1/1 plans) — completed 2026-06-05
- [x] Phase 17: Code Quality Review & Stability Validation (1/1 plans) — completed 2026-06-06

</details>

### 🚧 Next Milestone (Awaiting Definition)

- [x] Phase 18: TFT Display Integration (0/2 plans) (completed 2026-06-05)

## Phase Details

### Phase 18: TFT Display Integration
**Goal**: Integrate the 2.4" TFT (ILI9341) display using native `esp_lcd` to display real-time pump, timer, temperature, and network status in landscape mode.
**Depends on**: Phase 17
**Requirements**:
  - TFT-01: Initialize SPI bus (VSPI) and native `esp_lcd` driver for ILI9341 display.
  - TFT-02: Implement a lightweight display rendering module for custom fonts and shapes.
  - TFT-03: Render the landscape dashboard showing pump state, active timer, countdown, temperature, cooling relay state, float switch, and Wi-Fi IP.
  - TFT-04: Update the screen periodically (e.g., every 500ms or on state change) without blocking the main event loop or watchdog.
**Success Criteria** (what must be TRUE):
  1. The TFT display initializes successfully on boot.
  2. The display renders correct real-time status in landscape mode.
  3. The firmware builds without warnings and runs stably during regression testing.
**Plans**:
- [x] 18-01-PLAN.md — Initialize SPI bus, native esp_lcd ILI9341 driver, lightweight drawing primitives, and boot splash screen
- [x] 18-02-PLAN.md — Implement landscape status dashboard layout and FreeRTOS differential update task

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
| 17. Code Quality Review & Stability Validation | 1/1 | Complete | 2026-06-06 |
| 18. TFT Display Integration | 4/4 | Complete    | 2026-06-06 |

---
*Roadmap created: 2026-05-18*
*Last updated: 2026-06-06 for active milestone v1.4 completion*
