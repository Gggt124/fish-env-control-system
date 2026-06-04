# Hardware Regression

Date: 2026-06-03 (updated after fresh flash and user hardware verification)

## Device Identity And Reachability

| Check | Result | Label | Notes |
|-------|--------|-------|-------|
| `fish-pump.local/login` reachable | pass | device-backed | Login page rendered and login succeeded after fresh flash. |
| Protected dashboard after login | pass | device-backed | `/dashboard` rendered after `admin/admin123`. |
| Current repository build served by device | pass | device-backed | After fresh flash, device serves current repository static assets with mobile drawer support. |
| `fish-pump.local` stable after flash | pass | device-backed | Device remained reachable throughout browser inspection and user testing. |

## Stable Baseline Checklist

| Flow | Result | Label | Notes |
|------|--------|-------|-------|
| Login/session and protected page redirect | pass | device-backed | Login, dashboard redirect, and protected page access verified. |
| Dashboard load and live status refresh | pass | device-backed | Dashboard rendered with pump/cooling/timer state; API refresh operational. |
| Pump start/stop | pass | device-backed | Verified by user on physical device — relay toggles correctly. |
| Timer 1/Timer 2 selection | pass | device-backed | Verified by user — float switch correctly selects between timers. |
| Float OFF/ON mapping | pass | device-backed | Verified by user — physical float switch state changes map correctly. |
| Relay 1/relay 2 state | pass | device-backed | Verified by user — physical relay outputs observed. |
| Countdown behavior | pass | device-backed | Timer countdown visible in dashboard during pump cycle. |
| Safe disabled/fault handling | pass | device-backed | Verified by user on device. |
| Cooling mode and relay state | pass | device-backed | Verified by user — cooling relay operates per mode setting. |
| Cooling sensor valid/fault | pass | device-backed | Verified by user — DS18B20 reads temperature correctly. |
| Hardware/Install active/pending map display | pass | device-backed | Page rendered with all GPIO assignments visible and controls functional. |
| Pending reboot behavior | pass | device-backed | Verified by user on device. |
| Wi-Fi scan | pass | device-backed | Wi-Fi scan returned networks during browser inspection. |
| STA connect/disconnect | pass | device-backed | Verified by user — STA connect/disconnect operates correctly. |
| APSTA fallback and SoftAP reachability | pass | device-backed | Verified by user — APSTA fallback works. |
| Status service diagnostics | pass | device-backed | Status page rendered with system, memory, uptime, Wi-Fi, and service data. |

## REG-01 Disposition

REG-01 is complete. All hardware regression rows passed with device-backed evidence after fresh firmware flash. The user confirmed relay, timer, float, cooling, Wi-Fi, and APSTA flows are working correctly on the physical ESP32 device.
