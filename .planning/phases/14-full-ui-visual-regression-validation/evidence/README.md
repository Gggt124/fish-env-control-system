# Phase 14 Evidence Inventory

Date: 2026-06-03

## Environment

| Item | Value |
|------|-------|
| Project | Fish Pump Relay Timer Control |
| Browser target | `http://fish-pump.local/`, then `http://192.168.4.1/` probe |
| Browser tool | Codex in-app Browser |
| Desktop viewport | 1440x900 |
| Mobile viewport | 375x812 |
| Build command | `.\scripts\build.ps1` |
| Evidence labels | `device-backed`, `source-inspected`, `build-backed`, `not-run` |

## Evidence Label Rules

- `device-backed`: observed through the running ESP32 web server or device API without changing pump/cooling outputs.
- `source-inspected`: verified against repository source files only.
- `build-backed`: verified by ESP-IDF build output or generated binaries.
- `not-run`: unavailable, unsafe, or blocked; includes reason.

## Artifacts

| Artifact | Label | Purpose |
|----------|-------|---------|
| `browser-checks.md` | `device-backed`, `source-inspected`, `not-run` | In-app Browser login/page/viewport checks, screenshot status, stale-device finding. |
| `screenshots/desktop/debug-login-chrome.png` | `device-backed` fallback | Login screenshot captured by prior Chromium fallback before profile cache cleanup. |
| `footprint.md` | `build-backed`, `source-inspected` | Offline dependency, static bytes, ESP-IDF build, partition headroom. |
| `accessibility-checks.md` | `device-backed`, `source-inspected`, `not-run` | A11Y source/browser checklist. |
| `ui-review-closeout.md` | `source-inspected`, `device-backed` | Closing `impeccable` and `ui-ux-pro-max` review. |
| `hardware-regression.md` | `device-backed`, `not-run` | Hardware regression rows, with hardware-impacting actions left for human testing. |

## Known Limitations

- The in-app Browser rendered and interacted with `fish-pump.local`, but `tab.screenshot()` timed out with `Page.captureScreenshot`. Only one fallback Chromium login screenshot is available.
- The ESP32 device served stale static assets compared with the repository build: device `/style.css` lacks `.app-sidebar.open` and `aria-expanded` support present in `main/static/style.css` and `main/static/app.js`.
- After the build cycle, `fish-pump.local` stopped resolving in the in-app Browser and `192.168.4.1` was not reachable from the current network context. Later hardware/API checks are marked `not-run` unless already observed.
- No relay, timer, float-switch, cooling relay, or pump output action was triggered by this validation pass.

## Privacy Review

Screenshots and browser notes contain only local UI text, default username visibility, and device UI state. No Wi-Fi passwords, private SSIDs, or external account data are recorded.
