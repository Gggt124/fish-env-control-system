# Requirements: v1.3 UI Details Refinement

**Milestone:** v1.3
**Status:** Complete

## v1.3 Requirements

### Hardware / GPIO Page

- [x] **UI-17**: The pending map display on the Hardware/Install page must only list the GPIO roles whose pending values actually differ from their active values. If none differ, show a clean empty state label ("No pending changes").
- [x] **UI-18**: The unsaved warning ("Unsaved GPIO map changes") must disappear, and the save button must be disabled, if all dropdown selections are reverted to their baseline (active/pending) values.

### Wi-Fi Settings

- [x] **UI-19**: The Wi-Fi disconnect button text on the Wi-Fi page must be simplified to "Disconnect" (and "Disconnecting..." when clicked).
- [x] **UI-20**: Clicking the disconnect button must prompt the user with a confirmation dialog ("คุณต้องการตัดการเชื่อมต่อ Wi-Fi ใช่หรือไม่?") before sending the API request.

### Owner Dashboard Forms

- [x] **UI-21**: The Pump Settings and Cooling Settings save buttons must be disabled by default (no changes).
- [x] **UI-22**: Modifying any input in the Pump Settings or Cooling Settings forms must check if it differs from the saved baseline. If it differs, display the unsaved warning ("มีการแก้ไขที่ยังไม่ได้บันทึก") and enable the save button. If it is reverted back to the loaded configuration, the unsaved warning must disappear and the save button must be disabled.

### Navigation / Global

- [x] **UI-23**: Clicking the logout menu link in the sidebar must prompt the user with a confirmation dialog ("คุณต้องการออกจากระบบใช่หรือไม่?") before sending the logout request and redirecting.

### Styling

- [x] **UI-24**: The "Force OFF" cooling mode button on the dashboard must be styled using the red danger button design (`btn-danger`) to match the style of the pump "Stop" button.

### Regression

- [x] **REG-01**: All existing dashboard, status, Wi-Fi, and cooling features must remain fully operational.

## Traceability

| Requirement | Phase | Status | Outcome |
|-------------|-------|--------|---------|
| UI-17 | Phase 15 | Complete | Verified changes filtered, empty label visible |
| UI-18 | Phase 15 | Complete | Verified warning hides and save button disables on revert |
| UI-19 | Phase 15 | Complete | Verified text simplified to "Disconnect" |
| UI-20 | Phase 15 | Complete | Verified confirm dialog prompt added |
| UI-21 | Phase 15 | Complete | Verified save buttons disabled by default |
| UI-22 | Phase 15 | Complete | Verified warnings show on edit and hide on input reversion |
| UI-23 | Phase 15 | Complete | Verified logout confirm prompt active |
| UI-24 | Phase 15 | Complete | Verified button class changed to btn-danger |
| REG-01 | Phase 15 | Complete | Verified firmware builds and basic regression passes |
