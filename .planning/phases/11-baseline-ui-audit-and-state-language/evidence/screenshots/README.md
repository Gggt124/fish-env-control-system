# Screenshot Evidence And Backlog

The original Phase 11 run produced no screenshot images because the in-app
browser connection failed twice during setup with:

```text
windows sandbox failed: spawn setup refresh
```

After the Browser startup repair, a supplemental capture ran against a local
static server at `http://127.0.0.1:8765`. The available Chrome extension
backend did not advertise viewport override support, so retained images use
the honest `chrome-default` viewport label rather than claiming `1440px` or
`375px`.

On `2026-06-03`, the in-app browser route became available and captured the
same four protected-page baselines at a `375x812` mobile viewport. Full-page
screenshots are `371px` wide after scrollbar accounting. Sticky mobile headers
may repeat during full-page stitching on long pages; treat these as responsive
layout evidence rather than pixel-perfect visual references.

## Retained Simulated Evidence

| Surface | File | Notes |
| --- | --- | --- |
| Dashboard | `dashboard--default--chrome-default--simulated.jpg` | Static default layout and desktop sidebar. |
| Dashboard mobile | `dashboard--default--375--simulated.jpg` | Static mobile layout at `375x812`. |
| Hardware/Install | `hardware--default--chrome-default--simulated.jpg` | Static loading/default installer layout. |
| Hardware/Install mobile | `hardware--default--375--simulated.jpg` | Static mobile installer layout at `375x812`. |
| Status | `status--default--chrome-default--simulated.jpg` | Placeholder diagnostic layout. |
| Status mobile | `status--default--375--simulated.jpg` | Placeholder mobile diagnostics at `375x812`. |
| Wi-Fi | `wifi--default--chrome-default--simulated.jpg` | Static disconnected setup layout with deliberate example AP IP `192.168.4.1`. |
| Wi-Fi mobile | `wifi--default--375--simulated.jpg` | Static disconnected mobile setup layout at `375x812`. |

A Login default screenshot was captured and reviewed but not retained because
the static form displayed credential-like placeholders. These screenshots are
`simulated`; they do not prove API-backed or flashed-device behavior.

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

Capture each remaining reachable state at desktop `1440px` and mobile `375px`
widths.
Use the filename convention documented in `../README.md`.

## Privacy Gate

Before committing any screenshot:

- Remove or redact credentials and session cookies.
- Remove or redact private SSIDs and Wi-Fi passwords.
- Remove or redact STA and AP MAC addresses.
- Keep local IP details only when they are deliberate example values.

Post-repair privacy result: `pass` for the eight retained screenshots.
