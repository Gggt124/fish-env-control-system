# Phase 4 Research: Web Pump Control UI

**Researched:** 2026-05-20
**Phase:** 04-web-pump-control-ui

## Research Complete

Phase 4 can be implemented entirely in the existing embedded frontend files:

- `main/static/dashboard.html`
- `main/static/app.js`
- `main/static/style.css`

No new ESP-IDF component, web route, filesystem, CDN dependency, or API handler is needed. Phase 3 already exposes the pump API contract that the dashboard should consume:

- `GET /api/pump/config`
- `POST /api/pump/config`
- `GET /api/pump/status`
- `POST /api/pump/start`
- `POST /api/pump/stop`

The dashboard should remain the authenticated first screen and shift from Wi-Fi/system-first content to pump-control-first content.

## Existing Patterns To Reuse

- `dashboard.html` already uses the shared app shell, sidebar navigation, header actions, status cards, summary rows, and footer.
- `app.js` already centralizes `apiGet()`, `apiPost()`, `showToast()`, `setText()`, auth redirect handling, page initialization by route, and `/api/status` dashboard updates.
- `style.css` already has CSS variables, app shell layout, cards, section cards, summary grids, buttons, progress bars, toast states, responsive breakpoints, and utility classes.
- Static assets are embedded by `main/CMakeLists.txt`; keeping work inside existing embedded files avoids CMake changes.

## API Contract Findings

`GET /api/pump/config` returns seconds-only timer fields plus `auto_start`, `relay_polarity`, read-only hardware defaults, and `settings_status`:

- `timer1_on_sec`
- `timer1_off_sec`
- `timer2_on_sec`
- `timer2_off_sec`
- `auto_start`
- `relay_polarity`
- `float_gpio`
- `relay_gpio`
- `debounce_ms`
- `settings_status`

`POST /api/pump/config` is a full replacement save. The UI must send all editable fields:

- four timer duration fields in seconds
- `auto_start`
- the loaded `relay_polarity` value, preserved even though Phase 4 should not expose GPIO/debounce editing

`GET /api/pump/status` returns machine-friendly runtime fields that the UI must label:

- `running`
- `initialized`
- `config_valid`
- `initial_stabilizing`
- `float_state`
- `active_timer`
- `phase`
- `countdown_sec`
- `relay_energized`
- `float_gpio`
- `relay_gpio`
- `auto_start`
- `settings_status`

Successful start/stop responses include a nested `status` snapshot. The dashboard can apply that snapshot immediately, then continue polling.

## UI And Interaction Findings

The dashboard needs a restrained operational layout rather than a decorative landing page. The first viewport should help the operator answer, in order:

1. Is the controller running?
2. How much time remains?
3. Which timer and phase are active?
4. What are the float switch and relay states?
5. Can I start or stop safely now?

Timer editing belongs below the runtime panel on the same dashboard. Eight minute/second inputs are clearer than one seconds field per duration and align with TIME-01 through TIME-04. The UI should validate before save, but the API remains the source of truth.

The local countdown should tick once per second from the latest device snapshot, while `/api/pump/status` sync every 2 seconds remains authoritative. When the tab is hidden, the UI should stop local ticking and resync when visible again. If sync fails briefly, continue showing the estimated countdown with a stale indicator; after more than 5 seconds without a successful status snapshot, disable Start/Stop.

## Risks And Mitigations

| Risk | Mitigation |
|------|------------|
| Unsaved form changes are confused with active runtime config | Show an unsaved-changes warning and make Start/Stop operate only on applied device config. |
| UI sends read-only hardware fields and trips the API's read-only guard | Preserve only editable fields plus loaded `relay_polarity`; never submit GPIO/debounce fields. |
| Local countdown drifts from hardware state | Poll every 2 seconds while visible and let device snapshots overwrite local estimates. |
| Status API failure breaks the entire dashboard | Split config and status loading so config editing can continue when runtime status is temporarily unavailable. |
| Float state is mistaken for analog water level | Label float state as binary ON/OFF and explicitly tie ON to Timer 2 and OFF to Timer 1. |
| Existing Wi-Fi/system flows regress | Keep sidebar links, header Wi-Fi action, compact quick status, and existing `/status` and `/wifi` pages. |
| Static UI grows beyond embedded local constraints | Use existing HTML/CSS/vanilla JS only, no CDN, no packages, no fonts. |

## Recommended Plan Shape

1. Replace the dashboard body with a pump-first layout and extend CSS for runtime, timer form, field errors, stale/sync states, and responsive behavior.
2. Add JavaScript for config load/save, minute/second conversion, validation, auto-start editing, unsaved-change tracking, and Start/Stop actions.
3. Add live status polling, local countdown ticking, degraded/stale state handling, compact system status preservation, and build/static validation.

## Research Complete
