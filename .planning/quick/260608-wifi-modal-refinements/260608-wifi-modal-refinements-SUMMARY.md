---
status: complete
---
# 260608-wifi-modal-refinements: Wi-Fi Settings Layout Refinements

## Summary
Refined the Wi-Fi settings layout based on feedback:
1. Fixed the rescan ("สแกนใหม่") loading spinner hang by restoring its original HTML (icon and text) upon API success or failure in `doScan()`.
2. Styled the disconnected connection status card to be transparent and borderless.
3. Swapped the bell icon in the disconnected state with a standard `wifi-off` (Wi-Fi with a slash) SVG icon in both HTML and Javascript status-updater.
4. Enlarged the checkboxes (Password visibility & Static IP) inside the Wi-Fi connect modal to 18px and added a primary blue `accent-color` to them.
5. Removed the confirmation modal when clicking "เชื่อมต่อ" (Connect), allowing it to directly trigger the connection process and show the loading spinner on the button, while disabling the Cancel button to avoid conflicts.

## Completed Tasks
1. `main/static/index.html` - Replaced default status card SVG icon with a `wifi-off` SVG icon.
2. `main/static/app.js` - Updated `doScan()` to restore button innerHTML on completion. Swapped disconnected status icon SVG. Removed confirm modal wrapper from `doConnect()` and disabled/re-enabled Cancel button during loading states.
3. `main/static/style.css` - Set `.connection-status` to transparent and borderless by default, and added explicit padding in the `.connected` state. Added custom `.checkbox-label` checkbox size (18px) and accent color rules.

## Commits
- `4d2fd05`: feat(260608-wifi-modal-refinements): fix loading spinner hang, enlarge checkboxes, remove confirm modal, and style disconnected status card
