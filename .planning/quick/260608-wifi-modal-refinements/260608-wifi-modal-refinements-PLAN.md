# Plan: Wi-Fi Settings Layout Refinements

## Must-Haves
- Fix the hanging rescan button spinner in `doScan()`.
- Style the disconnected connection status card to have no background and no border.
- Replace the bell-off SVG with a slashed-out Wi-Fi off SVG for the disconnected state.
- Enlarge the checkboxes inside the Wi-Fi connect modal.
- Remove the confirmation dialog from the Wi-Fi connection trigger, allowing direct connection with a button loading state.

## Execution Plan

<plan>
<task>
<id>1</id>
<title>Refactor HTML files</title>
<description>Update `main/static/index.html` to change the default disconnected status icon SVG to a wifi-off icon.</description>
<file>main/static/index.html</file>
</task>

<task>
<id>2</id>
<title>Update JavaScript logic</title>
<description>Modify `main/static/app.js` to:
1. Restore the "สแกนใหม่" button HTML when scanning completes/fails to resolve the spinner hang.
2. Replace the disconnected SVG string with the wifi-off SVG.
3. Remove `showConfirmModal` wrapper in `doConnect()`, executing the connect logic immediately.
4. Disable the Cancel button while connecting in both `doConnect()` and `pollWifiConnection()` (and re-enable on failure).</description>
<file>main/static/app.js</file>
</task>

<task>
<id>3</id>
<title>Style CSS files</title>
<description>Update `main/static/style.css` to:
1. Make `.connection-status` transparent and borderless by default, only adding container styles when connected.
2. Enlarge checkboxes inside `.checkbox-label` to 18px and align them correctly.</description>
<file>main/static/style.css</file>
</task>
</plan>
