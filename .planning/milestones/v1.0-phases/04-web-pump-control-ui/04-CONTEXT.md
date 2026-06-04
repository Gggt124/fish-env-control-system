# Phase 4: Web Pump Control UI - Context

**Gathered:** 2026-05-20
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 4 turns the existing local `/dashboard` page into the primary pump-control surface. It adds authenticated web UI controls for Timer 1/Timer 2 ON/OFF durations, auto-start preference, Start/Stop actions, and live pump runtime status. The UI must use the existing embedded HTML/CSS/vanilla JS stack with no CDN or external dependencies, and must preserve access to Wi-Fi setup, system status, login/session, SoftAP fallback, and the existing protected pump APIs. This phase does not add editable GPIO/debounce settings, event logs, charts, WebSocket streaming, hardware validation, or new backend API routes beyond using the Phase 3 `/api/pump/*` contract.

</domain>

<decisions>
## Implementation Decisions

### Dashboard Structure
- **D-01:** `/dashboard` becomes the main Pump Control page rather than a Wi-Fi/system dashboard. Pump status and controls are the first-viewport priority.
- **D-02:** The top dashboard panel must show pump running state, countdown, active timer/phase, relay/float state, and Start/Stop controls.
- **D-03:** Timer 1/Timer 2 settings and auto-start controls stay on the same dashboard below the main runtime panel. Do not require a separate pump settings page in Phase 4.
- **D-04:** Existing Wi-Fi/system information should be reduced to a compact quick-status row or summary section lower on the dashboard. Full system and Wi-Fi details remain available through the existing `/status` and `/wifi` pages.

### Timer Settings And Save Flow
- **D-05:** Timer inputs use separate minute and second fields for each Timer 1/Timer 2 ON/OFF duration: eight input fields total.
- **D-06:** The UI converts minute/second inputs to the Phase 3 API's seconds-only fields: `timer1_on_sec`, `timer1_off_sec`, `timer2_on_sec`, and `timer2_off_sec`.
- **D-07:** The UI validates timer fields before sending a save request: required numeric input, total duration at least 5 seconds, and total duration at most 86400 seconds. Show field-level errors and avoid sending clearly invalid requests. The API remains the source of truth.
- **D-08:** After a successful save, keep the form editable and show saved feedback. Do not lock timer inputs like the simulation prototype.
- **D-09:** If the user changes timer or auto-start fields without saving, show an in-page unsaved-changes warning. Start/Stop controls remain available and act on the currently applied device configuration, not the unsaved form values.
- **D-10:** Auto-start is edited and saved with the timer configuration, but changing it must not imply runtime Start or Stop. It remains a boot behavior setting as locked in prior phases.

### Start Stop And Live Status
- **D-11:** Use two clear Start and Stop buttons in the top runtime panel. Disable the irrelevant action according to the current running state.
- **D-12:** While a Start or Stop request is in flight, show a pending state and disable both Start and Stop until the response returns.
- **D-13:** After Start/Stop returns, update the dashboard from the status snapshot in the API response when available, then continue normal sync.
- **D-14:** Runtime display priority is: controller running state, countdown, active timer/phase, then relay and float state.
- **D-15:** If pump status is unavailable, show a degraded state without breaking the whole dashboard. Display a pump-status error, temporarily disable Start/Stop, and keep config editing available if `/api/pump/config` can still load.

### Thai Status Wording
- **D-16:** Running state wording should distinguish controller state from relay state: `ระบบปั๊ม: กำลังทำงาน` and `ระบบปั๊ม: หยุดอยู่`.
- **D-17:** Float switch wording must make the binary nature obvious and tie it to timer selection, for example `ลูกลอย: OFF (ใช้ Timer 1)` and `ลูกลอย: ON (ใช้ Timer 2)`. Do not present float state as a continuous water level.
- **D-18:** Relay state wording should use ON/OFF plus a short Thai explanation, for example `รีเลย์: ON (จ่ายปั๊ม)` and `รีเลย์: OFF (ตัดปั๊ม)`.
- **D-19:** Timer phase wording should use `ช่วงเปิด` and `ช่วงปิด`, for example `Timer 1 - ช่วงเปิด`.

### Refresh And Countdown Behavior
- **D-20:** Use a split status model. The browser ticks the displayed countdown locally every 1 second from the latest device snapshot.
- **D-21:** Sync `/api/pump/status` every 2 seconds while the dashboard is visible. Device status always wins over the local countdown estimate.
- **D-22:** Trigger an immediate status sync after Start, Stop, Save, and when the tab becomes visible again.
- **D-23:** If status sync temporarily fails, continue the local countdown from the latest snapshot and show a waiting-for-sync or stale indicator.
- **D-24:** If no successful status sync occurs for more than 5 seconds, treat pump status as stale and temporarily disable Start/Stop.
- **D-25:** Stop local ticking while the tab is hidden. When the tab becomes visible again, sync immediately instead of trusting hidden-tab timer behavior.

### the agent's Discretion
Planner may choose exact layout names, CSS class names, Thai microcopy details, field IDs, helper function names, and visual styling within the existing design system, as long as the locked behavior above is preserved. Planner may decide whether to implement the compact Wi-Fi/system quick status as small cards, a summary row, or a restrained status strip.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Scope And Requirements
- `.planning/PROJECT.md` - Product context, active Phase 4 requirements, timer defaults, hardware defaults, and current key decisions.
- `.planning/REQUIREMENTS.md` - Requirement IDs covered by Phase 4: TIME-01, TIME-02, TIME-03, TIME-04, UI-01, UI-02, UI-03, UI-04, UI-05, UI-06, and UI-07.
- `.planning/ROADMAP.md` - Phase 4 goal, success criteria, and boundary from hardware validation work in Phase 5.
- `.planning/phases/01-hardware-safe-pump-control-core/01-CONTEXT.md` - Locked relay safety, binary float behavior, timer selection, start/stop idempotency, and timer bounds.
- `.planning/phases/02-pump-settings-persistence-and-boot-behavior/02-CONTEXT.md` - Locked persistence, auto-start, save/apply, and boot behavior decisions.
- `.planning/phases/03-authenticated-pump-control-api/03-CONTEXT.md` - Locked `/api/pump/*` routes, JSON field names, status strings, save semantics, and start/stop response behavior.

### Codebase Maps
- `.planning/codebase/CONVENTIONS.md` - Frontend, HTTP, component, and documentation conventions.
- `.planning/codebase/STRUCTURE.md` - Static UI files, web server ownership, and extension points.
- `.planning/codebase/STACK.md` - Embedded HTML/CSS/vanilla JS stack, ESP-IDF build, and no-CDN constraint.

### Source Integration Points
- `main/static/dashboard.html` - Existing dashboard shell to replace with pump-first content.
- `main/static/app.js` - Existing `apiGet`, `apiPost`, auth redirect, polling, toast, status refresh, and page-init patterns.
- `main/static/style.css` - Existing CSS variables, card/grid patterns, buttons, toast, and responsive app shell.
- `main/web_server.c` - Embedded static route serving, protected routes, and Phase 3 pump API handlers.

### Flow Reference
- `ref-file/flow-document-v6.md` - Dual Timer + Float Switch behavior reference and binary float-switch terminology. Use project defaults where this reference differs.
- `ref-file/flow-diagram-5.html` - UI simulation reference for timer/float/relay concepts. Treat as visual/behavior inspiration only; do not copy the simulation's lock-after-save flow into firmware UI.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `main/static/app.js`: provides plain `XMLHttpRequest` helpers, page initialization by `window.location.pathname`, existing status polling, toast feedback, auth redirect handling, and HTML/JS escaping helpers.
- `main/static/dashboard.html`: already has authenticated app shell, sidebar navigation, header actions, status-grid cards, summary-grid rows, and footer structure that can be reshaped for pump-first UI.
- `main/static/style.css`: already defines CSS custom properties, responsive app shell, cards, section cards, summary grids, buttons, progress bars, toast notifications, and utility classes.
- `main/web_server.c`: already serves `/dashboard`, `/style.css`, and `/app.js`, and exposes the Phase 3 pump APIs used by this UI.

### Established Patterns
- Frontend uses embedded static HTML/CSS/JS only; no CDN links, package dependencies, or external fonts.
- User-visible UI text is mostly Thai with English technical labels where helpful.
- Pages initialize based on path, so pump dashboard logic should live under the `/dashboard` branch without adding a framework.
- API helpers centralize transport behavior; new pump calls should reuse or minimally extend `apiGet()` and `apiPost()`.
- Existing JS updates DOM elements by IDs and tolerates missing elements through helpers such as `setText()`.

### Integration Points
- Dashboard should call `GET /api/pump/config` on load to populate Timer 1/Timer 2 fields and auto-start.
- Dashboard should call `GET /api/pump/status` for the latest runtime snapshot, then run local countdown ticking between syncs.
- Dashboard should call `POST /api/pump/config` with full replacement editable settings: four timer seconds fields, `auto_start`, and `relay_polarity` if the existing config contract requires preserving it.
- Dashboard should call `POST /api/pump/start` and `POST /api/pump/stop`, then update from the returned status snapshot.
- Timer save must preserve read-only hardware fields and must not attempt to submit `float_gpio`, `relay_gpio`, or `debounce_ms`.
- If relay polarity is included in the existing config contract, the Phase 4 UI should preserve the loaded value unless planner intentionally exposes a safe control for it within existing scope and wording.

</code_context>

<specifics>
## Specific Ideas

- First visible dashboard content should be a pump runtime panel, not the current Wi-Fi/system card grid.
- Countdown should feel live by ticking locally every second while still respecting device snapshots as truth.
- Use wording that avoids implying the float switch reports a 0-100% water level.
- Keep Start/Stop available even when timer form has unsaved changes, because stopping the controller should not be blocked by unsaved UI edits.
- The reference simulation's "save then lock inputs" behavior was explicitly rejected for the firmware UI.

</specifics>

<deferred>
## Deferred Ideas

None - discussion stayed within phase scope.

</deferred>

---

*Phase: 4-Web Pump Control UI*
*Context gathered: 2026-05-20*
