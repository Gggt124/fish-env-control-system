# Phase 16: Wi-Fi Setup UI Polish - Context

**Gathered:** 2026-06-05
**Status:** Ready for planning

<domain>
## Phase Boundary

Polish the Wi-Fi setup configuration UI. This includes replacing the CPU-heavy backdrop blur disabled panel overlay with a clean Empty State Card (Option 1) that instructs the user to choose a Wi-Fi from the left list. Selecting a network swaps the Empty State Card for the credentials form using a smooth CSS opacity transition (0.2s) while honoring the prefers-reduced-motion media query. The layout must stack normally on mobile.

</domain>

<decisions>
## Implementation Decisions

### Empty State Card Design
- **D-01:** Replace the blurred overlay (`.overlay-hint`) with a clean, styled Empty State Card inside the input panel area when no SSID is selected.
- **D-02:** The Empty State Card will contain:
  - An icon: `📶` (Wi-Fi symbol) or `👈` (finger pointing left) in the center.
  - A heading in Thai: "โปรดเลือกเครือข่าย" (Please select a network).
  - A description in Thai explaining: "เลือกเครือข่าย Wi-Fi ที่พบในตารางด้านซ้ายเพื่อกรอกข้อมูลการตั้งค่า" (Select a Wi-Fi network found in the table on the left to enter configuration details).
- **D-03:** The card will use the existing design system variables from `style.css` (e.g. `var(--surface-container-low)`, `border: 1px dashed var(--outline-variant)`, `border-radius: var(--radius-lg)`).
- **D-04:** Avoid performance-heavy GPU styles like `backdrop-filter: blur` entirely.

### Transitions & Animations
- **D-05:** Implement a smooth CSS fade-in transition (`opacity: 0` to `opacity: 1` over `0.2s` using `ease-out`) when swapping between the Empty State Card and the actual Wi-Fi credentials input form.
- **D-06:** Provide a media query for `@media (prefers-reduced-motion: reduce)` that performs an instant swap (0s transition) to support accessibility.

### Mobile Responsive Layout
- **D-07:** Maintain the standard stacked layout on mobile devices. The Empty State Card (and later the credentials form) will display directly underneath the Wi-Fi scan table when the viewport width is below the desktop breakpoint.

### the agent's Discretion
- The exact layout spacing and icon sizing within the Empty State Card.
- The classes used to toggle visibility between the Empty State Card and the input form (e.g. `.hidden` or opacity-based state classes).

</decisions>

<specifics>
## Specific Ideas

- The transition should feel snappy and clean, matching the styling of other cards on the dashboard.
- The Cancel button (ยกเลิก) must cleanly fade out the input form and fade the Empty State Card back in.

</specifics>

<canonical_refs>
## Canonical References

### Wi-Fi Configuration Assets
- `main/static/wifi.html` — The HTML markup of the Wi-Fi settings page.
- `main/static/style.css` — The CSS styles governing the input panel, overlays, and transitions.
- `main/static/app.js` — The JavaScript logic governing scan network selections, cancels, and state toggles.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `style.css` contains variables like `var(--surface-container-low)`, `var(--outline-variant)`, `var(--radius-lg)`, and `var(--primary)` which should be used for the card styling.
- `app.js` has functions `selectNetwork()` and `clearSelection()` which handle class additions and style toggles on the input panel.

### Established Patterns
- All authenticated UI pages are plain HTML, CSS, and vanilla JS with zero external CDN dependencies, ensuring full offline capability in SoftAP mode.
- User-facing text in the web UI uses Thai first.

### Integration Points
- `selectNetwork(ssid)` in `app.js` (line 1889) — transition from Empty State to the form.
- `clearSelection()` in `app.js` (line 1934) — transition from the form back to the Empty State.

</code_context>

<deferred>
## Deferred Ideas

- None — discussion stayed within phase scope.

</deferred>

---

*Phase: 16-wi-fi-setup-ui-polish*
*Context gathered: 2026-06-05*
