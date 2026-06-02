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
- Runtime capture attempt: local static server on `127.0.0.1:8123`
- Browser plugin result: `not-run`, the browser connection failed twice during
  setup with `windows sandbox failed: spawn setup refresh`.
- ESP32 device-backed run: `not-run`, no reachable test-device URL was
  available in this execution context.

## Screenshot Convention

When capture is available, store files under `evidence/screenshots/` as:

```text
<surface>--<state>--<viewport>--<source>.png
```

Examples:

```text
login--default--1440--device-backed.png
wifi--empty--375--simulated.png
hardware--pending-reboot--375--device-backed.png
```

No screenshots are committed in this baseline pack because browser automation
could not start. The absent `evidence/screenshots/` directory is intentional.

## Capture Index

| Surface | Desktop `1440px` | Mobile `375px` | Runtime state coverage | Reason when not run |
| --- | --- | --- | --- | --- |
| Login | `not-run` | `not-run` | Default, loading, auth error `not-run` | Browser connection failed before navigation. |
| App Shell | `not-run` | `not-run` | Desktop sidebar and mobile topbar `not-run` | Browser connection failed before navigation. |
| Dashboard | `not-run` | `not-run` | Loading, disabled, pump state, cooling state, sensor fault `not-run` | Requires browser capture and API-backed state. |
| Hardware/Install | `not-run` | `not-run` | Active map, pending map, pending reboot, save error `not-run` | Requires browser capture and API-backed state. |
| Status | `not-run` | `not-run` | Default diagnostics and long-value wrapping `not-run` | Browser connection failed before navigation. |
| Wi-Fi | `not-run` | `not-run` | Disconnected, loading, empty, scan error, selected, connect feedback, disconnect feedback `not-run` | Requires browser capture and API-backed or simulated fixtures. |

Every shipped page remains covered by source inspection in
`11-BASELINE-UI-AUDIT.md`.

## Privacy Review

Screenshots must be reviewed before commit. Exclude or redact:

- Credentials and session cookies.
- Private SSIDs and Wi-Fi passwords.
- STA and AP MAC addresses.
- Local IP details beyond deliberate example values.
- Any device identifier that is not required to explain the finding.

Privacy result for this run: `pass`, no screenshots were produced or committed.

## Current Limitations

- Responsive appearance at `1440px` and `375px` remains a Phase 14 screenshot
  validation item.
- Runtime interaction states remain `not-run`; source inspection identifies
  their code paths without claiming visual or hardware validation.
- The detector output is reproducible and committed as
  `impeccable-detector.json`.
