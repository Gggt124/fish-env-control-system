---
phase: 04-web-pump-control-ui
phase_number: "04"
phase_name: web-pump-control-ui
status: clean
depth: standard
files_reviewed: 3
scope_source: SUMMARY.md
reviewed_files:
  - main/static/app.js
  - main/static/dashboard.html
  - main/static/style.css
findings:
  critical: 0
  warning: 0
  info: 0
  total: 0
updated: 2026-05-20
supersedes_review_commit: bc588eb
fix_commit: e49e091
---

# Phase 04 Code Review: Web Pump Control UI

## Summary

Reviewed Phase 4 source scope from the phase summary artifacts:

- `main/static/app.js`
- `main/static/dashboard.html`
- `main/static/style.css`

No open issues remain at standard review depth. The two warning findings from the first Phase 4 review are resolved in fix commit `e49e091`.

## Resolved Findings

### WR-001: Late config responses can overwrite in-progress timer edits

**Status:** Resolved

**File:** `main/static/app.js:219`

`loadPumpConfig()` now captures the edit version at request start. When a response returns after local edits, the handler updates device metadata such as relay polarity but does not rewrite timer fields, clear validation, or mark the form clean. The save flow also no longer triggers a post-save reload that could race with fresh edits; it relies on the POST response and preserves an unsaved state if edits occur while the save request is in flight.

### WR-002: Saving before config load can replace relay polarity with the UI default

**Status:** Resolved

**File:** `main/static/app.js:133`

`pumpRelayPolarity` now starts as `null` and the save button stays disabled until `/api/pump/config` loads successfully. `savePumpConfig()` also has a hard guard that rejects saves without a loaded device polarity, so the full replacement payload cannot silently substitute a client-side relay polarity default.

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
- `node --check main/static/app.js` passed after the fix.
- Static review covered frontend state flow, API contract compatibility with `main/web_server.c`, validation bounds, and hidden hardware field preservation.
- No ESP-IDF build was run during this fix pass; Phase 4 verification already records a successful `.\scripts\build.ps1` run.
