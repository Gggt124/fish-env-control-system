# Phase 11: Baseline UI Audit And State Language - Context

**Gathered:** 2026-06-02
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 11 audits the shipped local web UI before implementation changes begin, records evidence-backed findings, and defines the shared product-dashboard state language for the Phase 12-13 improvement pass. The audit covers Login, App Shell, Dashboard, Hardware/Install, Wi-Fi, and Status surfaces at desktop and narrow mobile widths. It also establishes accessibility and ESP32-footprint expectations for later UI work.

This phase produces audit evidence and implementation briefs. It does not redesign the UI, change stable v1.1 relay/timer/cooling behavior, rewrite Wi-Fi/APSTA behavior, add remote frontend dependencies, or implement TFT support.

</domain>

<decisions>
## Implementation Decisions

### Audit Evidence Pack
- **D-01:** Inspect every shipped UI surface: Login, App Shell, Dashboard, Hardware/Install, Wi-Fi, and Status.
- **D-02:** Use the Browser plugin with the `impeccable` audit workflow to inspect the local web app automatically where practical.
- **D-03:** Capture baseline screenshots at desktop width `1440px` and responsive mobile width `375px`.
- **D-04:** Capture representative loading, error, empty, disabled, and pending-reboot states where they can be simulated safely.
- **D-05:** Store a Markdown audit report plus a screenshot folder organized by page and viewport.
- **D-06:** Prioritize findings with three levels: `Must fix`, `Should fix`, and `Enhancement`.

### Shared State Language
- **D-07:** Keep all frontend assets local and embedded. Do not add CDN dependencies, frontend frameworks, remote fonts, remote icon packages, large images, or heavy animations.
- **D-08:** Validate embedded frontend asset size and app-partition footprint during the milestone.
- **D-09:** Use concise action-specific loading text, temporarily disable the relevant control, and show a small spinner for in-progress operations.
- **D-10:** Show successful actions with contextual text close to the initiating control and a short toast where useful.
- **D-11:** Show errors near the relevant control or status area, include a brief recovery instruction, and announce the error programmatically.
- **D-12:** Distinguish three unavailable states explicitly: `processing`, `unavailable` with a reason, and `pending reboot` before new values become active.
- **D-13:** Keep user-facing state wording short and independent of the web layout so the same concepts can be reused by a future small TFT interface.

### Accessibility Contract
- **D-14:** Show a clear `2-4px` blue focus ring on every interactive control, including buttons, links, inputs, selects, checkboxes, navigation, and selectable network rows.
- **D-15:** Use `aria-live="polite"` for informational and success updates. Use `role="alert"` for errors requiring immediate action.
- **D-16:** Keep mobile interactive areas at least `44x44px`.
- **D-17:** Use visible form labels and concise helper instructions where needed. Prefer short Thai user-facing text while retaining precise technical terms such as `GPIO`, `STA`, `AP`, and `IP` where useful.

### Phase 12-13 Handoff Briefs
- **D-18:** Organize findings by page and flow: Login, App Shell, Dashboard, Hardware/Install, Wi-Fi, and Status.
- **D-19:** Mark whether each finding belongs to Phase 12 or Phase 13.
- **D-20:** Record the problem, screenshot evidence, user impact, and acceptance criteria for each finding.
- **D-21:** Create separate implementation briefs: Phase 12 receives Login, App Shell, and Dashboard; Phase 13 receives Hardware/Install, Wi-Fi, Status, and applicable state gaps.
- **D-22:** Adapt the `ui-ux-pro-max` checklist to ESP32 constraints and record the rationale. Keep accessibility, responsive behavior, state feedback, touch targets, contrast, and performance-footprint checks. Exclude CDN assets, remote fonts, icon packages, framework migration, and heavy animation.

### the agent's Discretion
Planner may choose the audit report filename, screenshot subdirectory naming, browser automation script structure, exact Thai microcopy inventory format, and brief filenames as long as the locked audit coverage, state language, accessibility contract, and ESP32 constraints above are preserved.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Scope And Requirements
- `.planning/PROJECT.md` - v1.2 UI-polish goal, stable v1.1 baseline, offline constraints, and hardware-readiness context.
- `.planning/REQUIREMENTS.md` - Phase 11 requirements `UI-13`, `A11Y-02`, and `A11Y-03`, plus Phase 12-14 downstream requirements.
- `.planning/ROADMAP.md` - Phase 11 boundary, success criteria, and Phase 12-14 handoff scope.
- `PRODUCT.md` - Product register, owner and installer needs, anti-references, state clarity principles, and accessibility expectations.
- `DESIGN.md` - Existing embedded UI tokens, layout rules, semantic palette, responsive behavior, interaction states, and no-CDN design contract.

### Codebase Maps
- `.planning/codebase/CONVENTIONS.md` - Framework-free JavaScript, Thai-first UI text, HTTP patterns, and frontend conventions.
- `.planning/codebase/STRUCTURE.md` - Static UI files and extension points in `main/static/`.
- `.planning/codebase/STACK.md` - ESP-IDF embedded frontend stack, offline dependency constraints, and build workflow.

### Existing UI Surfaces
- `main/static/login.html` - Login surface to audit for purpose, labels, loading, error recovery, focus, and mobile layout.
- `main/static/dashboard.html` - Owner control surface to audit for hierarchy, pump/cooling separation, state clarity, and mobile layout.
- `main/static/hardware.html` - Installer surface to audit for active/pending GPIO clarity, reboot-required state, and narrow-mobile usability.
- `main/static/wifi.html` - Wi-Fi setup surface to audit for scan, connect, disconnected, loading, error, empty, and disabled states.
- `main/static/status.html` - Diagnostics surface to audit for scanability and responsive behavior.
- `main/static/style.css` - Existing CSS token implementation, responsive rules, focus treatment, status styles, and embedded asset footprint.
- `main/static/app.js` - Existing API calls, polling, action feedback, dynamic status messages, Wi-Fi states, and DOM updates.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `PRODUCT.md` and `DESIGN.md`: committed product-dashboard register and visual tokens; Phase 11 should audit against these rather than inventing a new design direction.
- `main/static/style.css`: existing CSS custom properties, semantic colors, responsive shell, button styles, toasts, spinners, cards, and form patterns.
- `main/static/app.js`: centralized vanilla-JS API calls and page initializers; it already contains loading, success, error, disabled, disconnected, and pending-reboot behavior that should be inventoried and normalized.
- Browser plugin and `impeccable`: available local UI inspection workflow for desktop/mobile screenshots and evidence-backed critique.

### Established Patterns
- The web UI is embedded static HTML/CSS/JS with no CDN, remote fonts, icon packages, or frontend framework.
- User-visible copy is mostly Thai with retained English technical nouns where useful.
- Desktop uses a sidebar and mobile uses a compact top bar.
- Actions commonly disable controls and change button text while requests are in flight.
- Toasts exist, but durable contextual messages and assistive announcements must be audited consistently.

### Integration Points
- Baseline screenshots and audit findings should cover each static page and the shared shell before Phase 12-13 implementation begins.
- State-language findings should map back to dynamic DOM updates in `main/static/app.js` and shared selectors in `main/static/style.css`.
- Accessibility findings should identify controls missing visible focus, labels, minimum touch area, and live-region semantics.
- Footprint validation should use the existing ESP-IDF build flow after later embedded asset changes.

</code_context>

<specifics>
## Specific Ideas

- The controller may be used through SoftAP with no internet or through a local network without internet access. Local embedded assets are mandatory, not a fallback.
- The audit report should be practical implementation input: each finding includes evidence, impact, acceptance criteria, and its target phase.
- The shared state vocabulary should be compact enough to reuse later on a `2.4-inch` TFT display without coupling it to web-only layout patterns.

</specifics>

<deferred>
## Deferred Ideas

- Add a `2.4-inch` TFT local display in a future milestone. Reuse the Phase 11 state vocabulary, but do not implement TFT support in Phase 11-14.

</deferred>

---

*Phase: 11-Baseline UI Audit And State Language*
*Context gathered: 2026-06-02*
