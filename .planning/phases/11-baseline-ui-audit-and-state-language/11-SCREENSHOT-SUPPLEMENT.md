# Phase 11 Browser Screenshot Supplement

## Purpose

This supplement records the Browser evidence captured after repairing the
local Browser startup path. It does not re-run Phase 11 implementation work and
changes no file under `main/static/`.

## Capture Method

- Initial capture date: `2026-06-02`
- Responsive capture date: `2026-06-03`
- Initial source: local static server at `http://127.0.0.1:8765`
- Mobile source: local static server at `http://127.0.0.1:8766`
- Desktop source: local static server at `http://127.0.0.1:8767`
- Browser path: Browser plugin using the available Chrome extension backend
  for the initial desktop baseline and the in-app browser for responsive
  capture
- Evidence label: `simulated`
- Viewport labels: `chrome-default`, `1440`, and `375`

The in-app browser route was unavailable during the initial supplemental run.
It became available on `2026-06-03`, advertised viewport override support, and
captured screenshots at `1440x1000` and `375x812` viewports. Full-page JPEG
output is narrower on pages with a vertical scrollbar.

## Retained Evidence

| Surface | Screenshot |
| --- | --- |
| Dashboard desktop `1440x1000` | `evidence/screenshots/dashboard--default--1440--simulated.jpg` |
| Dashboard | `evidence/screenshots/dashboard--default--chrome-default--simulated.jpg` |
| Dashboard mobile | `evidence/screenshots/dashboard--default--375--simulated.jpg` |
| Hardware/Install desktop `1440x1000` | `evidence/screenshots/hardware--default--1440--simulated.jpg` |
| Hardware/Install | `evidence/screenshots/hardware--default--chrome-default--simulated.jpg` |
| Hardware/Install mobile | `evidence/screenshots/hardware--default--375--simulated.jpg` |
| Status desktop `1440x1000` | `evidence/screenshots/status--default--1440--simulated.jpg` |
| Status | `evidence/screenshots/status--default--chrome-default--simulated.jpg` |
| Status mobile | `evidence/screenshots/status--default--375--simulated.jpg` |
| Wi-Fi desktop `1440x1000` | `evidence/screenshots/wifi--default--1440--simulated.jpg` |
| Wi-Fi | `evidence/screenshots/wifi--default--chrome-default--simulated.jpg` |
| Wi-Fi mobile | `evidence/screenshots/wifi--default--375--simulated.jpg` |

The Login default screenshot was captured and reviewed but excluded from the
repository because its static placeholders exposed credential-like values.

## Privacy Review

Result: `pass` for the twelve retained screenshots.

- No session cookie is rendered.
- No private SSID or Wi-Fi password is rendered.
- No STA or AP MAC address is rendered.
- The Wi-Fi page renders only the deliberate local example IP `192.168.4.1`.
- Runtime values are placeholders rather than device identifiers.

## Remaining Validation

- Pixel-perfect desktop and mobile comparison where sticky headers are
  intentionally excluded from full-page screenshot stitching artifacts.
- Login evidence with credential placeholders removed or redacted.
- API-backed loading, success, error, empty, disabled, disconnected,
  connected, pending-reboot, and long-value states.
- Device-backed capture against a reachable flashed ESP32.
