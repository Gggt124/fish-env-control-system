# Hardware Regression

Date: 2026-06-03

## Device Identity And Reachability

| Check | Result | Label | Notes |
|-------|--------|-------|-------|
| `fish-pump.local/login` reachable | pass initially | device-backed | Login page rendered and login succeeded. |
| Protected dashboard after login | pass initially | device-backed | `/dashboard` rendered after `admin/admin123`. |
| Current repository build served by device | fail | device-backed/source-inspected | Device static assets are stale compared with repository source. |
| `fish-pump.local` after build cycle | fail | not-run | Later in-app Browser navigation returned `ERR_NAME_NOT_RESOLVED`. |
| `192.168.4.1/login` SoftAP probe | fail | not-run | Current network context could not reach SoftAP IP. |

## Stable Baseline Checklist

| Flow | Result | Label | Notes |
|------|--------|-------|-------|
| Login/session and protected page redirect | partial | device-backed | Login and dashboard redirect passed before target became unreachable. Logout path was not rerun. |
| Dashboard load and live status refresh | partial | device-backed | Dashboard rendered, Sync button visible; API refresh not rerun after mDNS loss. |
| Pump start/stop | not-run | not-run | POST start/stop changes hardware output and was not triggered without explicit safe hardware confirmation. |
| Timer 1/Timer 2 selection | not-run | not-run | Requires float switch manipulation or safe simulator. |
| Float OFF/ON mapping | not-run | not-run | Requires physical float switch state changes. |
| Relay 1/relay 2 state | not-run | not-run | Requires observing physical relay outputs. |
| Countdown behavior | not-run | not-run | Requires running pump cycle on hardware. |
| Safe disabled/fault handling | not-run | not-run | Requires hardware state setup or deliberate fault simulation. |
| Cooling mode and relay state | not-run | not-run | POST mode/test can change output and was not triggered. |
| Cooling sensor valid/fault | not-run | not-run | Requires DS18B20 hardware/fault manipulation. |
| Hardware/Install active/pending map display | partial | device-backed | Page rendered and controls visible; pending save/reboot not triggered. |
| Pending reboot behavior | not-run | not-run | Requires saving pending hardware map and reboot path. |
| Wi-Fi scan | not-run | not-run | Not triggered before target resolution failed. |
| STA connect/disconnect | not-run | not-run | Would change network state; not triggered. |
| APSTA fallback and SoftAP reachability | not-run | not-run | `192.168.4.1` not reachable from current browser context. |
| Status service diagnostics | partial | device-backed | Status page rendered at desktop; later API checks blocked by resolution loss. |

## REG-01 Disposition

REG-01 is not complete. Browser evidence shows the UI can initially reach the device, but relay, timer, float, cooling, Wi-Fi mutation, and APSTA fallback regression checks need a manual hardware run against the freshly built firmware. No browser-only or source-only row is used as hardware proof.
