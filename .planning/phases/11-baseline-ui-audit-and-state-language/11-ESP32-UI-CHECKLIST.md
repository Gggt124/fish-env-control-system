# Phase 11 ESP32 UI Checklist

## Use

Apply this checklist during Phase 12 and Phase 13 implementation and repeat it
during Phase 14 validation. It adapts product-dashboard guidance to the
existing ESP32 local web stack: plain HTML, CSS, and JavaScript embedded in
firmware.

## Baseline References

| Metric | Baseline | Meaning |
| --- | ---: | --- |
| Embedded static assets | `159,436` bytes | Audit reference from the current seven files under `main/static/`, not a hard budget. |
| Latest firmware binary | `1,111,616` bytes | Audit reference from `build/fish_pump_relay_timer_control.bin`, not a hard budget. |

Track changes against these references after frontend edits. Phase 14 owns the
full ESP-IDF build and partition-footprint comparison.

## Keep

| Area | Check |
| --- | --- |
| Product register | Keep the calm, clear, trustworthy local-control-panel voice. |
| Stack | Keep embedded plain HTML, CSS, and vanilla JavaScript. |
| Assets | Keep local system fonts and local Unicode icons with text or accessible-name fallback. |
| Offline operation | Confirm the UI works with no internet and uses no CDN, remote font, remote image, remote icon package, or remote script. |
| Tokens | Reuse the `DESIGN.md` CSS variables and semantic palette. |
| Semantics | Preserve semantic HTML landmarks, visible form labels, tables for diagnostics, and native controls where practical. |
| Safety | Keep SoftAP fallback wording, pump safety states, cooling sensor-fault handling, and active-versus-pending GPIO separation. |

## Adapt For ESP32

| Generic guidance | ESP32 adaptation |
| --- | --- |
| Loading indicators | Use short action-specific text and a lightweight spinner only where useful. Do not add a package. |
| Icons | Use accepted local Unicode icons and accessible text or names instead of remote icon libraries. |
| Motion | Keep state-conveying motion short and lightweight; add `prefers-reduced-motion: reduce`. |
| Performance | Track embedded static bytes and firmware binary size instead of web bundle metrics alone. |
| Responsive review | Review desktop `1440px` and narrow mobile `375px`; prioritize phone use near the board. |
| Error recovery | Keep Thai-first recovery copy concise enough for a small TFT vocabulary. |
| Empty states | Explain the result and next safe action without illustrations or large assets. |
| Focus | Use a clear `2-4px` blue `:focus-visible` ring with enough contrast against every surface. |

## Exclude

| Recommendation | Reason |
| --- | --- |
| CDN assets, Google Fonts, remote icon packages, or remote images | SoftAP and local-network operation may have no internet access. |
| Frontend framework migration or dependency-managed component library | The current embedded HTML/CSS/JS stack is intentionally small and stable. |
| Heavy animation library, decorative motion, or page-load choreography | Adds footprint and distracts from appliance operation. |
| Dark-mode expansion | Not required for this milestone and would widen visual regression scope. |
| Chart package, analytics surface, or image optimization work | No chart or image requirement exists in v1.2. |
| Decorative gradients, glass effects, or marketing hero layout | Conflicts with the operational product register. |

## Accessibility

- [ ] Every interactive control has visible keyboard focus using a clear
  `2-4px` blue `:focus-visible` ring.
- [ ] Links, buttons, inputs, selects, checkboxes, navigation items, Wi-Fi scan
  controls, and selectable network rows are included in the focus audit.
- [ ] Every form control has a visible label and concise helper instruction
  where the action is not obvious.
- [ ] Informational, loading, empty-result, unavailable, and success updates use
  `aria-live="polite"` where populated dynamically.
- [ ] Errors requiring immediate user action use `role="alert"`.
- [ ] State is never communicated by color alone.
- [ ] Icon-only mobile controls have an accessible name or visible text.
- [ ] Selectable Wi-Fi rows expose keyboard-operable selected and disabled
  semantics.

## Responsive And Touch

- [ ] Review every shipped page at desktop `1440px`.
- [ ] Review every shipped page at mobile `375px`.
- [ ] Keep mobile interactive areas at least `44x44px`.
- [ ] Check `.btn-sm`, `.scan-btn`, generated Wi-Fi selection actions, topbar
  controls, checkboxes, and links.
- [ ] Keep primary actions reachable without horizontal scrolling.
- [ ] Verify long SSIDs, IP addresses, MAC addresses, firmware strings, GPIO
  labels, and Thai status copy wrap without overlap.
- [ ] Verify active GPIO, pending GPIO, and reboot-required guidance remain
  legible in one-column mobile flow.

## State Feedback

- [ ] Use the eight canonical states from `11-STATE-LANGUAGE.md`: `loading`,
  `success`, `error`, `empty`, `disabled`, `unavailable`, `pending reboot`,
  and `disconnected`.
- [ ] Distinguish `processing`, reasoned `unavailable`, and `pending reboot`
  instead of dimming controls without explanation.
- [ ] Keep safety and recovery messages contextual; use toast only as an
  additional short confirmation.
- [ ] Confirm Wi-Fi scan/connect covers loading, empty, disconnected, connected,
  error, and retry guidance.
- [ ] Confirm Dashboard and Hardware/Install cover loading, success, error,
  disabled, unavailable, and pending reboot where applicable.

## Hierarchy And Copy

- [ ] Keep pump running state, selected timer and relay, phase, countdown, and
  float switch recognizable within seconds.
- [ ] Keep cooling temperature, mode, relay state, and sensor fault visibly
  separate from the pump relays.
- [ ] Keep daily operation controls visually distinct from configuration and
  secondary diagnostics.
- [ ] Keep user-facing state copy Thai-first with precise English technical
  nouns where useful.
- [ ] Use verb-plus-object action labels rather than generic `Save`, `OK`, or
  `Submit`.

## Motion

- [ ] Add `prefers-reduced-motion: reduce` handling for pulse, toast slide-in,
  spinner, and progress transitions.
- [ ] Keep motion tied to state change only.
- [ ] Review the bounded progress `width` transition; retain it only if visual
  verification confirms smooth behavior, otherwise use a transform-based fill.

## Offline And Footprint

- [ ] Run:

```powershell
rg -n "https?://|cdn|fonts\.googleapis|unpkg|jsdelivr" main/static
Get-ChildItem -File -LiteralPath "main\static" | Measure-Object -Property Length -Sum
```

- [ ] Confirm dependency search is clean or document every intentional result.
- [ ] Compare static bytes to the `159,436`-byte audit reference.
- [ ] During Phase 14, run `.\scripts\build.ps1` and compare firmware binary
  size to the `1,111,616`-byte audit reference.
- [ ] Treat both values as references, not hard budgets. Investigate growth
  that is large enough to threaten the app partition or indicate unsuitable
  assets.

## Phase 14 Evidence

- [ ] Capture privacy-reviewed screenshots for every affected page at `1440px`
  and `375px`.
- [ ] Capture representative loading, error, empty, disabled, disconnected,
  connected, sensor-fault, and pending-reboot states where safe.
- [ ] Label each screenshot `device-backed`, `simulated`, or `not-run`.
- [ ] Record not-run reasons instead of synthesizing visual results.
