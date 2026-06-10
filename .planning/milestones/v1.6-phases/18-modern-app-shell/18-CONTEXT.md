# Phase 18: Modern App Shell & Architecture Foundation - Context

**Gathered:** 2026-06-08
**Status:** Ready for planning

<domain>
## Phase Boundary

Establish SPA layout, CSS variables, and typography.

</domain>

<decisions>
## Implementation Decisions

### Typography System
- Modern System (system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto)
- Major Third (1.250 ratio) for clear hierarchy
- 16px (1rem) for accessibility standard

### CSS Architecture
- HSL (Hue, Saturation, Lightness) for easy programmatic adjustments
- 4px/8px baseline grid (e.g., --space-1: 4px, --space-2: 8px)
- Media query (prefers-color-scheme: dark) mapped to CSS variables

### SPA Layout Structure
- Hamburger menu opening a left-side drawer
- Max-width container (e.g., 1200px) centered

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `main/static/style.css` (Baseline to be refactored to use variables and the new system font stack)
- `main/static/app.js` (Baseline to structure SPA logic)
- `main/static/*.html` (Existing UI to wrap into SPA shell)

### Established Patterns
- No external dependencies (no CDNs, only inline SVG and system fonts)
- Embedded HTML/CSS/JS via ESP-IDF `EMBED_FILES`
- Vanilla JavaScript

### Integration Points
- `index.html` (Unified SPA shell)
- Existing API handlers in `web_server.c`

</code_context>

<specifics>
## Specific Ideas

No specific requirements — open to standard approaches.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.

</deferred>
