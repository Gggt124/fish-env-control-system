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

### ❌ DON'T
- Don't assume `wifi_manager_disconnect_sta()` is enough by itself — if AP auto-stop timer already fired (`WIFI_MODE_STA` + `s_ap_enabled=false`), the disconnect leaves the device unreachable via AP

---
