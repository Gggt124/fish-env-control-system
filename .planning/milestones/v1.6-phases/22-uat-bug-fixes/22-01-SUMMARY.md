---
phase: 22-uat-bug-fixes
plan: 01
subsystem: ui
tags:
  - styling
  - bugfix
  - dark-mode
  - sidebar
depends_on: []
provides:
  - light-mode-enforcement
  - login-view-layout
affects:
  - main/static/style.css
  - main/static/app.js
tech_stack_added: []
tech_stack_patterns:
  - DOM classlist toggling for route state
key_files_created: []
key_files_modified:
  - main/static/style.css
  - main/static/app.js
key_decisions:
  - "Decided to entirely remove the dark mode media query to strictly enforce the light theme as per UAT feedback."
  - "Added 'login-active' class to document body to manage fullscreen login layout cleanly."
duration_seconds: 150
completed_date: "2026-06-08T14:06:00+07:00"
---

# Phase 22 Plan 01: Remove Dark Mode and Hide Sidebar on Login Summary

Removed the automatic dark mode media query and fixed login layout by hiding the navigation elements when the login view is active.

## Execution Details
- Removed the `@media (prefers-color-scheme: dark)` block from `main/static/style.css` to prevent auto-switching on OS dark themes.
- Updated `handleRoute` in `main/static/app.js` to toggle a `login-active` class on `document.body` when the `view-login` is active.
- Appended CSS rules for `body.login-active` to hide the sidebar and topbar, and collapse margins to achieve a full-screen layout.

## Deviations from Plan
None - plan executed exactly as written.

## Threat Flags
None.

## Known Stubs
None.

## Self-Check: PASSED
- `main/static/style.css` verified to contain no dark mode media query.
- `main/static/app.js` correctly toggles the `login-active` class.
- Commits `ec94f94` and `2a5f069` successfully created.
