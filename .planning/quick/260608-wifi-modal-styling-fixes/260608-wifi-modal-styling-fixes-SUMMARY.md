---
status: complete
---
# Quick Task Summary: Wi-Fi Card and Icon Style Refinements

## Objective
Restore the white card container styling to the Wi-Fi connection status component and ensure the disconnected SVG icon appears in a greyish tone.

## Changes Made
1. **`main/static/style.css`**:
   - Restored `.connection-status` container styling to have a white surface background (`var(--surface)`), border (`1px solid var(--outline-variant)`), box shadow (`var(--shadow-sm)`), and padding (`12px 16px`).
   - Set the default `.connection-status .status-icon` color to `var(--on-surface-variant)` (greyish tone) to style the disconnected SVG icon correctly.

## Verification
- Built firmware using `idf.py build` (via `scripts/build.ps1`).
- Flashed firmware using `idf.py -p COM5 flash` (via `scripts/flash.ps1`).
