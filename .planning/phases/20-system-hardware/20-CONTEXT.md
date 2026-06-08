# Phase 20: System Status & Hardware/Install Pages - Context

**Gathered:** 2026-06-08
**Status:** Ready for planning

<domain>
## Phase Boundary

Overhaul the status view and GPIO configuration interface.

</domain>

<decisions>
## Implementation Decisions

### System Status Display
- Small, inline horizontal progress bars next to the labels

### Hardware/Install Pending State
- A yellow "Unsaved Changes" banner at the top of the card with an explicit "Apply" button

### Status Page Navigation
- Separate menu items in the main navigation drawer

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `app.js` routing logic from Phase 18 and 19.
- `style.css` variables and components.

### Established Patterns
- SPA views rendered via vanilla JS.
- Status data fetched via `apiGet('/api/status')`.

</code_context>

<specifics>
## Specific Ideas

No specific requirements — open to standard approaches.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.

</deferred>
