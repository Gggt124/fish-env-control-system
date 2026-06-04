# Phase 12 Implementation Brief: App Shell, Login, And Owner Dashboard

## Objective

Improve Login, authenticated navigation, and daily owner confidence while
preserving the stable v1.1 pump, cooling, Wi-Fi, and APSTA behavior. Phase 12
implements shared accessibility primitives early so Phase 13 can apply them to
Hardware/Install, Wi-Fi, and Status without rediscovery.

## Guardrails

- Keep the existing embedded plain HTML, CSS, and JavaScript stack.
- Keep all frontend assets local and offline-capable. Add no CDN dependency,
  remote font, remote icon package, remote image, or framework runtime.
- Keep Thai-first user-facing copy with precise technical nouns where useful.
- Keep static assets near the `159,436`-byte audit reference and compare the
  Phase 14 firmware build to the `1,111,616`-byte reference. These are
  references, not hard budgets.
- Preserve the Phase 11 boundary: Phase 11 changed no file under `main/static/`.
- Preserve stable v1.1 runtime behavior. Change presentation and state
  semantics, not relay, timer, cooling, Wi-Fi, or APSTA logic.

## Required Shared Primitives

- Add a clear `2-4px` blue `:focus-visible` ring for every interactive control.
- Add `aria-live="polite"` to informational, loading, unavailable, empty-result,
  and success regions where content changes dynamically.
- Add `role="alert"` to errors requiring immediate action.
- Keep safety, recovery, and pending-reboot messages contextual; use toast only
  as an additional short confirmation.
- Use the canonical vocabulary in `11-STATE-LANGUAGE.md`.
- Keep mobile interactive targets at least `44x44px`.

## Findings

### F-01: Shared dynamic feedback is not announced

- **Problem:** Dynamic status and error messages have no live-region or alert
  semantics.
- **Evidence:** Source search in `11-BASELINE-UI-AUDIT.md`; no `aria-live`,
  `role="status"`, or `role="alert"` exists under `main/static/`.
- **User impact:** Assistive-technology users may miss login failures, stale
  pump state, save results, and toasts.
- **Owner:** Phase 12 shared prerequisite; Phase 13 completes remaining
  surfaces.
- **Priority:** `Must fix`
- **Target files:** `main/static/login.html`, `main/static/dashboard.html`,
  `main/static/style.css`, `main/static/app.js`
- **Acceptance criteria:** Login and Dashboard informational or success updates
  use `aria-live="polite"`; immediate errors use `role="alert"`; durable safety
  messages remain visible near the initiating area.
- **Verification:** Search markup for live regions and alerts, keyboard-test
  Login and Dashboard, and screenshot default/loading/error states at `1440px`
  and `375px`.

### F-02: Shared keyboard focus treatment is incomplete

- **Problem:** Existing focus styling covers selected form fields but not every
  interactive control.
- **Evidence:** `main/static/style.css` focus selectors and source search in the
  baseline audit.
- **User impact:** Keyboard users can lose their position while navigating
  links, buttons, logout, topbar actions, and configuration controls.
- **Owner:** Phase 12 shared prerequisite; Phase 13 completes remaining
  surfaces.
- **Priority:** `Must fix`
- **Target files:** `main/static/style.css`, `main/static/login.html`,
  `main/static/dashboard.html`
- **Acceptance criteria:** Buttons, links, inputs, selects, checkboxes,
  navigation items, and icon-only controls receive a visible `2-4px` blue
  `:focus-visible` ring without clipping.
- **Verification:** Tab through Login and Dashboard at desktop and mobile
  widths; confirm every focus stop is visible and understandable.

### F-03: Mobile app-shell navigation is inconsistent

- **Problem:** Protected pages expose different mobile topbar controls and rely
  on Unicode glyph recognition without accessible names.
- **Evidence:** Protected page `.mobile-topbar` markup in the baseline audit.
- **User impact:** Phone and assistive-technology users cannot reliably move
  between Dashboard, Hardware/Install, Wi-Fi, Status, and logout.
- **Owner:** Phase 12
- **Priority:** `Must fix`
- **Target files:** `main/static/dashboard.html`, `main/static/hardware.html`,
  `main/static/wifi.html`, `main/static/status.html`, `main/static/style.css`,
  `main/static/app.js`
- **Acceptance criteria:** Every authenticated page offers a consistent mobile
  navigation pattern covering Dashboard, Hardware/Install, Wi-Fi, Status, and
  logout; icon-only controls have accessible names or visible labels.
- **Verification:** Tab and touch-test all protected pages at `375px`; capture
  the navigation state on each page.

### F-07: Dashboard operational hierarchy is too dense

- **Problem:** Pump runtime, cooling runtime, configuration, and diagnostics are
  present but remain visually close in weight.
- **Evidence:** `main/static/dashboard.html` and source hierarchy review in the
  baseline audit.
- **User impact:** The owner may need too much scanning before confirming pump
  safety.
- **Owner:** Phase 12
- **Priority:** `Should fix`
- **Target files:** `main/static/dashboard.html`, `main/static/style.css`,
  `main/static/app.js`
- **Acceptance criteria:** Pump running state, selected timer and relay, phase,
  countdown, and float switch are recognizable within seconds; cooling remains
  a separate operational channel; configuration and quick diagnostics are
  visually secondary.
- **Verification:** Capture Dashboard loading, unavailable, running, stopped,
  and cooling sensor-fault states at `1440px` and `375px`; confirm controls
  remain reachable.

### F-10: One-sided accent borders compete with hierarchy

- **Problem:** Active navigation, pump runtime, and cooling runtime use thick
  one-sided accent borders.
- **Evidence:** Impeccable detector warnings at `style.css:318`,
  `style.css:532`, and `style.css:767`.
- **User impact:** Panels compete visually and the interface feels less
  restrained than the product register requires.
- **Owner:** Phase 12 for nav, pump, and cooling; Phase 13 for Hardware/Install.
- **Priority:** `Enhancement`
- **Target files:** `main/static/style.css`
- **Acceptance criteria:** Selected navigation and runtime emphasis use quieter
  semantic surface, border, or text treatments without a thick one-sided
  accent stripe.
- **Verification:** Re-run the Impeccable detector and compare desktop/mobile
  screenshots for hierarchy clarity.

## Surface Acceptance

### Login

- Credential purpose, visible labels, loading state, authentication error, and
  recovery instruction are immediately clear.
- Errors are contextual and announced.
- Mobile layout remains usable at `375px`.

### App Shell

- Desktop sidebar and mobile navigation expose the same destinations.
- Logout remains reachable and clearly named.
- Icon-only controls include accessible names.

### Dashboard

- Pump safety state is the first operational focal point.
- Cooling remains separate from pump relays.
- Loading, success, error, disabled, and unavailable treatments follow the
  canonical vocabulary.

## Checklist Adaptation

| Keep | Adapt For ESP32 | Exclude |
| --- | --- | --- |
| Semantic CSS variables, system fonts, familiar controls, visible labels, state text, responsive shell | Use Unicode icons with accessible names, lightweight state feedback, Thai-first copy, `44x44px` targets, and byte tracking | CDN assets, remote fonts, remote icon packages, framework migration, decorative motion, marketing layout |

## Deferred And Not-Run Items

- Browser screenshots remain `not-run` because the Phase 11 browser connection
  failed twice during setup. Phase 12 implementation must capture reachable
  Login, shell, and Dashboard states when browser automation works.
- Device-backed screenshots remain `not-run` until a reachable ESP32 URL is
  available.
- Hardware/Install, Wi-Fi, Status, reduced-motion completion, and detailed
  touch-target normalization remain visible in the Phase 13 brief.
