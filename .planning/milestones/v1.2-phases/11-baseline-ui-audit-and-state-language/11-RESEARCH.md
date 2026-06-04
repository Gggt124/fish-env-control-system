# Phase 11 Research: Baseline UI Audit And State Language

**Phase:** 11-baseline-ui-audit-and-state-language  
**Date:** 2026-06-02  
**Status:** Complete  
**Requirements:** UI-13, A11Y-02, A11Y-03

## Scope Read

Phase 11 is a documentation and audit phase before frontend implementation changes begin. It must produce an evidence-backed baseline for Login, App Shell, Dashboard, Hardware/Install, Wi-Fi, and Status at desktop `1440px` and mobile `375px`, then hand Phase 12 and Phase 13 separate implementation briefs.

The phase must not redesign the UI, change the stable v1.1 runtime, rewrite Wi-Fi/APSTA behavior, add relay/timer/cooling logic, introduce remote assets, or add a frontend framework. The baseline should turn the existing `PRODUCT.md` register and `DESIGN.md` tokens into a compact state-language contract reusable by the embedded web UI and a future small TFT interface.

## Existing Frontend Baseline

### Embedded Asset Shape

`main/CMakeLists.txt` embeds seven local frontend files:

| Asset | Bytes |
|-------|------:|
| `main/static/app.js` | 80,022 |
| `main/static/style.css` | 34,802 |
| `main/static/dashboard.html` | 18,308 |
| `main/static/wifi.html` | 8,456 |
| `main/static/status.html` | 7,959 |
| `main/static/hardware.html` | 7,920 |
| `main/static/login.html` | 1,969 |
| **Total** | **159,436** |

The latest available `build/fish_pump_relay_timer_control.bin` is `1,111,616` bytes. Phase 11 should record this as a baseline, not rebuild firmware unless execution needs to refresh evidence. Phase 14 remains responsible for the full build and partition-footprint regression gate after implementation changes.

Static source search found no CDN, external script, remote font, Tailwind, Google Fonts, or Material Symbols references under `main/static`. Keep that offline constraint explicit in every handoff brief.

### Existing Strengths To Preserve

- `PRODUCT.md` already defines the correct product register: calm, clear, trustworthy local appliance control.
- `DESIGN.md` already defines local system fonts, semantic tokens, compact radii, restrained elevation, responsive shell breakpoints, and offline embedded constraints.
- Login has visible labels, autocomplete attributes, disabled submit while authenticating, and an inline error target.
- Dashboard already separates pump and cooling channels, protects stale/fault states, disables unsafe actions while data is stale or requests are pending, and uses contextual text plus toasts for several actions.
- Hardware/Install already separates active and pending maps, requires a reboot-confirmation checkbox, and renders reboot-required wording.
- Wi-Fi already exposes scan, selection, password visibility, connect, disconnect, and static-IP flows with disabled controls while unavailable.
- Status already separates system, memory, uptime, STA, AP, and service diagnostics into scannable groups.

## Source-Level Audit Findings

The Phase 11 executor must validate these findings visually and expand them into screenshot-backed audit rows.

### Accessibility Gaps

1. **Dynamic messages are not programmatically announced.** No `aria-live`, `role="status"`, or `role="alert"` attributes were found in the embedded HTML. This affects login errors, pump/cooling errors, hardware-save messages, Wi-Fi connect messages, and toasts. This is the primary A11Y-03 gap.
2. **Focus treatment is partial.** CSS defines `:focus` styling for `.input-wrapper input` and `.duration-field input/select`, but not a shared `:focus-visible` rule for links, buttons, checkboxes, navigation items, Wi-Fi scan controls, and selectable network rows. This is the primary A11Y-02 gap.
3. **Selectable Wi-Fi rows need keyboard semantics.** `app.js` generates clickable `.network-item` `<li>` rows with `onclick` for unconnected networks. The nested Select button is focusable, but row-level interaction and selected/disabled semantics should be audited and normalized.
4. **Compact actions need touch-target review.** `.btn-sm` uses `6px 12px` padding and the Wi-Fi `.scan-btn` is similarly compact. Phase 11 must measure representative actions at `375px` and flag any target smaller than `44x44px`.
5. **Icon-only mobile topbar controls need accessible names and consistent navigation.** Mobile headers use Unicode icons and back arrows. Phase 12 should make shell navigation understandable without relying on glyph recognition.
6. **Reduced motion is not covered.** `style.css` contains pulse, toast slide-in, spinner, and width transition rules but no `prefers-reduced-motion` override.

### State-Language Gaps

1. Existing async actions already use useful loading labels, disabled controls, errors, and success toasts, but wording and semantics are inconsistent across login, pump, cooling, hardware, and Wi-Fi.
2. Hardware pending-reboot wording exists and should become the canonical `pending reboot` state example.
3. Wi-Fi has an initial prompt, scanning state, scan error, selected-network state, connected state, and disconnect state. The executor must capture an explicit empty-network state and verify that connect errors include a recovery instruction.
4. Dashboard and Hardware/Install use multiple durable message targets, but the audit must distinguish contextual messages from transient toasts and map both to assistive announcements.
5. Disabled state should mean one of three explicit conditions from context: `processing`, `unavailable` with a reason, or `pending reboot`.

### Hierarchy And Responsive Risks

1. Dashboard contains dense pump runtime, cooling runtime, settings, and quick diagnostics. Phase 12 must preserve pump-first hierarchy while making daily controls visually distinct from configuration.
2. Mobile app-shell navigation is incomplete compared with the desktop sidebar. Phase 12 must define a consistent authenticated navigation pattern across all protected surfaces.
3. Hardware/Install is intentionally dense. Phase 13 must verify active, pending, and reboot-required information remains legible and ordered correctly at `375px`.
4. Status uses responsive card grids, but long SSIDs, MAC addresses, firmware values, and diagnostic labels need narrow-width screenshot validation.

## Deterministic Impeccable Detector

Command:

```powershell
node C:\Users\Copter\.agents\skills\impeccable\scripts\detect.mjs --json main/static
```

The detector returned six warnings:

| Rule | File | Line | Baseline Finding |
|------|------|-----:|------------------|
| `side-tab` | `main/static/style.css` | 318 | Active navigation uses a thick right accent border. |
| `side-tab` | `main/static/style.css` | 532 | Pump runtime uses a thick left accent border. |
| `side-tab` | `main/static/style.css` | 767 | Cooling runtime uses a thick left accent border. |
| `side-tab` | `main/static/style.css` | 819 | Hardware status banner uses a thick left accent border. |
| `layout-transition` | `main/static/style.css` | 470 | Progress fill animates `width`. |
| `flat-type-hierarchy` | `main/static/wifi.html` | 59 | Wi-Fi page type scale needs visual review. |

These are audit inputs, not automatic defects. The executor should classify false positives explicitly. For example, the progress bar width transition may be acceptable if it is bounded and does not cause visible jank, while thick side accents conflict with the current `impeccable` anti-pattern rules and deserve deliberate review.

## Impeccable And UI-UX-Pro-Max Adaptation

Apply the `impeccable` product-register audit and critique workflows sequentially during execution:

1. Run a visual critique for hierarchy, state clarity, cognitive load, persona risks, and product trust.
2. Run the technical audit for accessibility, performance, theming/token consistency, responsive behavior, and anti-patterns.
3. Run the deterministic detector and preserve the JSON findings.
4. Capture screenshot evidence where browser automation is practical.

Adapt the installed `ui-ux-pro-max` checklist to this embedded HTML/CSS/JS project:

| Keep | Adapt For ESP32 | Exclude |
|------|-----------------|---------|
| WCAG-oriented contrast, keyboard focus, labels, live regions, touch targets, no color-only meaning | Use local Unicode icons already accepted by `DESIGN.md`; use compact state text and lightweight spinners; validate asset bytes and firmware footprint | CDN assets, Google Fonts, remote icon packages, framework migration, heavy animation, image optimization work where no images exist |
| Mobile-first reflow, no horizontal overflow, readable type, clear primary action, loading/error/empty/disabled states | Test `375px` and `1440px`; keep system fonts; preserve APSTA/offline wording | Dark-mode expansion, chart guidance, package-based motion systems |
| Semantic tokens and consistent component vocabulary | Reuse `DESIGN.md` variables and existing plain CSS classes | New dependency-managed design-system packages |

The installed `ui-ux-pro-max` skill references a local `scripts/search.py` path that is not present on this machine. The executor should use the checklist embedded in the skill text unless that installation is repaired; Phase 11 must not block on the optional search helper.

## Browser Evidence Strategy

The Browser plugin is installed and its skill contract supports local URLs, background tabs, DOM snapshots, and screenshots. The current planning session could not start the browser automation connection because the Windows sandbox reported a local spawn/setup failure. Execution should retry before falling back.

Recommended execution behavior:

1. Prefer a known flashed-device URL if the ESP32 is reachable, because it exposes real authenticated state and APIs.
2. If device access is unavailable, serve `main/static/` locally and capture static baseline surfaces, clearly marking API-backed states as simulated or not-run.
3. Capture `1440px` desktop and `375px` mobile evidence when viewport control is available.
4. Organize screenshots by page and viewport. Record any state that could not be simulated safely.
5. Do not claim browser evidence passed when automation is unavailable; record the exact fallback used.

## Shared State Vocabulary

Use a compact vocabulary in the Phase 11 audit and handoff briefs:

| State | Meaning | Required UI Treatment |
|-------|---------|-----------------------|
| `loading` | A request is in flight. | Short action-specific text, relevant control disabled, small spinner where useful, polite announcement. |
| `success` | The requested action completed. | Contextual confirmation near the initiating area, short toast where useful, polite announcement. |
| `error` | The action failed or data is unsafe. | Message near the relevant control or status area, brief recovery instruction, immediate alert announcement. |
| `empty` | The request succeeded but returned no usable items. | Explain the empty result and the next available action. |
| `disabled` | An action cannot run. | Preserve semantic `disabled`, visibly reduce emphasis, and expose a reason when it is not obvious. |
| `unavailable` | A prerequisite or safe runtime condition is absent. | State the reason, such as stale data, sensor fault, or no selected SSID. |
| `pending reboot` | Saved hardware values are not active yet. | Show active and pending values separately and state that reboot is required before behavior changes. |
| `disconnected` | STA is not connected. | Preserve SoftAP fallback context and offer scan/connect recovery. |

## Recommended Phase 11 Plan Split

### Plan 11-01: Baseline Evidence Pack And Impeccable Audit

Create the screenshot directory and Markdown audit report. Run critique, technical audit, deterministic detector, and source checks across all six surfaces. Capture desktop/mobile screenshots and representative loading, error, empty, disabled, and pending-reboot states where practical. Classify findings as `Must fix`, `Should fix`, or `Enhancement`.

### Plan 11-02: Shared State Language And ESP32-Adapted Checklist

Create a concise state-language contract from `PRODUCT.md`, `DESIGN.md`, context decisions, audit evidence, and source patterns. Include accessibility semantics, focus contract, touch-target rule, contextual-message versus toast guidance, offline asset constraints, and footprint baseline.

### Plan 11-03: Phase 12 And Phase 13 Implementation Briefs

Create separate implementation briefs:

- Phase 12: Login, App Shell, Dashboard.
- Phase 13: Hardware/Install, Wi-Fi, Status, and applicable state gaps.

Each finding should include the problem, screenshot/source evidence, user impact, acceptance criteria, priority, and target phase. Record the adapted `ui-ux-pro-max` checklist in both briefs where applicable.

## Validation Architecture

Phase 11 produces documentation and evidence, so validation should combine fast source checks with manual review:

- After each task: run focused `rg` checks against the new report, checklist, brief, and evidence paths.
- After the evidence task: verify every required surface has desktop/mobile screenshot entries or an explicit not-run reason.
- After the language task: verify all eight canonical vocabulary states plus focus, labels, live regions, touch targets, local assets, and footprint are documented.
- After the handoff task: verify separate Phase 12 and Phase 13 briefs include priority, evidence, user impact, acceptance criteria, and adapted checklist sections.
- Before phase verification: run `node --check main/static/app.js`, the impeccable detector, offline-dependency search, and artifact completeness checks.

No new runtime tests or frontend packages are appropriate for Phase 11.

## Risks And Guardrails

- **Audit drift into redesign:** Do not modify `main/static/*` during Phase 11. Report issues and hand them to Phase 12 or 13.
- **False browser confidence:** Screenshot evidence from a local static server cannot prove real ESP32 API behavior. Label simulated and device-backed evidence distinctly.
- **Incomplete accessibility audit:** Treat missing programmatic announcements and missing shared focus-visible rules as baseline gaps even if visual screenshots look acceptable.
- **ESP32 footprint regression:** Keep the asset byte baseline visible so later visual polish stays compatible with the embedded partition budget.
- **Optional helper failure:** The missing `ui-ux-pro-max` search script is not a reason to omit its checklist; use the installed skill guidance directly.

## Reference Tracking

No external repositories were consulted for Phase 11 research, so `REFERENCE.md` does not need an update.

## RESEARCH COMPLETE
