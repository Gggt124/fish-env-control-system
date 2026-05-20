---
phase: 04-web-pump-control-ui
phase_number: "04"
phase_name: web-pump-control-ui
status: issues_found
depth: standard
files_reviewed: 3
scope_source: SUMMARY.md
reviewed_files:
  - main/static/app.js
  - main/static/dashboard.html
  - main/static/style.css
findings:
  critical: 0
  warning: 2
  info: 0
  total: 2
updated: 2026-05-20
---

# Phase 04 Code Review: Web Pump Control UI

## Summary

Reviewed Phase 4 source scope from the phase summary artifacts:

- `main/static/app.js`
- `main/static/dashboard.html`
- `main/static/style.css`

Found two warning-level issues in the dashboard config workflow. Both are frontend state-management problems that can affect operator confidence or hardware-safety assumptions without requiring backend changes.

## Findings

### WR-001: Late config responses can overwrite in-progress timer edits

**Severity:** Warning

**File:** `main/static/app.js:219`

`loadPumpConfig()` unconditionally writes all timer fields, updates `pump-auto-start`, clears validation, and calls `setPumpClean()` when its async `/api/pump/config` response arrives. It is called during dashboard initialization and again after a successful save at `main/static/app.js:346`. If an operator starts editing before the initial config request returns, or changes fields while a save/follow-up reload is in flight, the late response can replace the current form values and hide the unsaved warning.

That makes timer edits easy to lose silently. In the worst case, the UI can display "loaded" or "saved" after replacing operator-entered values with older values from a request that started before those edits.

**Recommendation:** Track a config request sequence or dirty timestamp and only apply `loadPumpConfig()` results when no local edits happened after the request started. Also consider disabling inputs during the initial load/save round trip, or avoiding the post-save reload when the POST response already contains the applied config.

### WR-002: Saving before config load can replace relay polarity with the UI default

**Severity:** Warning

**File:** `main/static/app.js:133`

The dashboard hides relay polarity as intended, but it initializes `pumpRelayPolarity` to `'active_low'` before the persisted config is loaded. `validatePumpConfig()` always includes that value in the full replacement payload at `main/static/app.js:313`. If `/api/pump/config` fails or has not completed and the operator enters timer values and saves, the UI sends `active_low` rather than preserving the device's actual configured polarity.

This conflicts with the project's GPIO safety constraint: relay modules vary, and an unintended polarity change can energize the pump unexpectedly once the backend applies the saved full replacement config.

**Recommendation:** Block pump config saves until `pumpConfig` has loaded successfully and `pumpRelayPolarity` came from the device. If the config load fails, keep the save button disabled and ask the operator to retry loading instead of constructing a replacement payload with a client-side hardware default.

## Positive Checks

- Dashboard DOM IDs match the JavaScript selectors for runtime status, timer fields, auto-start, save, Start, Stop, and alerts.
- Frontend timer validation matches backend bounds: 5 to 86400 seconds, with second subfields limited to 0..59.
- The frontend does not submit read-only hardware fields (`float_gpio`, `relay_gpio`, `debounce_ms`) in the pump config payload.
- Start and Stop actions use the authenticated pump API endpoints and disable while an action is pending or status is stale.
- Status rendering uses the backend's machine-friendly values for `active_timer`, `phase`, `float_state`, `relay_energized`, and `countdown_sec`.
- Static assets remain local-only; no CDN or external frontend dependency was introduced.

## Verification Notes

- Review depth: standard.
- Scope source: Phase 4 `*-SUMMARY.md` artifacts.
- `node --check main/static/app.js` passed.
- Static review covered frontend state flow, API contract compatibility with `main/web_server.c`, validation bounds, and hidden hardware field preservation.
- No ESP-IDF build was run during this review; Phase 4 verification already records a successful `.\scripts\build.ps1` run.
