# Production Readiness Plan

> เป้าหมาย: ยกระดับ `main_dashboard_mcu` จาก prototype สู่ production-grade firmware
> วิธีใช้: session แต่ละรอบให้ติ๊ก checkbox ที่ทำเสร็จแล้ว (`[ ]`) เพื่อให้ session ถัดไปรู้ความคืบหน้า

---

## P0 — Critical (Security vulnerabilities / crashes)

### P0-1: Add mutex to session system
- [x] `session.c` — add `#include "freertos/FreeRTOS.h"` + `#include "freertos/semphr.h"`
- [x] `session.c` — add `static SemaphoreHandle_t s_session_mutex`
- [x] `session.c` — `session_init()`: create mutex via `xSemaphoreCreateMutex()`
- [x] `session.c` — `session_create()`: lock → logic → unlock
- [x] `session.c` — `session_validate()`: lock → logic → unlock
- [x] `session.c` — `session_destroy()`: lock → logic → unlock
- [x] `session/CMakeLists.txt` — add `freertos` to `REQUIRES`
- [x] Build + manual test: login 2 tabs, refresh both, no crash

### P0-2: Add mutex to wifi_manager state
- [x] `wifi_manager.c` — add `#include "freertos/semphr.h"`
- [x] `wifi_manager.c` — add `static SemaphoreHandle_t s_wifi_mutex`
- [x] `wifi_manager.c` — `wifi_manager_init()`: create mutex
- [x] `wifi_manager.c` — guard all `s_*` state reads/writes in event handler
- [x] `wifi_manager.c` — guard all API functions (`start_ap`, `stop_ap`, `connect_sta`, etc.)
- [x] `wifi_manager.c` — guard all getter functions
- [x] `wifi_manager/CMakeLists.txt` — add `freertos` to `REQUIRES`
- [x] Build + test: AP starts, STA connects, scan works, no crash on rapid API calls

### P0-3: Rate limiting on /api/login
- [x] `web_server.c` — add `static int s_login_attempts` + `static int64_t s_login_block_until`
- [x] `web_server.c` — in `handle_api_login()`: check block, return 429 if blocked
- [x] `web_server.c` — on failure: increment, block 30s after 5 attempts
- [x] `web_server.c` — on success: reset counter
- [x] Build + test: 6 rapid wrong logins → 6th returns 429. Wait 30s → OK

### P0-4: Remove token from login JSON response
- [x] `web_server.c` — remove `cJSON_AddStringToObject(resp, "token", token);`
- [x] Build + test: login response is `{"ok":true,"username":"admin"}` only, frontend still works

### P0-5: Fix URL-encoded login body parser
- [x] `web_server.c` — add `url_decode()` helper: `+`→space, `%XX`→byte
- [x] `web_server.c` — replace manual `strstr()`/`memcpy()` parsing with `url_decode()`
- [x] Build + test: login with `%40`(`@`), `+` works. JSON login still works.

---

## P1 — High (Production hardening)

### P1-1: Boot resilience
- [x] `app_main.c` — Wi-Fi init failure: log but DON'T return (AP-only mode)
- [x] `app_main.c` — HTTP server failure: log but continue, retry in main loop
- [x] `app_main.c` — add `esp_task_wdt_init()` (30s watchdog, struct API)
- [x] `app_main.c` — main loop: `esp_task_wdt_reset()` every cycle
- [x] `web_server.c` — wrap `httpd_register_uri_handler()` with per-route error logging
- [x] Build + test: pull Wi-Fi antenna, boot → AP still available at 192.168.4.1

### P1-2: Input sanitization
- [x] `web_server.c` — add `is_valid_utf8_printable()` helper
- [x] `web_server.c` — validate username/password in login (reject control chars)
- [x] `web_server.c` — validate SSID (max 32), password (max 64), reject control chars
- [x] `web_server.c` — validate all cJSON string inputs for length before `strncpy`
- [x] Build + test: SSID with control chars rejected. Normal login/connect works.

### P1-3: Replace ESP_ERROR_CHECK with graceful handling
- [x] `wifi_manager.c` — replace `ESP_ERROR_CHECK` in scan, AP stop, disconnect with `if(...){log;return false;}`
- [x] `wifi_manager.c` — keep `ESP_ERROR_CHECK` only in `wifi_manager_init()` fatal paths
- [x] Build + test: Wi-Fi errors during scan/stop don't crash device

### P1-4: Cookie SameSite=Lax
- [x] `web_server.c` — login cookie: add `; SameSite=Lax`
- [x] `web_server.c` — logout cookie: add `; SameSite=Lax`
- [x] Build + test: login works, session persists, cross-site POST blocked

### P1-5: Origin/Referer check on POST APIs
- [x] `web_server.c` — add `is_same_origin()` helper
- [x] `web_server.c` — call in `handle_api_login()`, `handle_api_wifi_connect()`, `handle_api_logout()`
- [x] `web_server.c` — allow requests with no Origin/Referer (log warning)
- [x] Build + test: POST from browser works. POST from curl works (warns).

### P1-6: Fix session token generation entropy
- [x] `session.c` — review and fix `esp_random()` byte extraction if needed
- [x] (Currently correct: 4 calls × 2 bytes = 8 bytes. Documentation only.)

---

## P2 — Medium (Ops / Config / Testing)

### P2-1: NVS Encryption + Flash Encryption config
- [x] `sdkconfig.defaults` — add `CONFIG_SECURE_FLASH_ENC_ENABLED=y` + dev mode
- [x] `sdkconfig.defaults` — add bootloader size opts (compiled date off, log ERROR)
- [x] Create `partitions.csv` with PT offset 0x9000 + `nvs_key` partition (encrypted)
- [x] Build + encrypted-flash: boot OK, NVS keys generated, NVS encrypted

### P2-2: Unit test harness (SKIPPED — Phase 1 scope: no unit tests per AGENTS.md)

### P2-3: Graceful shutdown API (SKIPPED — no restart path in app_main, would be dead code)

### P2-4: Wi-Fi scan fix (SKIPPED — ESP32 native background scan works while STA connected)

### P2-5: Build verification script (SKIPPED — `idf.py build` is the standard workflow)

---

## P3 — Low / Phase 2 (Nice-to-have)

### P3-1: HTTPS support
- [ ] Generate self-signed certificate
- [ ] `web_server.c` — dual HTTP/HTTPS via `httpd_ssl_start()`
- [ ] `sdkconfig.defaults` — TLS config options
- [ ] Build + test: HTTPS works (browser warning expected in AP mode)

### P3-2: Secure Boot v2 + Flash Encryption Release
- [ ] Configure Secure Boot v2 in menuconfig
- [ ] Generate signing keys
- [ ] Sign firmware images
- [ ] Burn eFuses (one-time irreversible)
- [ ] Verify chain of trust on boot

### P3-3: OTA update
- [ ] New component `ota_service`
- [ ] `esp_https_ota` with signed image verification
- [ ] Web UI for firmware upload
- [ ] Rollback support

### P3-4: Multi-user / credential rotation
- [ ] NVS storage for users (username, hashed password, role)
- [ ] Password change API endpoint
- [ ] Admin/user role support
- [ ] Default credentials change on first login

### P3-5: WebSocket real-time dashboard
- [ ] Enable `CONFIG_HTTPD_WS_SUPPORT`
- [ ] WebSocket handler for push updates
- [ ] frontend: replace polling with WebSocket

### P3-6: Content-Security-Policy headers
- [ ] `web_server.c` — add CSP header to all responses
- [ ] `default-src 'self'; script-src 'self'; style-src 'self';`
- [ ] Test all pages work under CSP

---

## Execution Order

```
Session  1: P0-1  (session mutex)
Session  2: P0-2  (wifi_manager mutex)
Session  3: P0-3  (rate limiting)
Session  4: P0-4  (remove token from JSON)
Session  5: P0-5  (fix URL-encoded parser)
Session  6: P1-1  (boot resilience)
Session  7: P1-2  (input sanitization)
Session  8: P1-3  (graceful ESP_ERROR_CHECK)
Session  9: P1-4  (SameSite cookie)
Session 10: P1-5  (Origin check)
Session 11: P1-6  (entropy review)
Session 12-16: P2 items
Session 17+:    P3 items
```

> แต่ละ session: `[ ]` งานที่ทำเสร็จแล้ว → `idf.py build` → commit (optional)