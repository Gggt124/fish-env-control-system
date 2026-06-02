---
quick_id: 260601-iqx
status: complete
description: Add structured web server and Wi-Fi diagnostics logs for 12 hour soak testing
---

# Quick Task 260601-iqx Plan

## Context

The previous long-uptime mitigation build still needs a field soak. Add bounded, structured
serial diagnostics so the next 12-hour log can distinguish socket exhaustion, slow or failed
HTTP handlers, heap fragmentation, STA reconnect churn, SoftAP client churn, and scan cleanup.

## Tasks

1. Add HTTP session, request, route, send-failure, heap, and handler stack diagnostics.
2. Add Wi-Fi event, reconnect, AP fallback, client, and scan lifecycle diagnostics.
3. Emit system, HTTP, route, and Wi-Fi snapshots from the existing 30-second status loop.
4. Run `.\scripts\build.ps1` and inspect the scoped diff.

## Verification

- `.\scripts\build.ps1`
- `git diff --check`
- Hardware soak: capture boot through 12 hours of normal use plus STA connect/disconnect cycles.
