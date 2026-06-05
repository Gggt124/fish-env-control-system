# Phase 15: UI Refinements - Context

**Gathered:** 2026-06-04
**Status:** Ready for planning
**Source:** Requirements file

<domain>
## Phase Boundary
Implement UI refinements for the Hardware page, Wi-Fi settings, owner dashboard forms, navigation, and styling to improve usability and contrast.
</domain>

<decisions>
## Implementation Decisions

### Hardware / GPIO Page
- D-15-01: The pending map display on the Hardware/Install page must only list the GPIO roles whose pending values actually differ from their active values. If none differ, show a clean empty state label ("No pending changes") per UI-17.
- D-15-02: The unsaved warning ("Unsaved GPIO map changes") must disappear, and the save button must be disabled, if all dropdown selections are reverted to their baseline (active/pending) values per UI-18.

### Wi-Fi Settings
- D-15-03: The Wi-Fi disconnect button text on the Wi-Fi page must be simplified to "Disconnect" (and "Disconnecting..." when clicked) per UI-19.
- D-15-04: Clicking the disconnect button must prompt the user with a confirmation dialog ("คุณต้องการตัดการเชื่อมต่อ Wi-Fi ใช่หรือไม่?") before sending the API request per UI-20.

### Owner Dashboard Forms
- D-15-05: The Pump Settings and Cooling Settings save buttons must be disabled by default (no changes) per UI-21.
- D-15-06: Modifying any input in the Pump Settings or Cooling Settings forms must check if it differs from the saved baseline. If it differs, display the unsaved warning ("มีการแก้ไขที่ยังไม่ได้บันทึก") and enable the save button. If it is reverted back to the loaded configuration, the unsaved warning must disappear and the save button must be disabled per UI-22.

### Navigation / Global
- D-15-07: Clicking the logout menu link in the sidebar must prompt the user with a confirmation dialog ("คุณต้องการออกจากระบบใช่หรือไม่?") before sending the logout request and redirecting per UI-23.

### Styling
- D-15-08: The "Force OFF" cooling mode button on the dashboard must be styled using the red danger button design (`btn-danger`) to match the style of the pump "Stop" button per UI-24.

### the agent's Discretion
- All implementations must be fully compatible with the existing single-page app shell, static CSS variables, and session authentication.

</decisions>

<canonical_refs>
## Canonical References
- `main/static/app.js` — Client side JS logic for all page states, forms, and API requests.
- `main/static/dashboard.html` — Owner dashboard HTML containing Pump and Cooling form elements.
- `main/static/wifi.html` — Wi-Fi page HTML containing the disconnect button.
- `main/static/style.css` — CSS stylesheet containing design system and styling rules.
- `main/web_server.c` — Server side handlers for status and configurations.
</canonical_refs>

<deferred>
## Deferred Ideas
None.
</deferred>
