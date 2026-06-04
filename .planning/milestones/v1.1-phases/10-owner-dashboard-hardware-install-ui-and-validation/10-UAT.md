---
status: complete
phase: 10-owner-dashboard-hardware-install-ui-and-validation
source:
  - 10-01-SUMMARY.md
  - 10-02-SUMMARY.md
  - 10-03-SUMMARY.md
started: 2026-05-24T00:00:00.000Z
updated: 2026-06-02T12:15:00.000+07:00
---

## Current Test
<!-- OVERWRITE each test - shows where we are -->

[testing complete]

## Tests

### 1. Dashboard Loads As Daily Operation Surface
expected: After flashing and logging in, `/dashboard` opens without console-visible page breakage. It shows the pump operation area, cooling operation area, and links to Status, Wi-Fi, and Hardware/Install.
result: pass

### 2. Pump Runtime Status Is Owner-Readable
expected: The dashboard shows pump enabled/running state, float state, active timer, active relay, countdown, Relay 1 state, Relay 2 state, and Start/Stop controls. The countdown stays readable and does not resize the page awkwardly.
result: pass

### 3. Cooling Runtime Status Is Owner-Readable
expected: The dashboard shows temperature or `--` when invalid, cooling relay state, mode, sensor state, fault state, threshold, hysteresis, boot auto-enable, blocked reason, lockout countdown, and Test ON remaining time when applicable.
result: pass

### 4. Cooling Config And Runtime Mode Controls Work
expected: Saving threshold, hysteresis, boot auto-enable, and Test ON timeout updates the dashboard from the device. Auto and Force OFF change runtime mode. Test ON starts as a bounded runtime action and is not saved as a boot mode.
result: pass

### 5. Hardware/Install Page Is Wiring-First
expected: `/hardware` opens after login and shows wiring summary first, active GPIO summary next, editable safe GPIO dropdowns, and technical pinout metadata after the primary install information.
result: pass

### 6. Hardware Map Save Uses Pending Reboot Guardrails
expected: GPIO fields are dropdowns only. Save is disabled until the reboot acknowledgement is checked. Saving a changed map shows pending GPIO values and a reboot-required message while active runtime GPIOs remain unchanged before reboot.
result: pass

### 7. Pending Hardware Map Applies Only After Reboot
expected: After rebooting with a saved pending map, the active GPIO summary reflects the new map and the reboot-required state clears. Runtime pump/cooling GPIO behavior follows the active map, not the old pending display.
result: pass

### 8. Pump Relay Hardware Behavior Matches Float State
expected: With safe relay polarity confirmed, Float ON selects Timer 1 / Relay 1 and Float OFF selects Timer 2 / Relay 2. Relay 1 and Relay 2 are never energized together, and Stop forces both pump relays OFF.
result: pass

### 9. Cooling Hardware Behavior Is Safe
expected: A missing or unreadable DS18B20 forces the cooling relay OFF and shows sensor fault. A valid sensor reading follows threshold/hysteresis behavior. Force OFF keeps the relay OFF, and Test ON is bounded and lockout-aware.
result: pass

### 10. Wi-Fi And Status Pages Still Work
expected: `/status` still reports system/Wi-Fi/device state. `/wifi` still scans networks, can connect STA with saved credentials, and AP fallback remains available if STA fails.
result: pass
previous_issue: "พอเปิดนานๆแล้ว webapp จะมีปัญหา เหมือน error หรือ crash; logs show httpd_accept_conn error 23, send error 11, and POWERON_RESET after long run"
resolution: "Verified after scoped Wi-Fi/APSTA/HTTP fixes with a 13:38:10 real-use soak: no reboot, watchdog trip, spontaneous STA disconnect, scan failure, or monotonic heap leak. Three STA connects, two explicit forget/reconnect cycles, and nine scans completed successfully. Residual client socket closures remained bounded and drained without server failure."

## Summary

total: 10
passed: 10
issues: 0
pending: 0
skipped: 0
blocked: 0

## Resolved Gaps

- truth: "Long-running web dashboard/status polling remains responsive without exhausting ESP32 HTTP/LwIP sockets."
  status: resolved
  reason: "User reported long-running webapp errors/crash. Logs include httpd_accept_conn accept error 23, httpd send error 11, and later POWERON_RESET."
  severity: major
  test: 10
  root_cause: "HTTP server allowed the default 7 open sockets while captive DNS also uses a socket, leaving little or no LwIP descriptor headroom. Frontend /api/status polling also had no timeout or in-flight guard, so weak or stale browser connections could accumulate over hours. A separate APSTA issue allowed the API connect path and WIFI_EVENT_STA_DISCONNECTED handler to call esp_wifi_connect concurrently after an asynchronous disconnect, leaving STA busy until reboot and causing SoftAP radio churn. Latest soak evidence rules out a monotonic RAM leak: heap returns near 197 KB for hours, but weak-link refreshes produce static send errors and temporary heap pressure before classic ESP32 interrupt-watchdog reset `TG1WDT_SYS_RESET`. Embedded static assets were still sent as one uninterrupted response and fallback AP restoration could still run twice."
  artifacts:
    - components/app_config/app_config.h
    - main/web_server.c
    - main/static/app.js
    - docs/components.md
    - docs/development-notes.md
  verification:
    - "Final capture: logs/soak-wifi-scan-final-20260601-223025.log"
    - "Duration: at least 13:38:10 under real use"
    - "STA connects: 3/3; explicit forget/reconnect cycles: 2/2; scans: 9/9"
    - "No reboot, panic, brownout, watchdog trip, spontaneous STA disconnect, scan failure, or monotonic heap leak"
    - "HTTP: 8176 completed requests, 7 bounded stale-client handler failures, 0 static deadline aborts; active sockets drained from peak 7 to 0"
  debug_session: "Added frontend XHR timeouts plus /api/status in-flight guards, sends Connection: close/no-store headers, added /favicon.ico 204 response, routed captive probe paths like /connecttest.txt to /login instead of default 404, increased LwIP socket budget to 16, restored HTTP concurrency to 7 sockets, and changed /api/wifi/disconnect to respond before deferred STA disconnect so the UI does not report false failure. Long-uptime AP instability and STA reconnect requiring reboot were traced to overlapping esp_wifi_connect calls from the API and disconnect event paths. STA connect/retry now goes through one deferred timer, pending attempts are canceled by explicit disconnect/forget, APSTA bandwidth is fixed to HT20, and maximum TX power is set explicitly. Scan results now mark the active STA SSID as connected, render it as a disabled status row instead of asking for its password again, and short-circuit redundant direct API reconnect requests. The Wi-Fi form now includes an explicit show-password checkbox for mobile browsers. Repeated disconnect testing exposed another race: the owner Disconnect action now clears saved STA settings, restores SoftAP from the actual Wi-Fi mode, ignores stale GOT_IP events after disconnect, skips AP auto-stop when STA is no longer connected, and suppresses overlapping deferred disconnect tasks. Latest soak evidence showed stable heap rather than monotonic RAM loss, but weak-link refreshes ended in static send errors and `TG1WDT_SYS_RESET`, which ESP-IDF maps to interrupt watchdog on classic ESP32. Static assets now stream in 2 KB chunks with scheduler delays, socket send timeout is reduced to 5 seconds, AP restore is idempotent, and status diagnostics expose heap minimum/largest block, AP client RSSI, and HTTP send-error counters."
