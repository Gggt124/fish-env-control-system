# Plan: Wi-Fi Settings Page Styling and Modal Connection Dialog

## Must-Haves
- Wi-Fi view layout (`#view-wifi`) should have a full-width network list card, removing the split layout.
- Wi-Fi connection inputs (password, static IP settings, validation status) should reside in a modal dialog overlay (`#wifi-connect-modal`) which triggers when clicking a network.
- The connected status banner at the top of the Wi-Fi view should display a premium green design (light green background, emerald text, green checkmark icon).
- The disconnect button inside the connected banner must remain red (`btn-danger`).
- Successful connection should display in the modal for 2 seconds, then close the modal and update status.

## Execution Plan

<plan>
<task>
<id>1</id>
<title>Update Wi-Fi HTML structure</title>
<description>Modify `main/static/index.html` to place connection inputs (password, static IP checkboxes, and fields) inside a new modal wrapper `#wifi-connect-modal`. Make the network list card full width inside `.wifi-grid` and remove the empty state card.</description>
<file>main/static/index.html</file>
</task>

<task>
<id>2</id>
<title>Update Wi-Fi JavaScript Logic</title>
<description>Modify `main/static/app.js` to open `#wifi-connect-modal` on network selection. Redirect SSID display to the modal. Manage inputs within the modal and auto-close the modal upon successful connection after 2 seconds.</description>
<file>main/static/app.js</file>
</task>

<task>
<id>3</id>
<title>Style Wi-Fi layout and modal in CSS</title>
<description>Update `main/static/style.css` to remove grid columns for `.wifi-grid` so that the networks card spans full width. Add styles for `#wifi-connect-modal` (reusing modal overlay/card patterns). Add green styling for `.connection-status.connected` banner.</description>
<file>main/static/style.css</file>
</task>
</plan>
