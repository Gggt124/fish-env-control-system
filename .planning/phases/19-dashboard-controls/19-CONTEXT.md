# Phase 19: Dashboard & Control Implementation - Context

**Gathered:** 2026-06-08
**Status:** Ready for planning

<domain>
## Phase Boundary

Redesign Pump, Timer, and Cooling controls to card-based layout.

</domain>

<decisions>
## Implementation Decisions

### Card Layout Strategy
- 2-column grid on desktop, single column stack on mobile

### Control Interaction Design
- Modern pill-shaped toggle switch (iOS style)

### Data Visualization
- Circular progress rings with time remaining inside

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- The newly created SPA architecture (`app.js`, `index.html`) from Phase 18.
- The new CSS variables design system (`style.css`) established in Phase 18.

### Established Patterns
- Client-side routing to `views.dashboard` in `app.js`.
- Pure vanilla JS for interaction. No heavy frameworks.
- SVG icons only (no CDNs).

### Integration Points
- `/api/status` or existing websocket/poll endpoint providing the active timer state, pump state, and cooling state.

</code_context>

<specifics>
## Specific Ideas

No specific requirements — open to standard approaches.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.

</deferred>
