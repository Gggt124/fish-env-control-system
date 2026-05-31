---
status: verifying
trigger: "SoftAP signal becomes weak after long uptime and STA connect requires reboot before it works again."
created: 2026-05-31
updated: 2026-05-31
---

## Symptoms

- expected: SoftAP remains usable after long uptime and STA can reconnect without reboot.
- actual: SoftAP appears weak or unstable after long uptime. Repeated STA connect attempts fail until reboot.
- errors: Logs show `wifi:sta is connecting, return error`, STA disconnect reasons 36, 2, and 205, plus SoftAP client disconnect/reconnect churn.
- timeline: Seen during long-running Phase 10 Wi-Fi and dashboard hardware validation.
- reproduction: Leave the ESP32 running for an extended period, use the SoftAP, then attempt to connect STA repeatedly.

## Current Focus

- hypothesis: `wifi_manager_connect_sta()` starts a new STA connection immediately after asynchronous disconnect while the disconnect event handler also starts another connection. The overlapping STA operations leave the Wi-Fi driver busy and disrupt the shared APSTA radio.
- test: Serialize all STA connect attempts through one delayed timer, cancel pending attempts for explicit disconnect/forget, force HT20 bandwidth for APSTA stability, and set maximum TX power explicitly.
- expecting: No `wifi:sta is connecting, return error` during normal reconnect. SoftAP remains more stable while STA transitions and STA reconnect recovers without reboot.
- next_action: Run repeated AP/STA reconnect hardware validation and observe SoftAP signal stability over time.

## Evidence

- timestamp: 2026-05-31
  observation: Device logs show `wifi_mgr: Connecting to STA SSID`, immediately followed by `wifi_mgr: STA disconnected, reason=36` and `wifi:sta is connecting, return error`.
  implication: Multiple STA connect paths overlap while ESP-IDF is still processing the previous transition.
- timestamp: 2026-05-31
  observation: SoftAP client disconnects with reason 4 and reconnects while STA retry churn is active. Client association is logged as `bgn, 40U`.
  implication: APSTA radio churn and HT40 operation contribute to the user-visible SoftAP instability.

## Eliminated

- hypothesis: Wi-Fi power save progressively weakens the AP signal.
  reason: `main/app_main.c` already applies `WIFI_PS_NONE`; the failure correlates with repeated STA transition logs instead.

## Resolution

- root_cause: `wifi_manager_connect_sta()` called `esp_wifi_disconnect()` and then immediately called `esp_wifi_connect()`, while `WIFI_EVENT_STA_DISCONNECTED` could also call `esp_wifi_connect()`. These overlapping transitions left the ESP-IDF STA state busy and caused APSTA radio churn. The local setup AP also allowed HT40 operation even though setup reliability matters more than throughput.
- fix: Route startup, API connect, retry, and post-scan reconnect through one delayed STA timer. Cancel pending timer work for explicit disconnect/forget and recover a busy ESP-IDF STA state by disconnecting before rescheduling. Configure APSTA bandwidth as HT20 and set maximum TX power to 80 qdBm units (20 dBm).
- verification: `.\scripts\build.ps1` passed and `.\scripts\flash.ps1 -Port COM5` flashed successfully. Hardware AP signal and repeated STA reconnect UAT remain pending.
- files_changed:
  - components/app_config/app_config.h
  - components/wifi_manager/wifi_manager.c
  - docs/components.md
  - docs/development-notes.md
  - .planning/phases/10-owner-dashboard-hardware-install-ui-and-validation/10-UAT.md
