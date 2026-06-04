# Browser Checks

Date: 2026-06-03 (updated after fresh flash)

## Target Integrity

| Check | Result | Label | Evidence |
|-------|--------|-------|----------|
| Login route loads at `http://fish-pump.local/login` | pass | device-backed | Title `Fish Pump Control - Login`, body includes login form. |
| Login with `admin/admin123` redirects to dashboard | pass | device-backed | Redirected to `http://fish-pump.local/dashboard`; no console warnings/errors. |
| Device static CSS matches repository source | pass | device-backed | After fresh flash, device serves current repository CSS with `.app-sidebar.open` and `aria-expanded`. |
| Device static JS matches repository source | pass | device-backed | After fresh flash, device serves current repository JS. |

## Desktop 1440x900

| Page | Result | Console | Overflow | Primary Reachability | Label |
|------|--------|---------|----------|----------------------|-------|
| Login | pass | 0 issues | no | username/password/login visible | device-backed |
| Dashboard | pass | 0 issues | minor right clip on Timer 2 config at full page width | Sync, Hardware, Wi-Fi Setup visible; sidebar nav visible | device-backed |
| Hardware/Install | pass | 0 issues | no | Refresh, Dashboard, GPIO selects visible | device-backed |
| Wi-Fi | pass | 0 issues | no | Disconnect, scan, password form visible | device-backed |
| Status | pass | 0 issues | minor right clip on 4th card column | Refresh and sidebar nav visible | device-backed |

## Mobile 375x812

| Page | Result | Console | Overflow | Primary Reachability | Label |
|------|--------|---------|----------|----------------------|-------|
| Dashboard | pass | 0 issues | no | cards stack vertically; timer countdown visible; Start/Stop well-spaced | device-backed |
| Hardware/Install | pass | 0 issues | no | cards stack vertically; wiring summary single-column; GPIO tables readable | device-backed |
| Wi-Fi | pass | 0 issues | no | connected status/disconnect button stack properly; scan/password form visible | device-backed |
| Status | pass | 0 issues | no | all cards stack vertically in single column; all data fully visible | device-backed |

## Mobile Drawer Finding (Post-Flash)

At 375px, clicking the hamburger button correctly opens the navigation drawer that slides in from the left. The drawer shows:
- "Fish Pump Control" branding with gear icon
- "Online" status indicator (green dot)
- All navigation links: Dashboard, System Status, Wi-Fi Settings, Hardware/Install, ออกจากระบบ (Logout)
- Active page highlighted in blue
- Drawer overlays main content correctly
- Navigation between pages via the drawer works flawlessly

**Previous stale-asset blocker is resolved.** The `.app-sidebar.open { transform: translateX(0); }` rule now applies correctly on the freshly flashed device.

## Representative States

| State | Result | Label | Evidence |
|-------|--------|-------|----------|
| Loading/login submit | pass | device-backed | Login button disabled during submit and redirected. |
| Error/alert hooks | pass | source-inspected | `login-error` and Wi-Fi reconnect banner use `role="alert"`. |
| Empty Wi-Fi list initial state | pass | source-inspected | `network-list` starts with a centered empty/loading row and `aria-live="polite"`. |
| Disabled controls | pass | source-inspected/device-backed | Pump/cooling buttons and Wi-Fi input panel expose disabled states. |
| Disconnected/connected Wi-Fi | pass | device-backed | Wi-Fi page rendered current connected status to "KRAPAO JADJAN_2.4G" with IP and disconnect button. |
| Sensor fault | not-run | not-run | Requires hardware/sensor manipulation. |
| Pending reboot | not-run | not-run | Requires saving pending hardware map and reboot-safe confirmation. |

## Screenshot Evidence

| Path | Status | Label | Notes |
|------|--------|-------|-------|
| `screenshots/login_desktop.png` | captured | device-backed | Login page clean centered card layout. |
| `screenshots/dashboard_desktop_v2.png` | captured | device-backed | Dashboard with pump/cooling/timer sections. |
| `screenshots/dashboard_mobile.png` | captured | device-backed | Dashboard mobile stacked layout. |
| `screenshots/hardware_desktop.png` | captured | device-backed | Hardware/Install wiring summary and GPIO map. |
| `screenshots/hardware_mobile.png` | captured | device-backed | Hardware/Install mobile single-column. |
| `screenshots/wifi_desktop.png` | captured | device-backed | Wi-Fi connected state, scan section. |
| `screenshots/wifi_mobile.png` | captured | device-backed | Wi-Fi mobile layout. |
| `screenshots/status_desktop.png` | captured | device-backed | Status cards overview. |
| `screenshots/status_mobile.png` | captured | device-backed | Status mobile stacked layout. |
| `screenshots/status_mobile_menu_open.png` | captured | device-backed | Mobile drawer open with navigation links. |

## Minor Desktop Overflow Notes

Two minor right-edge clipping issues at full page width on desktop:
1. `/dashboard` — Timer 2 configuration section clips slightly at right edge.
2. `/status` — 4th card column (Wi-Fi Station) text truncates when 4+ cards in a row.

These are cosmetic at wide desktop viewport and do not affect primary action reachability or mobile usability. Not classified as blockers.

## Commands And Observations

- Browser viewport set to 1440x900 and 375x812.
- Console logs checked — 0 errors, 0 warnings on all observed pages.
- Horizontal overflow checked — no overflow on mobile; minor desktop clipping noted above.
- Thai text renders correctly on all pages (labels, buttons, headings, status messages).
