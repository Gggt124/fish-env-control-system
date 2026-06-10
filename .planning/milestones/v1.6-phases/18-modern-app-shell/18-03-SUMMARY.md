---
phase: 18-modern-app-shell
plan: 03
subsystem: "Web Server / SPA"
tags:
  - web
  - ui
  - refactor
  - spa
dependency_graph:
  requires:
    - 18-02
  provides:
    - 18-04
  affects:
    - main/static/index.html
    - main/web_server.c
tech_stack:
  added: []
  patterns:
    - Single Page Application (SPA) routing pattern
key_files:
  created: []
  modified:
    - main/static/index.html
    - main/CMakeLists.txt
    - main/web_server.c
  deleted:
    - main/static/login.html
    - main/static/dashboard.html
    - main/static/status.html
    - main/static/wifi.html
    - main/static/hardware.html
decisions:
  key_decisions:
    - Migrate all standalone HTML bodies directly into `<section id="view-*">` containers in `index.html`.
    - Keep C backend existing route handlers identical (including auth checks), just substitute the returned HTML blob with `index.html`.
metrics:
  duration_minutes: 5
  tasks_completed: 2
  tasks_total: 2
  files_changed: 6
  lines_added: 757
  lines_removed: 1020
  completed_date: "2026-06-07T20:59:00Z"
---

# Phase 18 Plan 03: Unified Frontend Shell Wiring Summary

**Goal:** Migrate legacy HTML fragments to unified SPA and wire C web server.

## Overview
The legacy standalone HTML pages (login, dashboard, status, wifi, hardware) were read and their main core layout contents were copied into `index.html` within their designated view sections (`<section class="view hidden">`). The original HTML files were safely deleted. The C web server `CMakeLists.txt` was updated to embed only `index.html`, and `web_server.c` was modified to point all original HTML endpoints to the `_binary_index_html_start` symbol while maintaining existing authentication checks.

## Key Changes
- Extracted main sections from `login.html`, `dashboard.html`, `status.html`, `wifi.html`, `hardware.html`.
- Migrated those sections into corresponding `<section id="view-*">` containers in `main/static/index.html`.
- Deleted the obsolete HTML files.
- Replaced the multiple HTML files in `EMBED_FILES` of `main/CMakeLists.txt` with `static/index.html`.
- Rewrote the extern definitions and `serve_static` handlers in `main/web_server.c` to serve `index.html` for all view routes.

## Deviations from Plan
None - plan executed exactly as written.

## Threat Flags
None.

## Known Stubs
None.

## Next Steps
Continue testing frontend routing to ensure UI views toggle properly based on the path.

## Self-Check: PASSED
- FOUND: main/static/index.html
- MISSING: main/static/login.html
- FOUND: 9e177e9
- FOUND: d5da227
