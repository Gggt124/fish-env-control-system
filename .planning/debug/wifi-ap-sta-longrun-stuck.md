---
status: resolved
trigger: "SoftAP signal becomes weak after long uptime and STA connect requires reboot before it works again."
created: 2026-05-31
updated: 2026-06-02
---

## Symptoms

- expected: SoftAP remains usable after long uptime and STA can reconnect without reboot.
- actual: SoftAP appears weak or unstable after long uptime and web refresh becomes slower. Repeated STA connect/disconnect now works and owner Disconnect clears credentials, but a long-running browser session eventually rebooted during refresh.
- errors: Earlier logs show `wifi:sta is connecting, return error`, STA disconnect reasons 36, 2, and 205, plus SoftAP client disconnect/reconnect churn. The long-running refresh log ends with static response send errors `104` and `11`, then `rst:0x8 (TG1WDT_SYS_RESET)`.
- timeline: Seen during long-running Phase 10 Wi-Fi and dashboard hardware validation.
- reproduction: Leave the ESP32 running for an extended period, use the SoftAP, then attempt to connect STA repeatedly.

## Current Focus

- hypothesis: The scoped Wi-Fi, APSTA scan, and HTTP transport mitigations remain stable under real use. Any remaining errors should be bounded client-session churn or external hardware transients rather than an uptime-triggered firmware failure.
- test: Continue the serial soak while exercising dashboard refreshes, navigation, scans, and explicit STA forget/reconnect transitions. Audit reset, watchdog, CPU, heap, radio, HTTP, pump, and cooling diagnostics together.
- expecting: No reset, watchdog trip, heap leak, scan failure, or spontaneous STA disconnect. Browser socket failures may occur when clients or interfaces close, but sessions must drain and the server must recover without reboot.
- next_action: Debug session closed after the passing 13+ hour soak. If weak SoftAP or STA failure recurs in a future build, record the exact time before rebooting and compare Windows RSSI with `[WIFI_RADIO]`, `[WIFI_DIAG]`, and `[HTTP_DIAG]`. If DS18B20 rediscovery repeats, inspect sensor power, ground, data wiring, and pull-up resistance.

## Evidence

- timestamp: 2026-05-31
  observation: Device logs show `wifi_mgr: Connecting to STA SSID`, immediately followed by `wifi_mgr: STA disconnected, reason=36` and `wifi:sta is connecting, return error`.
  implication: Multiple STA connect paths overlap while ESP-IDF is still processing the previous transition.
- timestamp: 2026-05-31
  observation: SoftAP client disconnects with reason 4 and reconnects while STA retry churn is active. Client association is logged as `bgn, 40U`.
  implication: APSTA radio churn and HT40 operation contribute to the user-visible SoftAP instability.
- timestamp: 2026-05-31
  observation: After repeated connect/disconnect cycles, Disconnect can fail to expose SoftAP. Reboot reconnects STA even though the owner pressed Disconnect before reboot.
  implication: Runtime disconnect does not clear NVS credentials, AP restoration trusts a potentially stale flag, and delayed Wi-Fi events/tasks need idempotent handling.
- timestamp: 2026-05-31
  observation: User confirmed repeated STA connect/disconnect works and explicit Disconnect clears credentials correctly after the latest fix.
  implication: The remaining issue is the independent long-uptime AP/web responsiveness path.
- timestamp: 2026-05-31
  observation: Heap stays near 197 KB for hours instead of decreasing monotonically. Immediately before reboot, repeated static send errors occur and free heap drops temporarily to 176124 bytes, followed by `rst:0x8 (TG1WDT_SYS_RESET)`.
  implication: Evidence does not match a steady RAM leak. ESP-IDF maps classic ESP32 reset reason `0x08` to interrupt watchdog; static response pressure and duplicate AP restore remain the scoped firmware risks.
- timestamp: 2026-06-01
  observation: A 13-hour soak of the first mitigation build completed without a reboot and heap recovered near 197 KB. The log still contains 9 failed static sends, 2 failed JSON sends, and two STA reason-8 fallback restores. Each restore starts DHCP twice.
  implication: The remaining instability is a failed-session cleanup and AP restore lifecycle issue, not a monotonic heap leak. ESP-IDF closes a session when the URI handler returns an error, so response helpers must propagate `ESP_ERR_HTTPD_RESP_SEND`.
- timestamp: 2026-06-01
  observation: Supplemental source audit found that `/api/wifi/scan` deletes its request-scoped semaphore after a 5-second timeout while `wifi_manager` retains the callback and stack context until `WIFI_EVENT_SCAN_DONE`. The manager also did not set `s_scan_in_progress` when starting the scan.
  implication: A late scan completion can call `xSemaphoreGive()` on a deleted semaphore and write to expired HTTP handler stack memory. Scan ownership, cancellation, and callback completion must be serialized inside `wifi_manager`.
- timestamp: 2026-06-01
  observation: ESP-IDF documents that `esp_wifi_scan_stop()` still produces `WIFI_EVENT_SCAN_DONE`. A new scan accepted before that canceled completion event is consumed could have its state cleared by the old event.
  implication: Scan cancellation needs a cleanup-pending state that rejects new scans until the canceled completion event has been handled.
- timestamp: 2026-06-01
  observation: The hardware soak reproduced the reported failure in under one hour. From 14:31:15 to 14:34:05, ten UI connect requests caused attempts to rise from 2 to 33 and disconnect events to rise from 2 to 36. Disconnect reasons included `2` (`WIFI_REASON_AUTH_EXPIRE`) and `205` (`WIFI_REASON_CONNECTION_FAIL`). Scan completion continued scheduling attempts after the retry count exceeded the configured limit, and scans attempted during active STA connect returned `ESP_ERR_WIFI_STATE`.
  implication: The remaining STA issue is an unbounded reconnect scheduling path, not uptime, temperature, CPU load, watchdog, or heap exhaustion.
- timestamp: 2026-06-01
  observation: Each `/api/wifi/connect` request held the HTTP handler for about 5010 ms. During reconnect churn, static responses slowed or returned `ESP_ERR_HTTPD_RESP_SEND`, while board diagnostics remained healthy: CPU load roughly 1-6%, TWDT `ESP_OK`, stable heap, and DS18B20 around 28-29 C.
  implication: The web UI hang is a secondary effect of the blocking connect endpoint plus APSTA radio churn.
- timestamp: 2026-06-01
  observation: The retry circuit-breaker build passed `.\scripts\build.ps1`, flashed successfully to COM5 with verified hashes, and booted with STA `192.168.1.40`, RSSI `-51 dBm`, `retry_blocked=false`, zero HTTP send failures, CPU below 1.1%, TWDT `ESP_OK`, and DS18B20 around 29.4-29.6 C.
  implication: The patched firmware is running with healthy baseline diagnostics. Manual failed-password and repeated connect/disconnect regression testing can proceed under the new soak logger.
- timestamp: 2026-06-01
  observation: After about five hours on the STA retry build, the user reported severe web sluggishness. STA remained connected at `192.168.1.40` with RSSI roughly `-52` to `-61` dBm, CPU stayed around 1-3%, TWDT stayed `ESP_OK`, and heap recovered to baseline after requests drained. From 19:36 to 19:42, static page and asset sends repeatedly blocked for roughly 8-13 seconds and failed with socket errors `11` and `104`. The HTTP counters showed nearly one accepted socket per request (`opened=1343`, `requests_started=1340`) because every response forced `Connection: close`.
  implication: The active bottleneck is transient HTTP socket backpressure amplified by forced TCP churn and retransmitting static assets, not STA reconnect churn, watchdog pressure, thermal load, or a monotonic memory leak.
- timestamp: 2026-06-01
  observation: The web transport optimization build passed `.\scripts\build.ps1`, `node --check main\static\app.js`, and `git diff --check`, then flashed successfully to COM5 with verified hashes. Local smoke tests showed `/style.css` `200 OK` with content-hash ETag followed by `304 Not Modified`, `/app.js` transferred in about 400 ms, and three sequential HTTP/1.1 requests reused one TCP connection (`connects=1`, then `0`, then `0`). New diagnostics reported `static_cache_hits=1`, `static_send_fail=0`, and `static_deadline_aborts=0`.
  implication: Conditional asset caching and persistent HTTP connections are active on-device. Browser soak validation can proceed under the new logger.
- timestamp: 2026-06-01
  observation: While the user reported weak SoftAP and failed STA connect, the ESP32 reported its associated SoftAP client at `-30` to `-36 dBm` and scanned `KRAPAO JADJAN_2.4G` at `-54` to `-55 dBm`, but Windows measured `FishPump-Setup` at only `-85` to `-86 dBm` (`14-15%`). STA attempts failed with reason `2` (`auth_expire`) followed by `205` (`connection_fail`).
  implication: The reported weak SoftAP was real and asymmetric: ESP32 receive remained healthy while its transmit path became weak. It was not a frontend display error, low router RSSI, CPU load, thermal load, heap exhaustion, or TWDT failure.
- timestamp: 2026-06-01
  observation: Reboot immediately restored `FishPump-Setup` to `-48 dBm` (`100%`) from the same Windows client. Runtime diagnostics showed `tx_power_qdbm=72`, `tx_power_err=ESP_OK`, and stable router RSSI near `-52` to `-55 dBm`. A controlled owner disconnect restored APSTA and logged `[WIFI_RADIO] reason=ap_restore ... requested_tx_power_qdbm=80 tx_power_qdbm=72 ...`; Windows then measured the restored SoftAP at `-47 dBm`.
  implication: The antenna path is not permanently damaged. The driver quantizes or limits the requested power to `72 qdBm` at runtime, and AP fallback now explicitly reapplies HT20 plus maximum TX power. Soak logs can distinguish driver state changes from an external RF or power-integrity fault.
- timestamp: 2026-06-01
  observation: Before scan dwell tuning, `/api/wifi/scan` took about `2836 ms`, found the router at `-55 dBm`, but dropped the SoftAP client with reason `3` and aborted the HTTP transport. After reducing active dwell to `30-60 ms` and adding `30 ms` home-channel dwell, three consecutive scans completed in about `1892-1895 ms`, found the router at `-50` to `-51 dBm`, returned HTTP results successfully, kept Windows associated at `-42` to `-44 dBm`, and ended with `ap_client_leave=0`.
  implication: Long all-channel scans were an independent setup-page reliability problem. Shorter per-channel dwell plus explicit home-channel dwell preserves the SoftAP session during network discovery.
- timestamp: 2026-06-02
  observation: The final radio and scan build ran for at least `13:34:11` under the serial logger without a reboot, panic, brownout, watchdog trip, assertion, stack overflow, or heap corruption. Across 1622 cooling, pump, board, Wi-Fi, and HTTP diagnostic samples, CPU load stayed about `0.5-5.3%`, task watchdog status stayed `ESP_OK`, main stack high-water mark stayed at or above `1796` bytes, Wi-Fi heap recovered to about `192 KB`, board internal heap recovered to about `232 KB`, and the largest internal free block stayed `110592` bytes.
  implication: The final build does not show an uptime-triggered CPU, watchdog, stack, fragmentation, or monotonic heap leak failure during the observed window.
- timestamp: 2026-06-02
  observation: Real STA use completed `3/3` requested connects and `9/9` scans with `start_fail=0`, `scan_fail=0`, and `ap_restore_fail=0`. The only two STA disconnect events were explicit owner forget operations with reason `8` (`assoc_leave`), followed by successful reconnects. Periodic radio diagnostics stayed at `tx_power_qdbm=72`, and associated STA RSSI stayed roughly `-68` to `-47 dBm`.
  implication: The retry circuit breaker, non-blocking connect endpoint, AP fallback restore, and scan dwell tuning remained stable during the soak. No spontaneous STA loss or radio setpoint degradation was captured.
- timestamp: 2026-06-02
  observation: HTTP diagnostics ended with `8176` completed requests, `7` handler failures, `6` static send failures, `1` JSON send failure, `173` static cache hits, and `0` static deadline aborts. The failures clustered during navigation or interface changes and matched peer closure or bounded socket send pressure (`errno 104`, `113`, and `11`). Active HTTP sockets reached the configured peak of `7`, then drained back to `0`; LwIP has `16` sockets available, HTTP LRU purge is enabled, static transfer duration is capped at `6000 ms`, and frontend XHR calls time out after `8000 ms` with in-flight guards on periodic polling.
  implication: Residual HTTP failures are bounded stale-client cleanup rather than a server leak or persistent hang. Raising the HTTP socket limit would not address the observed peer closure and slow-client errors, so no transport change is justified from this capture.
- timestamp: 2026-06-02
  observation: At `05:38:52`, the DS18B20 driver observed one `85.0 C` power-on value and returned `ESP_ERR_INVALID_STATE`. `cooling_control` rediscovered the GPIO 33 sensor in about `0.1` seconds and immediately resumed valid temperatures. All 1622 cooling diagnostic samples remained `fault=false`, `sensor=ok`, and `temp_valid=true`.
  implication: Cooling recovery handled the transient safely and did not energize the cooling relay from the invalid reading. A repeated `85.0 C` event would indicate sensor power, ground, data wiring, or pull-up integrity worth checking on the physical installation.

## Eliminated

- hypothesis: Wi-Fi power save progressively weakens the AP signal.
  reason: `main/app_main.c` already applies `WIFI_PS_NONE`; the failure correlates with repeated STA transition logs instead.

## Resolution

- root_cause: `wifi_manager_connect_sta()` called `esp_wifi_disconnect()` and then immediately called `esp_wifi_connect()`, while `WIFI_EVENT_STA_DISCONNECTED` could also call `esp_wifi_connect()`. These overlapping transitions left the ESP-IDF STA state busy and caused APSTA radio churn. The local setup AP also allowed HT40 operation even though setup reliability matters more than throughput. Separately, Wi-Fi discovery used a full-channel active scan with `100-300 ms` dwell per channel and no explicit home-channel dwell, which could disconnect the SoftAP browser client before the scan response returned.
- fix: Route startup, API connect, retry, and post-scan reconnect through one delayed STA timer. Cancel pending timer work for explicit disconnect/forget and recover a busy ESP-IDF STA state by disconnecting before rescheduling. Configure APSTA bandwidth as HT20 and set maximum TX power explicitly. Make the owner Disconnect action clear saved STA settings, reconcile SoftAP against the actual Wi-Fi mode, ignore stale GOT_IP events after disconnect, skip stale AP auto-stop callbacks, and suppress overlapping deferred disconnect tasks. Make AP restore idempotent, stream embedded static assets in 2 KB chunks with scheduler delays, reduce static send blocking exposure, and add runtime diagnostics for reset reason, heap minimum, largest block, AP client RSSI, HTTP send failures, radio TX power, country limits, and per-network scan RSSI. Configure AP before `esp_wifi_start()`, keep that config across STA-to-APSTA restores, and reapply HT20 plus maximum TX power after restoring APSTA mode. Propagate JSON, static-file, and redirect send failures so ESP-IDF closes failed sessions. Make `wifi_manager` own asynchronous scan state, reject overlapping scans, cancel callbacks before timeout cleanup, clear ESP-IDF AP lists on aborted reads, reject new scans until a canceled `WIFI_EVENT_SCAN_DONE` is consumed, and recover STA reconnect after scan completion/cancellation. Reduce active scan dwell to `30-60 ms` and return to the SoftAP home channel for `30 ms` between scanned channels. Add a retry-blocked circuit breaker to every automatic reconnect path, reject scans during an active STA transition, deduplicate repeated STA connect submissions, return `/api/wifi/connect` immediately, and poll connection status from the Wi-Fi page. Allow HTTP/1.1 persistent connections instead of forcing `Connection: close`, enable TCP keep-alive cleanup, serve CSS and JS with content-hash ETags and `304 Not Modified`, reduce the socket send timeout to two seconds, abort static transfers after six seconds, expose cache-hit and deadline-abort counters, and reduce dashboard polling frequency while retaining browser-side countdown updates.
- verification: `.\scripts\build.ps1` passed after adding automatic `IDF_TOOLS_PATH` discovery for the installed `C:\Espressif` toolchain. `.\scripts\flash.ps1 -Port COM5` flashed and verified hashes successfully on ESP32-D0WD-V3 revision v3.1 for the first mitigation build. A 38-second serial read produced stable health samples: `Heap=195892`, `HeapMin=195000`, `Largest=110592`, `MainStackHWM=1972`. The final 2026-06-01 supplemental scan cleanup build also passes `.\scripts\build.ps1` with firmware size `0x10bb00`; `git diff --check` passes. The final supplemental build was flashed successfully to COM5 with all image hashes verified. A post-flash RTS reset reached `System Ready`, started SoftAP and DHCP once, and reported `Heap=195896`, `HeapMin=195004`, `Largest=110592`, `MainStackHWM=1972`. The retry circuit-breaker and non-blocking connect endpoint build passes `.\scripts\build.ps1`, `node --check main\static\app.js`, and `git diff --check`; firmware size is `0x10ee40`. It flashed successfully to COM5 with all hashes verified. Its five-hour capture is preserved at `logs\soak-sta-regression-20260601-144610.log`. The web transport optimization build passes the same checks with firmware size `0x10f280`, flashed successfully to COM5 with verified hashes, and logged to `logs\soak-web-transport-20260601-195113.log`. Local HTTP smoke tests verified ETag `304` revalidation, sub-second CSS/JS responses, TCP connection reuse, and zero initial HTTP send failures or deadline aborts. The radio diagnostics and APSTA scan dwell build passes `.\scripts\build.ps1` with firmware size `0x10f640`, passes `git diff --check`, and flashed successfully to COM5 with verified hashes. Three consecutive hardware scans returned successfully while retaining the SoftAP client. Its active capture at `logs\soak-wifi-scan-final-20260601-223025.log` passed a `13:34:11` real-use soak with three successful STA connects, two explicit owner forget/reconnect cycles, nine successful scans, no reboot, no watchdog trip, no spontaneous STA disconnect, stable radio setpoint, recovering heap, and bounded HTTP stale-client failures.
- files_changed:
  - components/app_config/app_config.h
  - components/wifi_manager/wifi_manager.c
  - components/wifi_manager/wifi_manager.h
  - main/app_main.c
  - main/web_server.c
  - main/static/status.html
  - main/static/wifi.html
  - main/static/app.js
  - README.md
  - docs/components.md
  - docs/development-notes.md
  - scripts/build.ps1
  - scripts/flash.ps1
  - .planning/phases/10-owner-dashboard-hardware-install-ui-and-validation/10-UAT.md
