# Phase 16: Wi-Fi Setup UI Polish - Research

**Researched:** 2026-06-05
**Domain:** HTML, CSS, JavaScript (Vanilla Frontend)
**Confidence:** HIGH

## Summary

This research phase addresses the polish of the Wi-Fi setup configuration UI. The primary objective is to replace the CPU-heavy backdrop blur disabled panel overlay (`.overlay-hint`) with a clean Empty State Card (Option 1) that instructs the user to select a Wi-Fi network from the left-side list. When a network is selected, the Empty State Card is hidden, and the credentials form is presented using a smooth CSS opacity transition (0.2s) while honoring the `prefers-reduced-motion` media query. If the user cancels the selection, the credentials form is hidden and the Empty State Card is restored using the same transition. The layout stacks normally on mobile.

All changes are fully offline-compatible, relying exclusively on the native browser HTML5/CSS3/JavaScript stack without external CDN dependencies.

**Primary recommendation:** Use a wrapper container `div` inside the desktop grid layout to contain both the Empty State Card and the input panel, and use a CSS transition class (`.fade`) with a short JavaScript timeout (200ms) to choreograph the swap sequence without causing page height layout shifts.

## User Constraints

### Implementation Decisions

#### Empty State Card Design
- **D-01:** Replace the blurred overlay (`.overlay-hint`) with a clean, styled Empty State Card inside the input panel area when no SSID is selected.
- **D-02:** The Empty State Card will contain:
  - An icon: `📶` (Wi-Fi symbol) or `👈` (finger pointing left) in the center.
  - A heading in Thai: "โปรดเลือกเครือข่าย" (Please select a network).
  - A description in Thai explaining: "เลือกเครือข่าย Wi-Fi ที่พบในตารางด้านซ้ายเพื่อกรอกข้อมูลการตั้งค่า" (Select a Wi-Fi network found in the table on the left to enter configuration details).
- **D-03:** The card will use the existing design system variables from `style.css` (e.g. `var(--surface-container-low)`, `border: 1px dashed var(--outline-variant)`, `border-radius: var(--radius-lg)`).
- **D-04:** Avoid performance-heavy GPU styles like `backdrop-filter: blur` entirely.

#### Transitions & Animations
- **D-05:** Implement a smooth CSS fade-in transition (`opacity: 0` to `opacity: 1` over `0.2s` using `ease-out`) when swapping between the Empty State Card and the actual Wi-Fi credentials input form.
- **D-06:** Provide a media query for `@media (prefers-reduced-motion: reduce)` that performs an instant swap (0s transition) to support accessibility.

#### Mobile Responsive Layout
- **D-07:** Maintain the standard stacked layout on mobile devices. The Empty State Card (and later the credentials form) will display directly underneath the Wi-Fi scan table when the viewport width is below the desktop breakpoint.

#### the agent's Discretion
- The exact layout spacing and icon sizing within the Empty State Card.
- The classes used to toggle visibility between the Empty State Card and the input form (e.g. `.hidden` or opacity-based state classes).

### Deferred Ideas
- None — discussion stayed within phase scope.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Empty State Card Presentation | Frontend HTML (`main/static/wifi.html`) | — | Static markup for the initial state of the setup UI. |
| Design Tokens & Styles | Frontend CSS (`main/static/style.css`) | — | Exposes colors, spacing, borders, transitions, and media query support. |
| User Actions & Transition Logic | Frontend JS (`main/static/app.js`) | — | Handles network click, cancel button click, class toggling, and layout swapping. |
| SSID Rendering Sanitization | Frontend JS (`main/static/app.js`) | — | Escapes dynamic text using `escHtml` and `escJs` to prevent injection vectors. |

## Standard Stack

### Core
| Library / Technology | Version | Purpose | Why Standard |
|----------------------|---------|---------|--------------|
| HTML5 | Native | Layout structure | Standard document structure for the embedded web page. |
| CSS3 | Native | Styling & transition | Standard styling of cards, grid columns, and accessibility. |
| Vanilla JavaScript | ES5+ | State & event triggers | Native runtime browser execution without build dependencies. |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| CSS Transition | Web Animations API | Overkill for simple opacity toggles. CSS classes are simpler and more performant. |
| CSS Grid column swapping | Flexbox wrapper | Grid wrapper gives cleaner alignment control on desktop breakpoints. |

**Installation:**
No installation commands are necessary for this phase. The project strictly requires that the static assets remain independent of any package manager or third-party web dependencies.

## Package Legitimacy Audit

No packages are installed, modified, or updated during this phase.

| Package | Registry | Age | Downloads | Source Repo | slopcheck | Disposition |
|---------|----------|-----|-----------|-------------|-----------|-------------|
| None | — | — | — | — | — | Approved |

## Architecture Patterns

### System Architecture Diagram

```mermaid
graph TD
    User([User Client Browser])
    HTML[wifi.html]
    CSS[style.css]
    JS[app.js]
    API[/api/wifi/scan]
    
    User -->|Access /wifi| HTML
    HTML -->|Loads styles| CSS
    HTML -->|Loads script| JS
    JS -->|Polls network data| API
    
    subgraph UI Polish Scope
        JS -->|Toggles classes on user action| CardWrapper[Panel Wrapper]
        CardWrapper -->|Contains| ESCard[Empty State Card]
        CardWrapper -->|Contains| InputPanel[Input Credentials Form]
    end
```

### Recommended Project Structure
The existing project structure remains unchanged. Polish changes will strictly modify the following static assets:
```
main/
└── static/
    ├── wifi.html       # Updated to contain the new Empty State Card markup
    ├── style.css       # Updated with transition, prefers-reduced-motion, and empty state styling
    └── app.js          # Updated to toggle class-based transitions on selection and cancel
```

### Pattern 1: Transition Choreography
To transition opacity without stacking the cards vertically or horizontally during the transition, use a sequential class toggle inside `app.js`:

```javascript
// Source: app.js
function fadeSwap(toHide, toShow) {
    var reduced = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
    if (reduced) {
        toHide.classList.add('hidden');
        toShow.classList.remove('hidden');
        return;
    }

    toHide.classList.add('fade');
    setTimeout(function() {
        toHide.classList.add('hidden');
        toHide.classList.remove('fade');

        toShow.classList.add('fade');
        toShow.classList.remove('hidden');
        
        // Force reflow to register display: block/flex before opacity transition
        toShow.offsetHeight;
        
        toShow.classList.remove('fade');
    }, 200); // Matches the 0.2s duration
}
```

### Anti-Patterns to Avoid
- **GPU Backdrop Blurring**: Do not use `backdrop-filter: blur(3px)` or `-webkit-backdrop-filter: blur(3px)`. This forces expensive compositing layers that can trigger frames dropping on standard mobile browsers.
- **JS Animators**: Do not import or hand-roll custom javascript animation loops (using `setInterval` or `requestAnimationFrame`). CSS transitions are native, GPU-friendly, and automatically throttled by the browser.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| UI Animation | JS-based animation loops | Native CSS transitions | CSS transitions are optimized by browser layout engines and easily overridden by accessibility media queries. |
| Media Query Listeners in JS | Complex orientation/motion triggers | CSS media query `@media (prefers-reduced-motion)` | Pure CSS styles keep the presentation logic out of JS and execute instantly. |

## Runtime State Inventory

None — this phase only polishes static web assets (HTML/CSS/JS) and has no impact on stored data, live service config, OS-registered state, secrets, or build artifacts.

## Common Pitfalls

### Pitfall 1: Double-Trigger Layout Shifts
- **What goes wrong**: When transitioning from Empty State to the input panel, both cards are momentarily visible, causing the right column to grow vertically and push the footer down.
- **Why it happens**: If `display: none` is removed from the incoming card before the outgoing card gets `display: none`.
- **How to avoid**: Ensure the swap of the `.hidden` class happens after the first card finishes its fade-out transition, or use a container with absolute positioning (sequential fade-out/fade-in is simpler and safer).

### Pitfall 2: Bypassing Reduced Motion Settings
- **What goes wrong**: Users with vestibulo-ocular disorders experience disorienting visual animations because the website ignores operating system preferences.
- **Why it happens**: Hardcoded animation and transition times in CSS.
- **How to avoid**: Override standard CSS transitions inside a `@media (prefers-reduced-motion: reduce)` block to force `transition-duration: 0s !important`.

### Pitfall 3: Broken JS References after Markup Refactor
- **What goes wrong**: Selecting a Wi-Fi or clicking Cancel throws a JavaScript runtime exception because of a missing element ID.
- **Why it happens**: Moving the `#input-panel` ID or removing `#overlay-hint` without updating the query selectors in `app.js`.
- **How to avoid**: Keep the ID names consistent (`input-panel`, `overlay-hint` or its replacement card) and update references inside `selectNetwork()` and `clearSelection()`.

## Code Examples

### CSS Transition and Dark Mode Compatibility Styles
```css
/* Source: style.css draft */

/* Empty State Card */
.empty-state-card {
    background: var(--surface-container-low);
    border: 1px dashed var(--outline-variant);
    border-radius: var(--radius-lg);
    padding: 32px 24px;
    text-align: center;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    gap: var(--sp-md);
    min-height: 300px;
    box-sizing: border-box;
}

.empty-state-icon {
    font-size: 48px;
    line-height: 1;
    margin-bottom: var(--sp-xs);
    user-select: none;
}

.empty-state-card h3 {
    margin: 0;
    font-size: 18px;
    font-weight: 700;
    color: var(--on-surface);
}

.empty-state-card p {
    margin: 0;
    font-size: 14px;
    color: var(--on-surface-variant);
    line-height: 20px;
    max-width: 280px;
}

/* Transitions */
.empty-state-card, .input-panel {
    transition: opacity 0.2s ease-out;
    opacity: 1;
}

.empty-state-card.fade, .input-panel.fade {
    opacity: 0;
}

.empty-state-card.hidden, .input-panel.hidden {
    display: none !important;
}

@media (prefers-reduced-motion: reduce) {
    .empty-state-card, .input-panel {
        transition: none !important;
    }
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Backdrop Blur Overlays | Empty State Cards & Sequential Swapping | Phase 16 (Current) | Dramatically reduces CPU repaint overhead, improves user accessibility, and ensures clarity. |

## Assumptions Log

All claims in this research were verified or cited — no user confirmation needed.

## Open Questions

1. **Should the credentials form height match the empty state card height?**
   - *What we know:* The Empty State Card uses a dashed border and centered contents.
   - *What's unclear:* Whether a fixed minimum height should be enforced on both cards to completely eliminate footer layout adjustments during swap.
   - *Recommendation:* Add a `min-height: 300px` to both `.empty-state-card` and `.input-panel` so the visual weight of the right side remains uniform.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| Modern Web Browser | HTML5 / CSS Grid / CSS Variables | ✓ | Standard | Flexbox Layout fallback |
| CSS Transitions Support | Opacity Animation | ✓ | Standard | Instant swap |
| prefers-reduced-motion Media Query | Accessible transition toggle | ✓ | Standard | Fallback to standard 0.2s swap |

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | python unittest |
| Config file | none — see Wave 0 |
| Quick run command | `python -m unittest tests/test_ui_phase13.py` |
| Full suite command | `python -m unittest discover tests` |

### Phase Requirements → Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| WIFI-UI-01 | Credentials input panel replaced by Empty State Card when no network selected | Content Check | `python -m unittest tests/test_ui_phase16.py` | ❌ Wave 0 |
| WIFI-UI-02 | Transition from Empty State to Input Panel upon network selection | Content Check | `python -m unittest tests/test_ui_phase16.py` | ❌ Wave 0 |
| WIFI-UI-03 | Clicking Cancel button resets selection and restores Empty State Card | Content Check | `python -m unittest tests/test_ui_phase16.py` | ❌ Wave 0 |
| WIFI-UI-04 | Empty State uses existing variables and avoids backdrop-filter | Style Audit | `python -m unittest tests/test_ui_phase16.py` | ❌ Wave 0 |

### Sampling Rate
- **Per task commit:** Local file validation.
- **Phase gate:** Local test suite runs clean before shipping.

### Wave 0 Gaps
- [ ] `tests/test_ui_phase16.py` — covers WIFI-UI-01 through WIFI-UI-04.

## Security Domain

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V5 Input Validation | Yes | Use `escHtml` and `escJs` when rendering dynamic SSIDs to prevent DOM injection vulnerabilities. |
| V4 Access Control | Yes | Authenticated routes `/dashboard`, `/status`, and `/wifi` redirect to `/login` if session is absent. |

### Known Threat Patterns for ESP32 Web Frontend

| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| Dynamic SSID Injection | Tampering | Always wrap dynamic Wi-Fi scan inputs in HTML-escaping helper functions (`escHtml`, `escJs`) before DOM placement. |
| Host Spoofing / CSRF | Elevation of Privilege | Mitigated by CORS/Origin checks in the firmware routes (`web_server.c`). |

## Sources

### Primary (HIGH confidence)
- `main/static/wifi.html` - Static document structure.
- `main/static/style.css` - Custom properties and layout rules.
- `main/static/app.js` - JS dynamic handlers.
- `.planning/phases/16-wi-fi-setup-ui-polish/16-CONTEXT.md` - Phase constraints.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - Core web standards.
- Architecture: HIGH - Fully isolated to the frontend assets.
- Pitfalls: HIGH - Documented repainting issues with backdrop filters and mobile grid layouts.

**Research date:** 2026-06-05
**Valid until:** 2026-07-05
