# Phase 10 Research: Owner Dashboard, Hardware Install UI, And Validation

**Phase:** 10-owner-dashboard-hardware-install-ui-and-validation  
**Date:** 2026-05-24  
**Status:** Complete  
**Context note:** No `10-CONTEXT.md` exists. The user chose to continue without discuss-phase context, so this research uses `ROADMAP.md`, `REQUIREMENTS.md`, previous phase summaries, the owner/installer UI note, and current source contracts.

## Scope Read

Phase 10 is the owner-facing closeout for v1.1. It should consume the Phase 9 authenticated APIs and deliver two surfaces:

- Daily dashboard operation: pump enabled/running state, binary float state, active timer/relay, countdown, temperature, cooling relay state, threshold, auto-enable setting, and sensor fault state.
- Hardware/Install page: wiring-first visualization, active/pending GPIO summary, safe dropdown GPIO edits, reboot-required messaging, confirmation guardrails, and a secondary technical pinout view.

It should also validate the integrated v1.1 behavior with the ESP-IDF build and a hardware validation runbook/evidence path.

Out of scope remains unchanged: charts, WebSocket, MQTT/cloud, OTA UI, HTTPS, multi-user roles, new relay/timer runtime behavior, continuous water level, and freeform GPIO entry.

## Existing Implementation

### Current Frontend Shape

`main/static/dashboard.html` is already pump-first from Phase 4/7:

- Runtime panel IDs include `pump-running-label`, `pump-countdown`, `pump-active-timer`, `pump-phase`, `pump-float-state`, `pump-relay-state`, `pump-relay-1-state`, `pump-relay-2-state`, `pump-start-btn`, and `pump-stop-btn`.
- Timer settings form IDs include `timer1-on-min`, `timer1-on-sec`, `timer1-off-min`, `timer1-off-sec`, `timer2-on-min`, `timer2-on-sec`, `timer2-off-min`, `timer2-off-sec`, `timer1-start-phase`, `timer2-start-phase`, and `pump-auto-start`.
- Quick system status uses `/api/status` and IDs such as `dash-chip`, `dash-heap-pct`, `dash-rssi`, `dash-ap-clients`, and `dash-wifi-mode`.

`main/static/app.js` currently initializes only `/login`, `/dashboard`, `/status`, and `/wifi`. It consumes:

- `/api/pump/config`
- `/api/pump/status`
- `/api/pump/start`
- `/api/pump/stop`
- `/api/status`
- `/api/wifi/*`

It does not yet consume `/api/cooling/*` or `/api/hardware/map`.

### Authenticated API Surface Available For Phase 10

Phase 9 completed the API handoff:

- `GET /api/hardware/map` returns `active`, `pending`, `pending_valid`, `reboot_required`, active/pending status strings, and role-scoped `options`.
- `POST /api/hardware/map` requires auth, same-origin protection, `confirm_reboot_required:true`, role-safe GPIO values, and saves pending GPIO maps only.
- `GET /api/cooling/status` returns runtime mode, auto-enable, temperature validity, sensor state, fault, relay state, threshold/hysteresis, lockout, Test ON countdown, demand, blocked reason, DS18B20 GPIO, and cooling relay GPIO.
- `GET /api/cooling/config` returns persisted threshold, hysteresis, auto-enable, mode, test timeout, compressor min-off, cooling relay polarity, validation limits/enums, and nested runtime status.
- `POST /api/cooling/config` validates and saves cooling config, rejects persisted `test_on`, and applies runtime config from active hardware pins.
- `POST /api/cooling/mode` supports runtime `auto`, `force_off`, and `test_on`; Test ON does not persist.
- `/api/pump/config` supports `relay1_polarity`, optional `relay2_polarity`, legacy `relay_polarity`, timer start phases, and read-only hardware GPIO fields.

All new mutation APIs require `require_auth(req)` and `is_same_origin(req, false)`.

### Route And Embed Constraints

`main/web_server.c` currently embeds `login.html`, `dashboard.html`, `wifi.html`, `status.html`, `style.css`, and `app.js`. To add a Hardware/Install page:

- Add `static/hardware.html` to `main/CMakeLists.txt` `EMBED_FILES`.
- Declare `_binary_hardware_html_start` and `_binary_hardware_html_end` in `web_server.c`; the `static/` prefix is stripped from embedded symbols.
- Add a protected `GET /hardware` handler matching `/dashboard`, `/status`, and `/wifi` auth/redirect behavior.
- Register the `/hardware` route. `APP_TEMPLATE_HTTP_MAX_URI_HANDLERS` is already 32 after Phase 9, and the route table has room for one new page route.
- Update static page navigation in `dashboard.html`, `status.html`, and `wifi.html` so Hardware/Install is discoverable.
- Update `app.js` init routing with `path === '/hardware'`.

### UI Design Constraints

Project constraints require an offline local UI:

- No CDN, web fonts, external JS/CSS, Material Symbols, or package assets.
- Use plain HTML/CSS/vanilla JS with existing system font stack.
- Existing UI text is mostly Thai with English technical labels; new labels should match that blend.
- Use stable dimensions for dashboard and install status tiles so dynamic status text does not shift layout.
- Use controls appropriate to the domain: selects for GPIO options, toggles/checkboxes for binary settings, segmented/tabs for technical view switching, numeric inputs for threshold/time values, and clear buttons for commands.

## Recommended Plan Split

### Plan 10-01: Owner Dashboard Cooling Operation

Extend the existing pump dashboard instead of replacing it. Add a cooling operation band/cards below or beside the pump runtime section:

- Temperature with invalid/unknown state.
- Cooling relay ON/OFF.
- Cooling mode: Auto, Force OFF, Test ON.
- Sensor state/fault code.
- Threshold and hysteresis.
- Auto-enable on boot.
- Lockout/test countdown and blocked reason where useful.

Wire `/api/cooling/config`, `/api/cooling/status`, `/api/cooling/config` POST, and `/api/cooling/mode` POST. Preserve pump countdown behavior and pump config form.

### Plan 10-02: Hardware/Install Page

Add `main/static/hardware.html`, protected `/hardware` route, embed symbols, navigation entries, styles, and JS:

- Wiring visualization first: role labels, default pins, active pins, and clear connector hints for float, Relay 1, Relay 2, DS18B20, and cooling relay.
- GPIO summary next: active vs pending values, pending status, reboot-required status, and safe dropdown editors.
- Technical pinout as a secondary tab/view: safe option metadata and capability flags.
- Save flow posts `{ map, confirm_reboot_required: true }` only after an explicit confirmation checkbox.
- If no pending values exist, edit from active values. If pending values exist, show pending values as pending and make reboot-required messaging prominent.

### Plan 10-03: Validation, Docs, And Closeout

Run source checks and the ESP-IDF build, update operator/docs handoff, and capture hardware validation steps/evidence:

- Dual pump relays: Float ON -> Timer 1/Relay 1, Float OFF -> Timer 2/Relay 2, never both relays energized.
- DS18B20 fault: missing/unreadable sensor forces cooling relay OFF and dashboard shows fault.
- Cooling relay: threshold/hysteresis, Force OFF, Auto, Test ON, compressor lockout visibility.
- Hardware map: pending save, reboot-required display, active map unchanged before reboot, active map after reboot if flashed/tested.
- Wi-Fi/setup regression: SoftAP, login, dashboard, hardware page, status page, Wi-Fi scan/connect.

## Key Risks

- **Dashboard overload:** Phase 10 must add cooling status without turning the daily dashboard into an installer panel. GPIO editing belongs on `/hardware`.
- **API shape mismatch:** Cooling config POST requires full config fields, not just changed UI fields. The frontend should keep loaded config state and submit all required fields.
- **Test ON persistence:** UI must call `/api/cooling/mode` for Test ON and never save `mode:"test_on"` through `/api/cooling/config`.
- **Pending GPIO confusion:** Hardware page must make active vs pending values obvious and state that pending values do not drive runtime until reboot.
- **Embedded symbol naming:** New hardware page symbols must be `_binary_hardware_html_*`, not `_binary_static_hardware_html_*`.
- **Manual validation dependence:** Build can validate repository integration, but relay/float/DS18B20 behavior needs flashed-device evidence.

## Verification Guidance

Focused checks:

- `rg "hardware.html|_binary_hardware_html|/hardware|handle_get_hardware" main/CMakeLists.txt main/web_server.c main/static`
- `rg "/api/cooling/config|/api/cooling/status|/api/cooling/mode|cooling-temperature|cooling-relay|cooling-sensor|cooling-auto" main/static`
- `rg "/api/hardware/map|confirm_reboot_required|pending_valid|reboot_required|hardware-map" main/static main/web_server.c`
- `rg "http://|https://|cdn|Tailwind|Google Fonts|Material Symbols" main/static` should return no matches.
- `.\scripts\build.ps1`
- `Test-Path build\fish_pump_relay_timer_control.bin`

Manual validation should be recorded in the Phase 10 summary or verification artifact with exact wiring, COM port, firmware build, and observed dashboard/API behavior.

No external repositories were consulted for Phase 10 research, so no `REFERENCE.md` update is required.

## RESEARCH COMPLETE
