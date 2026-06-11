# Phase 21: Wi-Fi Setup & UX Polish - Context

**Gathered:** 2026-06-08
**Status:** Ready for planning

<domain>
## Phase Boundary

Overhaul Wi-Fi Setup stepper and integrate loading/modal feedback across the app.

</domain>

<decisions>
## Implementation Decisions

### Wi-Fi Setup Stepper
- A horizontal numbered stepper at the top of the card

### Loading State Indicators
- The button text fades out and is replaced by a centered CSS spinner

### Modal Confirmations
- A custom modal dialog with a blurred backdrop (glassmorphism style)

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `app.js` routing and API fetching logic from prior phases.
- `style.css` variables, layout grid, and modern UI components.

### Established Patterns
- Client-side DOM manipulation without frameworks.
- Inline SVGs.

</code_context>

<specifics>
## Specific Ideas

No specific requirements — open to standard approaches.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.

</deferred>
