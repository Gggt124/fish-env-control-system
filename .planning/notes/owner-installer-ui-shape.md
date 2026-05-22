---
title: "Owner and installer UI shape"
date: "2026-05-22"
context: "Exploration of temperature cooling control, dual timer relays, and editable GPIO mapping"
---

# Owner And Installer UI Shape

The web UI should serve two different moments without making daily operation feel like a setup tool.

The owner dashboard should focus on operation: pump enabled state, float state, active timer/relay, countdown, temperature, cooling relay state, threshold, auto-enable setting, and sensor fault state. GPIO editing should not be on the main dashboard surface.

The Hardware/Install page should remain owner-accessible but feel intentionally advanced. It should be optimized for short mobile use during wiring: a wiring-style visualization first, then a pin summary table, then a secondary technical pinout tab for debugging. GPIO edits should use safe dropdown enums only, require a checkbox confirmation, save as pending changes, and show that reboot is required before the wiring map becomes active.

Temperature cooling should be presented as a separate channel from pump control. The default daily controls are `Auto`, `Force OFF`, and a short `Test ON` action for wiring verification. `Test ON` should return to the previous mode after timeout and should not persist across reboot.
