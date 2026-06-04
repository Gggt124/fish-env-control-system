# Project Research Summary

**Project:** Fish Pump Relay Timer Control
**Domain:** Embedded offline ESP32 owner dashboard UI polish
**Researched:** 2026-06-02
**Confidence:** HIGH

## Executive Summary

v1.2 should improve the full embedded local web UI without changing the stable v1.1 firmware contract. The product is a local appliance controller, not a generic admin dashboard: login, navigation, daily operation, installation, Wi-Fi setup, and status diagnostics should form one coherent flow. The owner should understand pump and cooling safety within seconds, while the installer should have a separate mobile-friendly Hardware/Install flow with explicit wiring and reboot guidance.

No stack additions are recommended. The existing HTML, CSS, JavaScript, ESP-IDF embedded assets, authenticated APIs, and bounded polling model are sufficient. The visual thesis is a calm operational control panel: current state first, primary actions obvious, advanced settings separated, and error states explicit.

The main risks are accidental firmware behavior changes during UI work, dense equal-weight presentation, incomplete edge states, desktop-only wiring flow, and failing to close the DS18B20 pull-up readiness gap. Prevent these with frontend-scoped phases, state-matrix design, screenshot verification at desktop and mobile widths, keyboard checks, and a final ESP-IDF build regression.

Use `frontend-ui-ux`, `ui-ux-pro-max`, and `impeccable` as complementary gates. `ui-ux-pro-max` supplies design-system, accessibility, responsive, touch, and interaction checks. `impeccable` supplies the product-register critique, audit, hardening, and pre-ship polish passes. Recommendations must be adapted to the existing embedded UI: preserve local Unicode icons where they fit the offline design system, and do not introduce remote assets, a framework migration, or decorative motion.

## Key Findings

### Stack Additions

None. Keep plain embedded HTML/CSS/JS and existing APIs.

### Feature Table Stakes

- Owner-first dashboard hierarchy for pump and cooling state.
- Explicit loading, error, empty, disabled, success, and pending reboot states.
- Responsive mobile layouts for daily control, wiring, and Wi-Fi setup.
- Visible focus, clear labels/instructions, and programmatically recognizable status updates.
- DS18B20 DQ-to-3.3 V external `4.7 kOhm` pull-up documentation.
- Screenshot verification plus `idf.py build`.

### Watch Out For

1. Do not rewrite relay/timer/cooling or APSTA logic unless a verified UI state bug requires it.
2. Do not add a frontend framework, CDN, fonts, icon bundles, charts, or analytics.
3. Preserve polling behavior and bounded DOM updates to protect long-uptime behavior.
4. Treat the eight scanner rows as bookkeeping debt only.

## Suggested Roadmap Shape

### Phase 11: Baseline UI Audit And State Language

Run an `impeccable` critique and technical audit on the current UI before implementation. Capture baseline findings, refresh the UI contract, and define shared hierarchy, responsive, accessibility, and state-presentation conventions.

### Phase 12: App Shell Login And Owner Dashboard

Use `ui-ux-pro-max`, `frontend-ui-ux`, `PRODUCT.md`, and `DESIGN.md` to improve login, navigation, and the daily owner dashboard.

### Phase 13: Install Setup And Status UX

Improve Hardware/Install, DS18B20 pull-up guidance, Wi-Fi edge states, and status diagnostics.

### Phase 14: Full UI Visual Regression Validation

Repeat the `impeccable` critique/audit/polish review against every improved page and verify screenshots, keyboard use, embedded size, build validity, and stable-baseline constraints.

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | Existing embedded stack already fits the offline ESP32 constraint. |
| Features | HIGH | User scope, current UI, and owner/installer note align. |
| Architecture | HIGH | Frontend and documentation boundary is clear. |
| Pitfalls | HIGH | v1.1 soak evidence and current code identify regression risks. |

## Sources

### Primary

- Local implementation: `main/static/`
- Local UI note: `.planning/notes/owner-installer-ui-shape.md`
- W3C WCAG 2.2: https://www.w3.org/TR/WCAG22/
- W3C WAI Understanding Reflow: https://www.w3.org/WAI/WCAG22/Understanding/reflow.html
- W3C WAI Understanding Status Messages: https://www.w3.org/WAI/WCAG22/Understanding/status-messages
- W3C WAI Understanding Focus Visible: https://www.w3.org/WAI/WCAG22/Understanding/focus-visible
- W3C WAI Understanding Labels or Instructions: https://www.w3.org/WAI/WCAG22/Understanding/labels-or-instructions
- W3C WAI Understanding Target Size Minimum: https://www.w3.org/WAI/WCAG22/Understanding/target-size-minimum

---
*Research completed: 2026-06-02*
*Ready for roadmap: yes*
