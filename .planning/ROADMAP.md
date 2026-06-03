# Roadmap: Fish Pump Relay Timer Control

**Created:** 2026-05-18  
**Mode:** Vertical MVP
**Current Milestone:** v1.2 Owner UI Polish And Hardware Readiness

## Milestones

- ✅ **v1.0 MVP** — Phases 1-5 (shipped 2026-05-20) — `.planning/milestones/v1.0-ROADMAP.md`
- ✅ **v1.1 Dual Relay Cooling And Install UI** — Phases 6-10 (shipped 2026-06-02) — `.planning/milestones/v1.1-ROADMAP.md`
- 🔄 **v1.2 Owner UI Polish And Hardware Readiness** — Phases 11-14 (planning)

## Phases

<details>
<summary>✅ v1.0 MVP (Phases 1-5) — SHIPPED 2026-05-20</summary>

#### Phase 1: Hardware-Safe Pump Control Core

**Goal:** Establish safe pump-control defaults and core relay/timer runtime behavior.

**Status:** Complete (3/3 plans) — completed 2026-05-19

#### Phase 2: Pump Settings Persistence And Boot Behavior

**Goal:** Persist pump settings and safe boot behavior.

**Status:** Complete (3/3 plans) — completed 2026-05-19

#### Phase 3: Authenticated Pump Control API

**Goal:** Add authenticated API control for pump runtime and configuration state.

**Status:** Complete (3/3 plans) — completed 2026-05-20

#### Phase 4: Web Pump Control UI

**Goal:** Add the initial embedded web UI for pump control.

**Status:** Complete (3/3 plans) — completed 2026-05-20

#### Phase 5: Hardware Validation And Regression Pass

**Goal:** Validate hardware behavior and regression safety for the v1.0 MVP.

**Status:** Complete (1/1 plan) — completed 2026-05-20

</details>

<details>
<summary>✅ v1.1 Dual Relay Cooling And Install UI (Phases 6-10) — SHIPPED 2026-06-02</summary>

#### Phase 6: Hardware Contract And Persistent Map Foundation

**Goal:** Define and persist the hardware map contract for pump, float, cooling, and install flows.

**Status:** Complete (3/3 plans) — completed 2026-05-22

#### Phase 7: Dual Timer Relay Pump Runtime

**Goal:** Implement dual-timer relay pump runtime behavior.

**Status:** Complete (3/3 plans) — completed 2026-05-23

#### Phase 8: DS18B20 Cooling Runtime

**Goal:** Add DS18B20 cooling runtime support.

**Status:** Complete (3/3 plans) — completed 2026-05-23

#### Phase 9: Authenticated Hardware And Cooling APIs

**Goal:** Add authenticated APIs for hardware map and cooling runtime state.

**Status:** Complete (3/3 plans) — completed 2026-05-23

#### Phase 10: Owner Dashboard, Hardware Install UI, And Validation

**Goal:** Add owner dashboard cooling visibility and Hardware/Install configuration UI with validation.

**Status:** Complete (3/3 plans) — completed 2026-06-02

</details>

### v1.2 Owner UI Polish And Hardware Readiness

#### Phase 11: Baseline UI Audit And State Language

**Goal:** Audit the existing UI before implementation, capture evidence-backed findings, and establish the shared product-dashboard state language for the improvement pass.

**Requirements:** UI-13, A11Y-02, A11Y-03

**Success criteria:**
1. Existing login, app shell, dashboard, Hardware/Install, Wi-Fi, and Status surfaces receive an `impeccable` critique and technical audit before implementation changes begin.
2. Baseline findings identify hierarchy, responsive, accessibility, loading, error, empty, disabled, and pending-reboot gaps with screenshot evidence where practical.
3. Existing `DESIGN.md` tokens and `PRODUCT.md` product-register principles define the shared state language without adding a frontend framework, CDN, or remote assets.
4. Improvement requirements include visible keyboard focus, understandable labels or instructions, and programmatically recognizable dynamic status and error messages.
5. The Phase 12-13 implementation briefs record the applicable `ui-ux-pro-max` design-system checklist, adapted to offline embedded ESP32 constraints.

#### Phase 12: App Shell Login And Owner Dashboard

**Goal:** Use the baseline audit and `ui-ux-pro-max` guidance to improve entry, navigation, and daily operator confidence while preserving stable v1.1 control behavior.

**Requirements:** UI-08, UI-09, UI-10, UI-14, UI-15

**Success criteria:**
1. `ui-ux-pro-max`, `frontend-ui-ux`, `PRODUCT.md`, and `DESIGN.md` guide the implementation changes within the existing embedded HTML/CSS/JS stack.
2. Login clearly presents credential entry, loading, authentication error, and recovery guidance on desktop and narrow mobile layouts.
3. Authenticated navigation is consistent across dashboard, Hardware/Install, Wi-Fi, Status, and logout on desktop and narrow mobile layouts.
4. Dashboard makes pump state, selected timer/relay, phase, countdown, and float switch state recognizable within seconds.
5. Cooling temperature, mode, relay state, and sensor fault appear as a separate operational channel from the pump relays.
6. Daily operation controls are visually distinct from settings and secondary diagnostics.

#### Phase 13: Install Setup, Status UX, and Professional UI Overhaul

**Goal:** Improve installer, Wi-Fi setup, and system-status workflows, and perform a deep CSS overhaul to elevate the entire app shell to a professional, premium aesthetic (Bento Grid, modern typography, glassmorphism) without adding external asset footprint.

**Requirements:** HW-06, UI-11, UI-12, SETUP-01, SETUP-02, UI-16

**Success criteria:**
1. `style.css` is completely overhauled with a professional HSL color palette, modern system font stacks, and subtle micro-animations/shadows to feel like a premium SaaS dashboard.
2. Hardware/Install clearly distinguishes active GPIO, pending GPIO, and reboot-required state on desktop and narrow mobile layouts while preserving safe dropdown and confirmation behavior.
3. Hardware/Install and hardware-readiness documentation explicitly show the external `4.7 kOhm` DS18B20 DQ-to-`3.3 V` pull-up requirement.
4. Wi-Fi scan and connect flow clearly presents loading, error, empty, disconnected, and connected states without disturbing APSTA behavior.
5. Dashboard and Hardware/Install operations intentionally present loading, success, error, disabled, and pending-reboot states.
6. Status diagnostics for system, memory, uptime, Wi-Fi station, access point, and services remain scannable and visually cohesive with the new aesthetic.

#### Phase 14: Full UI Visual Regression Validation

**Goal:** Close v1.2 with a repeated full-UI quality review plus screenshot, accessibility, build, footprint, and stable-baseline regression evidence.

**Requirements:** A11Y-01, VER-01, VER-02, VER-03, VER-04, REG-01

**Success criteria:**
1. Login, app shell, dashboard, Hardware/Install, Wi-Fi, and Status pages remain usable on a narrow mobile viewport without hidden primary content or actions.
2. Screenshot evidence covers every affected page at desktop and narrow mobile widths plus representative loading, error, empty, disabled, and pending-reboot states.
3. A closing `impeccable` critique/audit/polish review repeats the Phase 11 checks against every improved page, with actionable findings addressed or explicitly recorded.
4. The applicable `ui-ux-pro-max` checklist is reviewed against the completed UI within offline embedded constraints.
5. `idf.py build` succeeds without unsuitable frontend dependencies or unsafe embedded asset growth.
6. Manual regression confirms stable v1.1 relay, timer, cooling, Wi-Fi, and APSTA flows remain intact.

## Progress

| Phase | Milestone | Plans Complete | Status | Completed |
|-------|-----------|---------------|--------|-----------|
| 1. Hardware-Safe Pump Control Core | v1.0 | 3/3 | Complete | 2026-05-19 |
| 2. Pump Settings Persistence And Boot Behavior | v1.0 | 3/3 | Complete | 2026-05-19 |
| 3. Authenticated Pump Control API | v1.0 | 3/3 | Complete | 2026-05-20 |
| 4. Web Pump Control UI | v1.0 | 3/3 | Complete | 2026-05-20 |
| 5. Hardware Validation And Regression Pass | v1.0 | 1/1 | Complete | 2026-05-20 |
| 6. Hardware Contract And Persistent Map Foundation | v1.1 | 3/3 | Complete | 2026-05-22 |
| 7. Dual Timer Relay Pump Runtime | v1.1 | 3/3 | Complete    | 2026-05-23 |
| 8. DS18B20 Cooling Runtime | v1.1 | 3/3 | Complete    | 2026-05-23 |
| 9. Authenticated Hardware And Cooling APIs | v1.1 | 3/3 | Complete   | 2026-05-23 |
| 10. Owner Dashboard, Hardware Install UI, And Validation | v1.1 | 3/3 | Complete | 2026-06-02 |
| 11. Baseline UI Audit And State Language | v1.2 | 3/3 | Complete    | 2026-06-02 |
| 12. App Shell Login And Owner Dashboard | v1.2 | 3/3 | Complete | 2026-06-03 |
| 13. Install Setup, Status UX, and Professional UI Overhaul | v1.2 | 3/3 | Complete | 2026-06-03 |
| 14. Full UI Visual Regression Validation | v1.2 | 0/TBD | Planned | — |

---
*Roadmap created: 2026-05-18*
*Last updated: 2026-06-03 after GSD health reconciliation*
