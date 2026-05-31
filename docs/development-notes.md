# Development Notes

## ESP-IDF Build

### ✅ DO
- Run `idf.py reconfigure` after adding or changing `idf_component.yml` — CMake won't pick up new component dependencies until regenerated
- Read `build/log/idf_py_stderr_output_*` and `build/log/idf_py_stdout_output_*` for build errors — `idf.py build` stdout truncation hides the actual error lines
- Create `idf_component.yml` manually before `idf.py add-dependency` — the CLI tool requires the manifest file to already exist

### ❌ DON'T
- Don't pipe `idf.py build` output through `Select-Object -Last` — error messages are in stderr/log files, not on the last N lines of stdout
- Don't assume ESP-IDF 6.1 has built-in cJSON — v6.x removed `components/json/`, use `espressif/cjson` via component manager instead

### 💡 TIP
- `idf.py fullclean` then `idf.py build` forces a fresh CMake reconfigure, resolving stale dependency issues
- After `idf.py reconfigure` succeeds, component dependencies appear in `build/dependencies.lock`

## C Header Best Practices

### ✅ DO
- Include the exact IDF header that defines types used in your header — e.g. `#include "esp_wifi_types.h"` for `wifi_auth_mode_t`
- Make every header self-contained so it compiles when included alone

### ❌ DON'T
- Don't rely on transitive includes from `.c` files — a header must include its own type dependencies
- Don't rely on transitive includes from other components for new API calls — when adding `esp_timer_get_time()` to `web_server.c`, include `esp_timer.h` explicitly instead of assuming it comes through `wifi_manager.h`; `-Werror=implicit-function-declaration` will catch this at build time

---

## Frontend (JavaScript / Embedded Static Files)

### ✅ DO
- Use `[System.IO.File]::ReadAllText` with explicit UTF8 encoding when PowerShell `.Replace()` must modify JS files containing Unicode/Thai text — avoids encoding corruption
- When rewriting a corrupted embedded file, rebuild the entire file from scratch rather than surgical edits — embedded files are compact enough for full rewrites
- Make `apiPost`/XHR handlers parse JSON body on ALL HTTP status codes (not just 200) — non-200 responses like 429 carry actionable error data (`retry_after`, `error` type) that the callback needs

### ❌ DON'T
- Don't use PowerShell `-replace` with multi-line regexes on JS files containing Unicode — the string matching becomes unreliable and can silently replace the wrong code block
- Don't assume a JS function's error handler is the only place that needs updating when adding a new error type — also fix the transport layer (`apiPost`) to deliver the parsed response

### 💡 TIP
- Browser DevTools Network tab shows the raw response body even when JavaScript crashes — use it to verify server responses independently of frontend logic

---

## ESP-IDF Build Workflow (Windows)

### ✅ DO
- Include `#include` for every ESP-IDF API directly in the `.c` file that calls it — transitive includes from other components are not guaranteed and may break without warning
- Before adding features that need a new HTTP route, check both backend (is there a handler function?) AND frontend (is there a UI control to trigger it?) — missing disconnect button showed incomplete implementation

### ❌ DON'T
- Don't use the old 2-arg `esp_task_wdt_init(true, timeout_ms)` API in ESP-IDF 6.1 — it now takes a `esp_task_wdt_config_t` struct with `.timeout_ms`, `.idle_core_mask`, `.trigger_panic` fields

### 💡 TIP
- Every `idf.py` command needs `export.ps1` sourced in the same PowerShell invocation — export state does NOT persist between separate bash tool calls

---

## Wi-Fi Manager Logic

### ✅ DO
- When implementing `disconnect_sta()`, always check if the AP was auto-stopped and restore it — otherwise the ESP32 can end up in STA-only mode with no fallback AP
- `wifi_manager_forget_sta()` already had the AP restore pattern; use it as a reference when fixing `disconnect_sta()`
- Serialize STA connect attempts. `esp_wifi_disconnect()` completes
  asynchronously, and `WIFI_EVENT_STA_DISCONNECTED` can race with a direct
  `esp_wifi_connect()` call if both paths start a reconnect.
- Prefer HT20 for the local setup AP and set maximum TX power explicitly.
  Classic ESP32 APSTA uses one shared radio, so repeated STA channel/auth
  churn can make the SoftAP appear weak or unstable to the installer.

### ❌ DON'T
- Don't assume `wifi_manager_disconnect_sta()` is enough by itself — if AP auto-stop timer already fired (`WIFI_MODE_STA` + `s_ap_enabled=false`), the disconnect leaves the device unreachable via AP
- Don't call `esp_wifi_connect()` directly from multiple event/API paths.
  Route retries through one deferred scheduler so the Wi-Fi driver cannot
  remain busy until reboot.

---

## Phase 6 Hardware Contract

### DO
- Treat `components/hardware_map` as the single source for safe GPIO options,
  role names, defaults, and map validation.
- Keep raw NVS keys inside `components/nvs_store`; higher layers should use
  typed load/save helpers.
- Preserve current runtime behavior until Phase 7: float GPIO and pump Relay 1
  feed the existing single-relay pump control, while Relay 2 and cooling relay
  remain stored/read-only.
- Keep pending hardware maps separate from active maps. Pending values require
  reboot before use.

### DON'T
- Don't duplicate GPIO safety lists in web handlers or UI code.
- Don't drive Relay 2, DS18B20, or cooling relay from Phase 6 integration code.
- Don't accept hardware or cooling mutation fields in `/api/pump/config`; Phase
  9 owns authenticated mutation APIs.

---

## Phase 7 Dual Relay Pump Runtime

### DO
- Use the v1.1 mapping everywhere: Float ON -> Timer 1 / Relay 1, Float OFF -> Timer 2 / Relay 2.
- Treat `active_relay`, `relay1_energized`, and `relay2_energized` as the source of truth for manual relay validation; `relay_energized` remains a compatibility alias only.
- Keep stopped-state preview separate from running behavior. The controller may report a ready `active_timer`, `active_relay`, `phase`, and `countdown_sec` while `running=false`, but both relay energized fields must stay false.
- Save Timer 1 and Timer 2 start phase as `on` or `off` through `/api/pump/config`; OFF is a real initial off phase, not a UI label.
- During hardware validation, confirm Relay 1 and Relay 2 never energize together and confirmed float transitions turn the old relay OFF before the new channel starts.

### DON'T
- Don't bring DS18B20 cooling runtime, cooling relay control, or hardware GPIO mutation APIs into Phase 7.
- Don't display Float state as unknown merely because the pump is stopped; the runtime keeps reading/debouncing float while stopped when configuration is valid.

---

## Phase 8 DS18B20 Cooling Runtime

### DO
- Keep `cooling_control` separate from `pump_control`; cooling relay state is not
  part of `/api/pump/status`.
- Use `/api/cooling/status` for validation fields: mode, auto-enable,
  temperature validity, sensor state, fault code, relay state, demand,
  blocked reason, lockout countdown, and test countdown.
- Treat missing/unreadable DS18B20 as safe-off: fault after 3 consecutive failed
  reads, recovery after 2 consecutive successful reads.
- Rediscover the one-wire bus after DS18B20 read failures. A sensor that was
  missing at boot or reconnected later must recover without reboot once reads
  succeed again.
- Preserve boot/reinit compressor protection. The cooling relay is treated as
  just turned OFF on runtime init, so Auto and Test ON wait for min-off.
- Keep Test ON runtime-only. It may be added to future mutation APIs, but it
  must not persist across reboot.

### DON'T
- Don't add `/api/cooling/config` or cooling POST routes before the Phase 9 API
  work.
- Don't allow Test ON to bypass compressor lockout.
- Don't rely on stale temperatures after a read failure or sensor fault.

---

## Phase 9 Authenticated Hardware And Cooling APIs

### DO
- Use `/api/hardware/map` for Hardware/Install UI data. The response includes
  active GPIO values, pending GPIO values, safe dropdown options, pending
  status, and reboot-required status.
- Save GPIO edits as pending values only. POST `/api/hardware/map` requires
  `confirm_reboot_required: true`, validates each GPIO against
  `hardware_map_options_for_role()`, and clears pending state when submitted
  values match the active map.
- Use `/api/cooling/config` for persisted cooling threshold, hysteresis,
  auto-enable, mode, test timeout, compressor minimum off-time, and cooling
  relay polarity.
- Use `/api/cooling/mode` for runtime Auto, Force OFF, and Test ON. Test ON is
  timeout-limited by `cooling_control` and is not saved to NVS.
- Use `/api/pump/config` for dual-channel pump settings. `relay1_polarity` and
  `relay2_polarity` are independent; legacy `relay_polarity` is a Relay 1
  compatibility alias.
- Keep all mutation routes behind `require_auth(req)` and
  `is_same_origin(req, false)`.

### DON'T
- Don't apply pending hardware map GPIOs to pump or cooling runtime before
  reboot.
- Don't persist `test_on` as a cooling config mode; route it through runtime
  mode control.
- Don't put GPIO mutation fields in `/api/pump/config`.
- Don't claim the owner dashboard or Hardware/Install UI is complete in Phase
  9. Phase 10 owns those frontend surfaces.

---

## Phase 10 Owner Dashboard And Hardware/Install UI

### DO
- Keep `/dashboard` focused on daily operation: pump state, float state, active
  timer/relay, countdown, cooling temperature, cooling relay, threshold,
  auto-enable, sensor fault, lockout, and bounded runtime mode actions.
- Use `/api/cooling/config` for persistent threshold, hysteresis, auto-enable,
  test timeout, min-off, and relay polarity. Use `/api/cooling/mode` for Auto,
  Force OFF, and Test ON runtime changes.
- Apply ordinary cooling config saves in place so saving settings does not
  restart the current lockout countdown. Only a real relay OFF transition or a
  full hardware reinit should start a fresh lockout window.
- Promote a valid pending hardware map during boot before pump and cooling
  runtime initialization, then clear pending state. Runtime GPIOs should never
  continue using the old active map after a successful reboot with pending
  installer changes.
- Keep Test ON runtime-only. Do not save `mode: "test_on"` through
  `/api/cooling/config`.
- Keep `/hardware` wiring-first. Render active GPIOs separately from pending
  GPIOs, and explain that pending values need reboot before becoming active.
- Populate Hardware/Install dropdowns from `/api/hardware/map` option arrays
  instead of duplicating safe GPIO lists in HTML or JavaScript.
- Keep frontend polling bounded. XHR requests need timeouts and in-flight
  guards so a weak Wi-Fi/browser connection cannot accumulate stale HTTP
  sockets over hours of dashboard use.
- Leave socket headroom for non-HTTP services by increasing the LwIP socket
  budget rather than starving browser concurrency. ESP-IDF's HTTP default of 7
  client sockets is useful for page refreshes because browsers fetch HTML, CSS,
  JavaScript, and API JSON concurrently.
- Handle common captive-portal probe paths explicitly. Windows/macOS/Android
  may request `/connecttest.txt`, `/ncsi.txt`, `/hotspot-detect.html`, or
  `/generate_204` after joining the SoftAP; routing them avoids noisy default
  404 responses and keeps clients pointed at the local login page.
- For `/api/wifi/disconnect`, respond before changing STA state. If the request
  arrives over the STA interface, disconnecting first tears down the TCP path
  and the browser reports failure even though the device did disconnect.
- Mark the active STA SSID in `/api/wifi/scan` and render it as a disabled
  status row. Re-selecting the network that is already connected needlessly
  asks for a password and can restart APSTA radio transitions.

### DON'T
- Don't add freeform numeric GPIO entry to the frontend.
- Don't mix installer GPIO editing into the daily dashboard.
- Don't claim hardware validation passed from source/build evidence alone.
