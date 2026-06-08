# 260608-wifi-modal-layout: Wi-Fi Settings Page Styling and Modal Connection Dialog

## Summary
Refactored the Wi-Fi settings page to use a single-column layout, making the networks list card span the full width of the container. Replaced the right-hand split input panel/empty state with a dedicated `#wifi-connect-modal` dialog overlay that prompts for password and static IP credentials when a network is selected. Styled the connected status banner at the top to feature a premium green visual identity, keeping the disconnect button styled as red. Added a 2-second auto-close timeout upon successful connection.

## Completed Tasks
1. `main/static/index.html` - Moved SSID, password, static IP settings, and cancel/connect buttons inside `#wifi-connect-modal`. Made networks list card full width.
2. `main/static/app.js` - Modified `selectNetwork()` to open the connection modal and reset inputs, `clearSelection()` to close the modal and reset states, and added a 2-second timeout to close the modal when connection is successful.
3. `main/static/style.css` - Removed split grid layout from `.wifi-grid`. Styled the modal overlay/card for Wi-Fi connection. Added custom styling rules for `.connection-status.connected`.

## Commits
- `9d4e318`: feat(260608-wifi-modal-layout): refactor Wi-Fi layout to full width and use modal for connection
