# Phase 11 UI Evidence Inventory

## Purpose

This evidence pack records the shipped `main/static/` UI baseline before Phase
12 and Phase 13 implementation changes. Evidence labels are intentionally
strict:

| Label | Meaning |
| --- | --- |
| `device-backed` | Observed against a flashed ESP32 and its real APIs. |
| `simulated` | Observed from a local static server or controlled fixture. |
| `source-inspected` | Verified from committed HTML, CSS, or JavaScript only. |
| `not-run` | Not observed at runtime. The reason is recorded. |

Source inspection and simulated output do not prove ESP32 runtime behavior.

## Reproduction Commands

```powershell
node "C:\Users\Copter\.agents\skills\impeccable\scripts\detect.mjs" --json main/static
rg -n "https?://|cdn|fonts\.googleapis|unpkg|jsdelivr" main/static
rg -n "aria-live|role=\"status\"|role=\"alert\"|focus-visible|prefers-reduced-motion|btn-sm|scan-btn" main/static
git diff --name-only -- main/static
Get-ChildItem -File -LiteralPath "main\static" | Measure-Object -Property Length -Sum
```

## Environment

- Review date: `2026-06-02`
- Repository branch: `codex/v1.2-owner-ui-polish`
- Shipped UI source: `main/static/`
- Static asset baseline: `159,436` bytes
- Latest available firmware binary baseline: `1,111,616` bytes
- Original runtime capture attempt: local static server on `127.0.0.1:8123`
- Original Browser plugin result: `not-run`, the browser connection failed
  twice during setup with `windows sandbox failed: spawn setup refresh`.
- Post-repair supplement: local static server on `127.0.0.1:8765`
- Post-repair Browser result: four privacy-reviewed `simulated` protected-page
  screenshots captured through the available Chrome extension backend at its
  default desktop viewport. See `../11-SCREENSHOT-SUPPLEMENT.md`.
- Mobile follow-up result: four privacy-reviewed `simulated` protected-page
  screenshots captured through the in-app browser at a `375x812` viewport.
- Desktop follow-up result: four privacy-reviewed `simulated` protected-page
  screenshots captured through the in-app browser at a `1440x1000` viewport.
- Device-backed follow-up result: four privacy-reviewed screenshots captured
  from the reachable flashed ESP32 at `http://fish-pump.local`: Dashboard
  running and Hardware active map at `1440x1000` and `375x812`.
- ESP32 device-backed run: `not-run`, no reachable test-device URL was
  available in this execution context.

## Screenshot Convention

When capture is available, store files under `evidence/screenshots/` as:

```text
<surface>--<state>--<viewport>--<source>.<image-extension>
```

Examples:

```text
login--default--1440--device-backed.png
wifi--empty--375--simulated.png
hardware--pending-reboot--375--device-backed.png
```

The post-repair supplement commits four JPEG screenshots. See
`evidence/screenshots/README.md` for the retained images, privacy decision, and
remaining capture backlog.

## Capture Index

| Surface | Desktop `1440px` | Mobile `375px` | Runtime state coverage | Reason when not run |
| --- | --- | --- | --- | --- |
| Login | Captured then excluded | `not-run` | Default captured but excluded; loading and auth error `not-run` | Default image exposed credential-like placeholders, so the privacy gate excluded it. |
| App Shell | `simulated` `1440x1000` | `simulated` `375x812` | Desktop sidebar and mobile topbar captured on protected pages. | Dynamic interaction remains unverified. |
| Dashboard | `device-backed` running `1440x1000` | `device-backed` running `375x812` | Static default plus live running pump and cooling runtime captured; loading, disabled, stopped, cooling sensor fault `not-run` | Remaining dynamic states require controlled conditions. |
| Hardware/Install | `device-backed` active map `1440x1000` | `device-backed` active map `375x812` | Static default plus live active map captured; pending map, pending reboot, save error `not-run` | Remaining dynamic states require controlled conditions. |
| Status | `simulated` `1440x1000` | `simulated` `375x812` | Live page reached but device-backed screenshot excluded; representative long values `not-run` | Live page contains real SSID and MAC values. |
| Wi-Fi | `simulated` `1440x1000` | `simulated` `375x812` | Live connected page reached but device-backed screenshot excluded; loading, empty, scan error, selected, connect feedback, disconnect feedback `not-run` | Live page contains a private SSID and STA IP address. |

Every shipped page remains covered by source inspection in
`11-BASELINE-UI-AUDIT.md`.

## Privacy Review

Screenshots must be reviewed before commit. Exclude or redact:

- Credentials and session cookies.
- Private SSIDs and Wi-Fi passwords.
- STA and AP MAC addresses.
- Local IP details beyond deliberate example values.
- Any device identifier that is not required to explain the finding.

Privacy result for the post-repair supplement: `pass` for the sixteen committed
protected-page screenshots. The Login screenshot was reviewed and excluded
because it displayed credential-like placeholders.

## Current Limitations

- Static responsive appearance is captured at explicit `1440x1000` and
  `375x812` viewports. Dynamic states remain deferred.
- Runtime interaction states remain `not-run`; source inspection identifies
  their code paths without claiming visual or hardware validation.
- The detector output is reproducible and committed as
  `impeccable-detector.json`.
