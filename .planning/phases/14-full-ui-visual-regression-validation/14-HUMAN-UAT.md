---
status: partial
phase: 14-full-ui-visual-regression-validation
source:
  - 14-VERIFICATION.md
started: 2026-06-03
updated: 2026-06-03
---

# Phase 14 Human UAT

## Current Test

Awaiting hardware/device-backed validation on the freshly built firmware.

## Tests

### 1. Flash current firmware

expected: ESP32 serves static assets matching current repository build and mobile drawer opens at 375px.
result: pending

### 2. Capture required screenshots

expected: Login, Dashboard, Hardware/Install, Wi-Fi, Status, and representative states have desktop and mobile screenshots or an approved screenshot-capture fallback.
result: pending

### 3. Run stable hardware regression

expected: Relay, timer, float, cooling, Wi-Fi STA, APSTA fallback, login/session, and status flows pass on hardware or produce concrete issues.
result: pending

## Summary

total: 3
passed: 0
issues: 0
pending: 3
skipped: 0
blocked: 0

## Gaps

- Current device static assets were stale compared with repository source.
- In-app Browser screenshot capture timed out.
- Hardware-impacting POST actions were intentionally not run.
