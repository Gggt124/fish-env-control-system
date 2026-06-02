# Screenshot Capture Backlog

No screenshot images were produced during Phase 11 execution. The in-app
browser connection failed twice during setup with:

```text
windows sandbox failed: spawn setup refresh
```

The audit therefore labels all runtime capture rows `not-run`. Do not treat the
absence of images as runtime validation.

## Required Follow-Up

Phase 14 should capture privacy-reviewed evidence for:

- Login default, loading, and authentication error states.
- Desktop sidebar and mobile authenticated navigation.
- Dashboard loading, unavailable, running, stopped, and cooling sensor-fault
  states.
- Hardware/Install active map, pending map, pending reboot, save success, and
  save error states.
- Status diagnostics with representative long SSID, IP, MAC, firmware, and
  service values.
- Wi-Fi disconnected, scanning, empty, scan error, selected, connecting,
  success, connect error, and disconnect feedback states.

Capture each reachable state at desktop `1440px` and mobile `375px` widths.
Use the filename convention documented in `../README.md`.

## Privacy Gate

Before committing any screenshot:

- Remove or redact credentials and session cookies.
- Remove or redact private SSIDs and Wi-Fi passwords.
- Remove or redact STA and AP MAC addresses.
- Keep local IP details only when they are deliberate example values.
