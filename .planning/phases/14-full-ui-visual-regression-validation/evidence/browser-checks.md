# Browser Checks

Date: 2026-06-03

## Target Integrity

| Check | Result | Label | Evidence |
|-------|--------|-------|----------|
| Login route loads at `http://fish-pump.local/login` | pass | device-backed | Title `Fish Pump Control - Login`, body includes login form, viewport 1440. |
| Login with `admin/admin123` redirects to dashboard | pass | device-backed | Redirected to `http://fish-pump.local/dashboard`; no console warnings/errors. |
| Device static CSS matches repository source | fail | device-backed/source-inspected | Device `/style.css` length `35457`, SHA-256 `4d8b2aff...`, missing `.app-sidebar.open`; repo `main/static/style.css` length `36841`, includes `.app-sidebar.open`. |
| Device static JS matches repository source | fail | device-backed/source-inspected | Device `/app.js` length `77925`, SHA-256 `9418bd3...`; repo `main/static/app.js` SHA-256 `567412b...`. |

## Desktop 1440x900

| Page | Result | Console | Overflow | Primary Reachability | Label |
|------|--------|---------|----------|----------------------|-------|
| Login | pass | 0 issues | no | username/password/login visible | device-backed |
| Dashboard | pass | 0 issues | no | Sync, Hardware, Wi-Fi Setup visible; sidebar nav visible | device-backed |
| Hardware/Install | pass | 0 issues | no | Refresh, Dashboard, GPIO selects visible | device-backed |
| Wi-Fi | pass | 0 issues | no | Disconnect, scan, password form visible | device-backed |
| Status | pass | 0 issues | no | Refresh and sidebar nav visible | device-backed |

## Mobile 375x812

| Page | Result | Console | Overflow | Primary Reachability | Label |
|------|--------|---------|----------|----------------------|-------|
| Dashboard | partial | 0 issues | no | content reachable by scroll; drawer on device stale build fails to slide into viewport | device-backed |
| Hardware/Install | partial | 0 issues | no | content reachable by scroll; drawer on device stale build not verified open | device-backed |
| Wi-Fi | partial | 0 issues | no | page content visible; top action rendered as back arrow on stale device build | device-backed |
| Status | partial | 0 issues | no | content reachable by scroll; drawer on device stale build not verified open | device-backed |

## Drawer Finding

At 375px, clicking the hamburger on the device toggled `app-sidebar open` and `drawer-overlay open`, but computed sidebar transform remained `matrix(1, 0, 0, 1, -280, 0)` and the links stayed offscreen. Repository source already contains the expected `.app-sidebar.open { transform: translateX(0); }`, so this is treated as a stale-device-build blocker rather than a source-code regression.

## Representative States

| State | Result | Label | Evidence |
|-------|--------|-------|----------|
| Loading/login submit | pass | device-backed | Login button disabled during submit and redirected. |
| Error/alert hooks | pass | source-inspected | `login-error` and Wi-Fi reconnect banner use `role="alert"`. |
| Empty Wi-Fi list initial state | pass | source-inspected | `network-list` starts with a centered empty/loading row and `aria-live="polite"`. |
| Disabled controls | pass | source-inspected/device-backed | Pump/cooling buttons and Wi-Fi input panel expose disabled states. |
| Disconnected/connected Wi-Fi | partial | device-backed/source-inspected | Wi-Fi page rendered current controls; connect/disconnect was not triggered. |
| Sensor fault | not-run | not-run | Requires hardware/sensor manipulation. |
| Pending reboot | not-run | not-run | Requires saving pending hardware map and reboot-safe confirmation. |

## Screenshot Evidence

| Path | Status | Label | Notes |
|------|--------|-------|-------|
| `screenshots/desktop/debug-login-chrome.png` | captured | device-backed fallback | Login page rendered correctly. |
| desktop/mobile page screenshots | blocked | not-run | Codex in-app Browser `Page.captureScreenshot` timed out for `fullPage:false` and `fullPage:true`. |

## Commands And Observations

- In-app Browser viewport set to 1440x900 and 375x812 through the browser viewport capability.
- `tab.dev.logs({ levels: ["error", "warning"] })` returned 0 issues on observed pages.
- Horizontal overflow was checked with `document.documentElement.scrollWidth > document.documentElement.clientWidth`.
