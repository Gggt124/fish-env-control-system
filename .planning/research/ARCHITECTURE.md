# Architecture Research

**Domain:** Embedded offline ESP32 owner dashboard UI polish
**Researched:** 2026-06-02
**Confidence:** HIGH

## Boundary

v1.2 should remain inside the embedded frontend and documentation boundary unless execution reveals a verified UI state bug caused by an API contract issue.

```text
Existing ESP-IDF runtime and APIs (stable v1.1)
                    |
                    v
       Existing apiGet/apiPost transport
                    |
                    v
 Existing page-specific render and action functions
                    |
                    v
 Shared CSS tokens, layout, and state presentation
                    |
                    v
 Desktop/mobile screenshot and keyboard verification
```

## Existing Surfaces

| Surface | Existing Responsibility | v1.2 Direction |
|---------|-------------------------|----------------|
| `dashboard.html` | Pump runtime, cooling runtime, settings, compact diagnostics | Establish owner-first hierarchy and reduce equal-weight presentation. |
| `hardware.html` | Wiring visualization, active/pending GPIO map, safe edit flow | Improve setup guidance and DS18B20 pull-up readiness messaging. |
| `wifi.html` | Scan, select, password, static IP, connect flow | Clarify loading/error/empty scan and connection states. |
| `status.html` | Dense diagnostics | Preserve diagnostics while improving responsive readability and state feedback where needed. |
| `style.css` | Shared design tokens and responsive layouts | Make systemic token and state improvements before page-local exceptions. |
| `app.js` | Transport, polling, rendering, form state | Normalize loading/error/empty rendering and accessible status announcements with bounded DOM updates. |

## Patterns

### Owner First, Installer Separate

Daily operation belongs on `/dashboard`. Advanced pin mapping and wiring confirmation remain on `/hardware`. Do not move GPIO editing into the daily owner surface.

### State Presentation Matrix

Each asynchronous surface should define loading, ready, empty, error, disabled, and success states. Reuse existing API transport and update existing containers instead of introducing a new state framework.

### Progressive Disclosure

Show the decision-critical state first: pump running/stopped, selected timer/relay, countdown, float state, temperature, cooling mode, cooling fault. Keep diagnostics and configuration lower in the page or behind clear sections.

### Bounded Embedded Updates

Use text replacement, class toggles, and small generated lists. Avoid unbounded DOM growth, large inline assets, or polling changes that could disturb the passed APSTA soak behavior.

## Suggested Build Order

1. Define UI state inventory, hierarchy, and shared CSS/accessibility conventions.
2. Polish dashboard and Hardware/Install flows with DS18B20 readiness guidance.
3. Polish Wi-Fi/status edge states and run screenshot, keyboard, build, and regression verification.

## Internal Integration Points

| Boundary | Rule |
|----------|------|
| Frontend -> pump/cooling APIs | Preserve v1.1 API behavior and polling intervals unless a UI state bug is proven. |
| Frontend -> hardware API | Preserve safe enum options, active/pending values, checkbox confirmation, and reboot-required behavior. |
| Frontend -> Wi-Fi APIs | Preserve scan/connect/disconnect behavior and APSTA fallback behavior. |
| Static assets -> ESP-IDF embed | Keep filename-based `EMBED_FILES` symbols unchanged unless adding a new static file is unavoidable. |

## Sources

- Local UI implementation: `main/static/`
- Local architecture map: `.planning/codebase/ARCHITECTURE.md`
- W3C WAI Understanding Reflow: https://www.w3.org/WAI/WCAG22/Understanding/reflow.html
- W3C WAI Understanding Focus Visible: https://www.w3.org/WAI/WCAG22/Understanding/focus-visible

---
*Architecture research for: embedded offline ESP32 owner dashboard UI polish*
*Researched: 2026-06-02*
