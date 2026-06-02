# Phase 11 Baseline UI Audit

## Scope And Evidence

This is a source-backed baseline audit of the shipped embedded UI. It covers
Login, App Shell, Dashboard, Hardware/Install, Status, and Wi-Fi without
changing `main/static/`. Runtime screenshot capture was attempted with a local
static server, but the browser connection failed twice during setup. Runtime
checks are therefore labeled `not-run`, not passed.

A post-repair supplement later captured four privacy-reviewed protected-page
desktop baselines through the available Chrome extension backend. These are
explicitly labeled `simulated` and `chrome-default`; they do not replace the
deferred `1440px`, API-backed, or device-backed checks. A later in-app browser
pass captured four static protected-page mobile baselines at `375x812`. See
`11-SCREENSHOT-SUPPLEMENT.md`.

Evidence inventory: `evidence/README.md`  
Detector output: `evidence/impeccable-detector.json`

## Audit Health Score

| # | Dimension | Score | Key finding |
| --- | --- | ---: | --- |
| 1 | Accessibility | 1/4 | Dynamic messages lack live-region or alert semantics and focus treatment is incomplete. |
| 2 | Performance | 3/4 | Static assets are lean and local; one bounded progress width transition needs later visual review. |
| 3 | Responsive Design | 2/4 | Breakpoints exist, but compact controls and narrow-width overflow remain unverified. |
| 4 | Theming | 3/4 | CSS variables are established; several local hard-coded values and one-sided accents remain. |
| 5 | Anti-Patterns | 2/4 | Four thick side accents are deliberate-looking but conflict with the approved product audit guidance. |
| **Total** |  | **11/20** | **Acceptable: significant focused work remains.** |

## Anti-Patterns Verdict

The UI is a recognizable embedded control panel, not a generic marketing
surface. Its system fonts, compact layout, local-only assets, restrained
palette, and direct controls fit the product. The main visual tell is repeated
one-sided colored borders on navigation and runtime panels. These accents,
plus dense equal-weight operational panels, make the interface look less
deliberate than the underlying firmware deserves.

## Source Inspection Matrix

| Surface or concern | Evidence label | Source anchors | Baseline observation | Downstream owner |
| --- | --- | --- | --- | --- |
| Login | `source-inspected` | `main/static/login.html`, login styles, `initLogin()` | Visible labels, autocomplete, disabled loading button, and inline error target exist. The error target has no `role="alert"`. | Phase 12 |
| App Shell | `source-inspected` | sidebar and `.mobile-topbar` markup across protected pages | Desktop navigation covers all protected routes and logout. Mobile headers vary by page and rely on icon or back-arrow glyphs without accessible names. | Phase 12 |
| Dashboard hierarchy | `source-inspected` | `main/static/dashboard.html`, pump and cooling renderers | Pump runtime appears first and cooling is separate, but dense settings and diagnostics remain visually close in weight. | Phase 12 |
| Hardware/Install | `source-inspected` | `main/static/hardware.html`, hardware renderers | Active and pending maps, reboot confirmation, and installer pinout exist. Narrow-width ordering and reboot emphasis require runtime capture. | Phase 13 |
| Status | `source-inspected` | `main/static/status.html`, status renderer | Diagnostics are grouped into cards. Long SSID, MAC, firmware, and diagnostic values need narrow-width visual validation. | Phase 13 |
| Wi-Fi scan/connect/disconnect | `source-inspected` | `main/static/wifi.html`, `doScan()`, `doConnect()`, `doDisconnect()` | Disconnected, scanning, empty, error, selected, connecting, success, and disconnect code paths exist. Feedback is not programmatically announced. | Phase 13 |
| CSS hierarchy | `source-inspected` | `main/static/style.css` | Reusable tokens, breakpoints, compact cards, and restrained colors exist. Side accents and closely spaced Wi-Fi type need deliberate review. | Phase 12 and Phase 13 |
| Motion | `source-inspected` | pulse keyframes, toast animation, spinner, progress width transition | Motion communicates state, but no `prefers-reduced-motion` override exists. | Phase 13 |
| Focus treatment | `source-inspected` | input and duration-field focus selectors | Some form inputs receive focus styling. There is no shared `:focus-visible` contract for buttons, links, nav, checkboxes, scan control, or selectable Wi-Fi rows. | Phase 12 |
| Live regions | `source-inspected` | HTML search and dynamic JS updates | No `aria-live`, `role="status"`, or `role="alert"` attributes were found. | Phase 12 and Phase 13 |
| Touch targets | `source-inspected` | `.btn-sm`, `.scan-btn`, generated network select button | Compact controls exist. Their mobile hit areas require `44x44px` normalization or proof during implementation. | Phase 13 |
| Offline dependencies | `source-inspected` | dependency search across `main/static/` | No CDN, remote font, remote script, or remote asset URL was found. | Preserve in both phases |

## State Coverage Matrix

| State | Existing source evidence | Runtime evidence | Baseline conclusion |
| --- | --- | --- | --- |
| `loading` | Login, pump, cooling, hardware, Wi-Fi scan, Wi-Fi connect, and disconnect paths change labels or disable controls. | `not-run` | Normalize copy and announcement behavior. |
| `success` | Toasts and contextual text exist for several actions. | `not-run` | Keep contextual confirmation; add polite announcements. |
| `error` | Inline error areas, Wi-Fi text, pump alerts, and toasts exist. | `not-run` | Add recovery text consistently and use `role="alert"` where immediate action is required. |
| `empty` | Wi-Fi scan renders a no-network row. | `not-run` | Preserve the next-action prompt and announce dynamic population politely. |
| `disabled` | Buttons and panels are disabled during processing or unavailable states. | `not-run` | Distinguish processing, unavailable with reason, and pending reboot. |
| `unavailable` | Stale/fault and prerequisite paths exist in JavaScript. | `not-run` | Make the reason visible and reusable across surfaces. |
| `pending reboot` | Hardware active and pending map sections plus confirmation wording exist. | `not-run` | Promote this to a canonical durable state. |
| `disconnected` | Wi-Fi page and status render STA disconnected context. | `not-run` | Preserve SoftAP fallback and scan/connect recovery. |

## Detector Review

| Detector warning | Evidence | Classification | Priority | Owner |
| --- | --- | --- | --- | --- |
| Active nav thick right border | `style.css:318` | Validate visually; likely replace with quieter selected-state treatment. | `Enhancement` | Phase 12 |
| Pump runtime thick left border | `style.css:532` | Repeated one-sided card accent conflicts with the approved product guidance. | `Should fix` | Phase 12 |
| Cooling runtime thick left border | `style.css:767` | Same systemic side-accent issue. | `Should fix` | Phase 12 |
| Hardware status thick left border | `style.css:819` | Same systemic side-accent issue on installer feedback. | `Should fix` | Phase 13 |
| Progress fill width transition | `style.css:470` | Bounded progress update; keep only if later capture confirms no visible jank and reduced motion is handled. | `Enhancement` | Phase 13 |
| Wi-Fi flat type hierarchy warning | `wifi.html:59` | Validate visually at both target widths; preserve compactness if scanability is adequate. | `Enhancement` | Phase 13 |

## Actionable Findings

| ID | Priority | Surface | Problem | Evidence source | User impact | Recommendation | Owner |
| --- | --- | --- | --- | --- | --- | --- | --- |
| F-01 | `Must fix` | Shared dynamic feedback | Dynamic status and error messages have no live-region or alert semantics. | Source search: no `aria-live`, `role="status"`, or `role="alert"` under `main/static/`. | Assistive-technology users may miss login failures, stale device state, save results, Wi-Fi feedback, and toasts. | Add `aria-live="polite"` for informational and success updates and `role="alert"` for immediate errors. | Phase 12 prerequisite, Phase 13 surface completion |
| F-02 | `Must fix` | Shared keyboard interaction | Focus treatment is limited to selected form fields, with no shared `:focus-visible` rule. | `style.css` focus selectors and source search. | Keyboard users can lose their position on links, buttons, navigation, checkboxes, Wi-Fi scan, and network selection controls. | Add a clear `2-4px` blue focus ring for every interactive control. | Phase 12 prerequisite, Phase 13 surface completion |
| F-03 | `Must fix` | App Shell | Mobile navigation is inconsistent and icon/back-arrow controls lack accessible names. | Protected page mobile topbars. | Phone and assistive-technology users cannot reliably move between authenticated surfaces. | Provide one consistent mobile navigation pattern and accessible control names. | Phase 12 |
| F-04 | `Should fix` | Wi-Fi | Selectable network rows use click handlers without a complete keyboard or selected-state contract. | Generated `.network-item` markup in `app.js`. | Keyboard users depend on nested buttons and cannot reliably understand row selection or disabled connected rows. | Normalize row semantics, selected state, disabled state, and keyboard operation. | Phase 13 |
| F-05 | `Should fix` | Shared mobile controls | `.btn-sm` and `.scan-btn` are compact and may fall below `44x44px`. | `style.css`, generated network select action. | Phone users near the device may miss taps during setup or diagnosis. | Enforce a `44x44px` minimum interactive area while retaining compact visual styling. | Phase 13 |
| F-06 | `Should fix` | Shared motion | Pulse, toast, spinner, and progress transitions have no reduced-motion override. | `style.css`; no `prefers-reduced-motion` result. | Motion-sensitive users cannot request a quieter UI. | Add a reduced-motion media query that removes nonessential motion and shortens state transitions. | Phase 13 |
| F-07 | `Should fix` | Dashboard | Runtime, configuration, and quick diagnostics remain dense and close in visual weight. | `dashboard.html`, source hierarchy inspection. | Owners may need too much scanning before confirming pump safety. | Make pump state and daily controls dominant; keep settings and diagnostics secondary. | Phase 12 |
| F-08 | `Should fix` | Hardware/Install | Active, pending, and reboot-required information exists but narrow-width legibility is unverified. | `hardware.html`, runtime capture `not-run`. | Installers could wire against saved values that are not active yet. | Preserve separate active and pending values, strengthen reboot-required messaging, and verify at `375px`. | Phase 13 |
| F-09 | `Should fix` | Wi-Fi | Scan/connect feedback exists but is inconsistent and some error text lacks a specific recovery action. | `doScan()`, `doConnect()`, `pollWifiConnection()`. | Setup users may not know whether to retry, check password, select a network, or rely on SoftAP fallback. | Apply canonical loading, empty, disconnected, success, and recovery-oriented error copy. | Phase 13 |
| F-10 | `Enhancement` | Shared visual language | Four one-sided colored borders create repeated accent stripes. | Impeccable detector output. | The UI looks less restrained and panels compete unnecessarily for attention. | Use quieter selected-state and semantic surface treatments. | Phase 12 and Phase 13 |
| F-11 | `Enhancement` | Status | Long technical values are source-present but narrow-width wrapping is unverified. | `status.html`, runtime capture `not-run`. | Mobile diagnostics may become difficult to scan if values overlap or overflow. | Verify long SSID, IP, MAC, firmware, and diagnostic labels at `375px`; add wrapping where needed. | Phase 13 |
| F-12 | `Enhancement` | Wi-Fi typography | Closely spaced type sizes may flatten hierarchy. | Impeccable detector output for `wifi.html:59`. | Scan results and recovery guidance may take longer to parse. | Validate visually and adjust only if scanability is weak. | Phase 13 |
| F-13 | `Enhancement` | Progress motion | Progress fill animates `width`. | Impeccable detector output for `style.css:470`. | On a constrained device this can cause avoidable layout work. | Keep only if later capture remains smooth; otherwise replace with a transform-based fill. | Phase 13 |

## Page-By-Page Runtime Checks

| Surface | Check | Status | Reason |
| --- | --- | --- | --- |
| Login | Default, loading, and auth-error screenshots at `1440px` and `375px` | `not-run` | Browser setup failed twice before navigation. |
| App Shell | Desktop sidebar and mobile navigation screenshots | `simulated` static baseline | Chrome-default desktop sidebar and `375x812` mobile topbars are captured on protected pages. |
| Dashboard | Pump loading, unavailable, running, stopped, and cooling sensor-fault screenshots | `simulated` static baseline; dynamic states `not-run` | Chrome-default desktop and `375x812` mobile static layouts are captured. Dynamic states require API-backed fixtures or a device. |
| Hardware/Install | Active map, pending map, pending reboot, save-success, and save-error screenshots | `simulated` static baseline; dynamic states `not-run` | Chrome-default desktop and `375x812` mobile static layouts are captured. Dynamic states require API-backed fixtures or a device. |
| Status | Desktop and narrow diagnostics with long technical values | `simulated` static baseline; long values `not-run` | Chrome-default desktop and `375x812` mobile placeholder layouts are captured. Representative long values require API-backed fixtures or a device. |
| Wi-Fi | Disconnected, scanning, empty, scan-error, selected, connecting, success, connect-error, and disconnect screenshots | `simulated` static baseline; dynamic states `not-run` | Chrome-default desktop and `375x812` mobile disconnected layouts are captured. Remaining states require API-backed or controlled fixtures. |

## Positive Findings To Preserve

- Embedded assets total `159,436` bytes and remain fully local.
- The UI uses system fonts, semantic CSS variables, compact radii, and a
  restrained operational palette.
- Login labels and autocomplete attributes are present.
- Pump and cooling runtime channels are already separated.
- Hardware/Install already distinguishes active and pending GPIO maps and
  requires reboot confirmation.
- Wi-Fi already implements the important operational state paths in source.
- Status diagnostics are grouped into understandable system categories.

## Phase 12 And Phase 13 Direction

Phase 12 should own Login, App Shell, Dashboard hierarchy, and shared
accessibility primitives needed by all pages. Phase 13 should finish those
primitives across Hardware/Install, Wi-Fi, and Status while improving setup
states, touch targets, reduced motion, and narrow-width presentation.

Phase 14 must perform the missing device-backed or simulated screenshot pass at
`1440px` and `375px`, plus build and footprint regression validation.
