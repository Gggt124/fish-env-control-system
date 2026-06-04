# Stack Research

**Domain:** Embedded offline ESP32 owner dashboard UI polish
**Researched:** 2026-06-02
**Confidence:** HIGH

## Recommendation

Keep the existing embedded frontend stack unchanged:

| Technology | Purpose | Recommendation |
|------------|---------|----------------|
| Embedded HTML | Page structure | Keep static pages under `main/static/`; improve semantic structure and state containers in place. |
| Plain CSS | Visual hierarchy and responsive layout | Extend existing CSS variables, layout grids, focus states, and responsive breakpoints. |
| Vanilla JavaScript | API polling and UI state rendering | Reuse existing `apiGet`, `apiPost`, polling, validation, and renderer functions. |
| ESP-IDF `EMBED_FILES` | Offline asset delivery | Keep all assets local and embedded; do not add CDN or runtime network dependencies. |

## New Dependencies

None. v1.2 is a focused UI/UX and documentation milestone. A frontend framework, icon package, font package, or animation library would increase flash usage and maintenance cost without solving the operator problems.

## Existing Fit

- `main/static/style.css` already defines shared tokens, responsive breakpoints, buttons, cards, alerts, disabled states, dashboard runtime panels, and Hardware/Install layouts.
- `main/static/app.js` already centralizes HTTP transport and owns status rendering for dashboard, cooling, hardware map, Wi-Fi setup, and diagnostics.
- The embedded static assets are approximately 150 KB before firmware embedding, so focused edits remain appropriate for classic ESP32.

## Development Tools

| Tool | Purpose | Notes |
|------|---------|-------|
| `idf.py build` | Firmware and embedded asset validation | Required release gate. |
| In-app browser screenshots | Visual verification | Capture desktop and narrow mobile viewports plus loading/error/empty states where applicable. |
| Browser keyboard pass | Accessibility verification | Verify focus visibility, logical order, and usable controls without a mouse. |
| `ui-ux-pro-max` | Design-system and UI checklist gate | Apply product-dashboard, accessibility, touch, responsive, and interaction guidance within offline embedded constraints. |
| `impeccable` | Product UI critique and polish gate | Use root `PRODUCT.md` and `DESIGN.md` to guide critique, audit, hardening, and final polish. |

## What Not To Add

| Avoid | Why | Use Instead |
|-------|-----|-------------|
| React, Vue, or a component bundle | Adds build complexity and flash cost for a small offline UI. | Existing static HTML, CSS variables, and JavaScript renderers. |
| CDN fonts, icons, or CSS | SoftAP clients may have no internet access. | System fonts and local HTML entities already used by the project. |
| Heavy animation | Can distract from operational state and add low-end rendering cost. | Static hierarchy and minimal state transitions. |
| New firmware runtime APIs for cosmetic work | Risks the stable v1.1 baseline. | Re-present existing API state unless a verified UI state bug requires a firmware fix. |

## Sources

- Local frontend: `main/static/*.html`, `main/static/style.css`, `main/static/app.js`
- W3C WCAG 2.2: https://www.w3.org/TR/WCAG22/
- W3C WAI Understanding Reflow: https://www.w3.org/WAI/WCAG22/Understanding/reflow.html

---
*Stack research for: embedded offline ESP32 owner dashboard UI polish*
*Researched: 2026-06-02*
