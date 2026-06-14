---
status: complete
---
# Quick Task Summary: Wi-Fi Off SVG Icon Path Fix

## Objective
Fix the broken/disjointed Wi-Fi disconnected SVG icon by replacing it with the standard Feather Icons `wifi-off` SVG definition.

## Changes Made
1. **`main/static/index.html`**:
   - Replaced the SVG paths under `#conn-icon` with the official Feather Icons `wifi-off` SVG paths.
2. **`main/static/app.js`**:
   - Replaced the inline SVG string inside `updateConnectionStatus()` with the official Feather Icons `wifi-off` SVG paths to ensure consistency when dynamically rendering the state.

## Verification
- Built firmware using `idf.py build` (via `scripts/build.ps1`).
- Flashed firmware using `idf.py -p COM5 flash` (via `scripts/flash.ps1`).
