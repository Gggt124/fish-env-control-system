# Phase 13 Implementation Brief: Hardware Install, Wi-Fi Setup, And Status UX

## Objective

Improve installer, setup, and diagnostics workflows across their operational
states while preserving safe hardware-map behavior and stable v1.1 Wi-Fi/APSTA
runtime behavior. Apply the shared state and accessibility primitives from
Phase 12 to Hardware/Install, Wi-Fi, and Status.

## Guardrails

- Keep the embedded plain HTML, CSS, and JavaScript stack.
- Keep all assets local and offline-capable. Add no CDN dependency, remote
  font, remote icon package, remote image, or framework runtime.
- Keep Thai-first recovery copy and precise technical nouns.
- Keep the external `4.7 kOhm` DS18B20 DQ-to-`3.3 V` pull-up guidance visible.
- Preserve safe GPIO dropdowns, reboot confirmation, and separate active and
  pending maps.
- Preserve the `159,436`-byte static asset and `1,111,616`-byte firmware audit
  references as comparison values, not hard budgets.
- Preserve the Phase 11 no-edit boundary for `main/static/`.

## Findings

### F-01: Complete shared dynamic feedback semantics

- **Problem:** Hardware/Install, Wi-Fi, Status, and shared toasts need the
  live-region and alert primitives established in Phase 12.
- **Evidence:** Baseline source search: no `aria-live`, `role="status"`, or
  `role="alert"` under `main/static/`.
- **User impact:** Assistive-technology users may miss scan results, connection
  feedback, GPIO save results, and durable reboot state.
- **Owner:** Phase 13 surface completion
- **Priority:** `Must fix`
- **Target files:** `main/static/hardware.html`, `main/static/wifi.html`,
  `main/static/status.html`, `main/static/app.js`
- **Acceptance criteria:** Dynamic informational and success messages announce
  politely; immediate errors use alerts; durable pending-reboot text stays
  visible and recognizable.
- **Verification:** Search markup and exercise reachable dynamic states with
  assistive semantics inspection.

### F-02: Complete shared focus treatment on setup surfaces

- **Problem:** Setup controls and generated network rows need the visible focus
  primitive established in Phase 12.
- **Evidence:** Partial focus selectors in `main/static/style.css`.
- **User impact:** Keyboard users can lose position in dense installer and
  network setup flows.
- **Owner:** Phase 13 surface completion
- **Priority:** `Must fix`
- **Target files:** `main/static/style.css`, `main/static/hardware.html`,
  `main/static/wifi.html`, `main/static/status.html`, `main/static/app.js`
- **Acceptance criteria:** Every interactive setup control receives a visible
  `2-4px` blue `:focus-visible` ring without clipping.
- **Verification:** Tab through Hardware/Install, Wi-Fi, and Status at `1440px`
  and `375px`.

### F-04: Wi-Fi selection rows lack a complete keyboard contract

- **Problem:** Generated `.network-item` rows rely on click handlers and do not
  expose a complete selected and disabled interaction model.
- **Evidence:** Generated Wi-Fi markup in `main/static/app.js`.
- **User impact:** Keyboard users may not understand which network is selected
  or whether a connected row can be activated.
- **Owner:** Phase 13
- **Priority:** `Should fix`
- **Target files:** `main/static/app.js`, `main/static/style.css`,
  `main/static/wifi.html`
- **Acceptance criteria:** Wi-Fi rows are keyboard operable; connected rows
  expose disabled semantics; selected rows expose recognizable selected state;
  nested action behavior is unambiguous.
- **Verification:** Run scan with controlled or device-backed networks, tab
  through rows, select one, and inspect selected/disabled semantics.

### F-05: Compact controls need `44x44px` mobile targets

- **Problem:** `.btn-sm`, `.scan-btn`, generated Wi-Fi selection actions,
  topbar controls, and checkboxes may be too small for reliable mobile use.
- **Evidence:** Compact CSS rules and source inventory in the baseline audit.
- **User impact:** Installer taps may miss while working near the board.
- **Owner:** Phase 13
- **Priority:** `Should fix`
- **Target files:** `main/static/style.css`, `main/static/hardware.html`,
  `main/static/wifi.html`, `main/static/status.html`
- **Acceptance criteria:** Mobile interactive areas are at least `44x44px`
  while retaining compact visual styling.
- **Verification:** Inspect representative controls at `375px`; capture Wi-Fi,
  Hardware/Install, and Status screenshots.

### F-06: Reduced motion is missing

- **Problem:** Pulse, toast, spinner, and progress transitions have no
  reduced-motion override.
- **Evidence:** `main/static/style.css`; no `prefers-reduced-motion` rule.
- **User impact:** Motion-sensitive users cannot request a quieter interface.
- **Owner:** Phase 13
- **Priority:** `Should fix`
- **Target files:** `main/static/style.css`
- **Acceptance criteria:** `prefers-reduced-motion: reduce` removes
  nonessential motion and shortens necessary feedback transitions.
- **Verification:** Inspect computed behavior under reduced-motion preference
  and capture representative states.

### F-08: Hardware active, pending, and reboot state need mobile proof

- **Problem:** The source contains active and pending maps plus reboot wording,
  but narrow-width legibility and emphasis are unverified.
- **Evidence:** `main/static/hardware.html`, hardware renderers, and Phase 11
  runtime screenshot status `not-run`.
- **User impact:** Installers could wire against saved GPIO values before they
  become active.
- **Owner:** Phase 13
- **Priority:** `Should fix`
- **Target files:** `main/static/hardware.html`, `main/static/style.css`,
  `main/static/app.js`, hardware-readiness documentation
- **Acceptance criteria:** Active and pending GPIO values stay separate;
  reboot-required wording is durable; save requires the reboot confirmation;
  external `4.7 kOhm` DS18B20 pull-up guidance is explicit and readable at
  `375px`.
- **Verification:** Capture active map, pending map, pending reboot, save
  success, and save error states at `1440px` and `375px`.

### F-09: Wi-Fi setup feedback is inconsistent

- **Problem:** Scan, connect, and disconnect states exist but copy and recovery
  guidance vary by path.
- **Evidence:** `doScan()`, `doConnect()`, `pollWifiConnection()`, and
  `doDisconnect()` in `main/static/app.js`.
- **User impact:** Setup users may not know whether to retry, check password,
  select a network, or rely on SoftAP fallback.
- **Owner:** Phase 13
- **Priority:** `Should fix`
- **Target files:** `main/static/wifi.html`, `main/static/app.js`,
  `main/static/style.css`
- **Acceptance criteria:** Wi-Fi clearly presents loading, empty, disconnected,
  connected, selection-required, connecting, success, scan error, connect
  error, and disconnect feedback with a next action; SoftAP fallback stays
  visible.
- **Verification:** Capture each reachable Wi-Fi state at `1440px` and `375px`
  using controlled fixtures or device-backed APIs.

### F-10: Complete side-accent cleanup on Hardware/Install

- **Problem:** Hardware status uses a thick one-sided accent border.
- **Evidence:** Impeccable detector warning at `style.css:819`.
- **User impact:** Installer feedback competes with more important active
  versus pending information.
- **Owner:** Phase 13
- **Priority:** `Enhancement`
- **Target files:** `main/static/style.css`
- **Acceptance criteria:** Hardware emphasis uses a quieter semantic treatment
  without a thick one-sided stripe.
- **Verification:** Re-run detector and compare screenshots.

### F-11: Status long values need narrow-width wrapping proof

- **Problem:** Long SSID, IP, MAC, firmware, and diagnostic strings are
  source-present but unverified at `375px`.
- **Evidence:** `main/static/status.html`; Phase 11 runtime screenshot status
  `not-run`.
- **User impact:** Mobile diagnostics may overlap or become hard to scan.
- **Owner:** Phase 13
- **Priority:** `Enhancement`
- **Target files:** `main/static/status.html`, `main/static/style.css`
- **Acceptance criteria:** Long values wrap or remain readable without hiding
  labels or creating horizontal overflow.
- **Verification:** Capture representative long values at `375px`.

### F-12: Wi-Fi typography hierarchy needs visual validation

- **Problem:** Wi-Fi uses closely spaced type sizes that may flatten scan
  hierarchy.
- **Evidence:** Impeccable detector warning for `main/static/wifi.html:59`.
- **User impact:** Users may take longer to distinguish network names, metadata,
  status, and actions.
- **Owner:** Phase 13
- **Priority:** `Enhancement`
- **Target files:** `main/static/wifi.html`, `main/static/style.css`
- **Acceptance criteria:** Network name, metadata, status, and action hierarchy
  remains compact but immediately scannable.
- **Verification:** Compare Wi-Fi list screenshots at `1440px` and `375px`.

### F-13: Progress animation should remain lightweight

- **Problem:** Progress fill animates the layout property `width`.
- **Evidence:** Impeccable detector warning at `style.css:470`.
- **User impact:** Constrained devices may perform unnecessary layout work.
- **Owner:** Phase 13
- **Priority:** `Enhancement`
- **Target files:** `main/static/style.css`, `main/static/app.js`
- **Acceptance criteria:** Retain the bounded transition only if capture is
  smooth and reduced motion is handled; otherwise use a transform-based fill.
- **Verification:** Review animation under normal and reduced-motion settings.

## Surface Acceptance

### Hardware/Install

- Active GPIO, pending GPIO, reboot requirement, safe dropdown options,
  confirmation checkbox, and the external DS18B20 pull-up are clear at desktop
  and mobile widths.

### Wi-Fi

- Loading, empty, disconnected, connected, error, selection, connect, and
  disconnect states are explicit without changing APSTA behavior.

### Status

- System, memory, uptime, STA, AP, and service diagnostics remain scannable at
  `1440px` and `375px`.

## Checklist Adaptation

| Keep | Adapt For ESP32 | Exclude |
| --- | --- | --- |
| Native controls, semantic tokens, local system fonts, clear state text, responsive layout | Use lightweight spinner and transitions, Unicode icons with names, Thai-first recovery copy, `44x44px` mobile targets, static-byte and firmware-size tracking | CDN assets, remote fonts, remote icon packages, framework migration, heavy animation libraries, dark-mode expansion, chart packages |

## Deferred And Not-Run Items

- Browser screenshots and device-backed state evidence remain `not-run` from
  Phase 11. Phase 13 must capture reachable setup states when automation or a
  test device is available.
- Full visual regression, final Impeccable audit/polish, ESP-IDF build,
  partition-footprint comparison, and stable v1.1 hardware regression remain
  deferred to Phase 14.
