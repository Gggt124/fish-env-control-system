# Requirements: Milestone v1.4 Wi-Fi UI Polish and Code Review

## v1 Requirements

### Wi-Fi Setup UI Polish (WIFI-UI)
- [x] **WIFI-UI-01**: When no Wi-Fi network is selected, the credentials input panel is hidden and replaced by a clean, styled Empty State Card instructing the user to select a network from the left.
- [x] **WIFI-UI-02**: Clicking a network in the scan list hides the Empty State Card and displays the input form with a smooth fade-in transition.
- [x] **WIFI-UI-03**: Clicking the "Cancel" (ยกเลิก) button resets the selection, hiding the input form and restoring the Empty State Card.
- [x] **WIFI-UI-04**: The Empty State Card must use the existing CSS variables for design system consistency and completely avoid the performance-heavy CSS backdrop blur.

### Code Quality Review (CODE-REV)
- [ ] **CODE-REV-01**: Review C firmware source files for compile warnings, logical bugs, and race conditions.
- [ ] **CODE-REV-02**: Review javascript/html files for console errors, styling inconsistency, or cleanup opportunities.
- [ ] **CODE-REV-03**: Safely resolve all identified issues while preserving verified pump, cooling, and Wi-Fi soak stability.

## Future Requirements (Deferred)
- None

## Out of Scope
- Migrating the frontend to a framework (React, Vue, etc.) or introducing external CSS/JS libraries (must stay plain HTML/CSS/vanilla JS).
- Changing the underlying connection state API or connection logic itself (focus is purely on UI/UX and codebase polish).

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| WIFI-UI-01 | Phase 16 | Complete |
| WIFI-UI-02 | Phase 16 | Complete |
| WIFI-UI-03 | Phase 16 | Complete |
| WIFI-UI-04 | Phase 16 | Complete |
| CODE-REV-01 | Phase 17 | Pending |
| CODE-REV-02 | Phase 17 | Pending |
| CODE-REV-03 | Phase 17 | Pending |

