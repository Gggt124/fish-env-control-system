# Coding Conventions

**Analysis Date:** 2026-06-14

## Naming Patterns

**Files:**
- C sources: lowercase snake_case (e.g., `web_server.c`, `nvs_store.h`)
- Static assets: lowercase single-word or snake_case (e.g., `dashboard.html`, `app.js`)
- Python tests: `test_ui_phase[N].py`

**Functions:**
- Public Component APIs: prefix-scoped by component using snake_case (e.g., `wifi_manager_init()`, `session_validate()`, `pump_control_start()`)
- Private helper functions: lowercase snake_case without prefix (e.g., `confirm_timer_cb()`, `apply_pump_settings_to_config()`)
- Frontend JS: camelCase (e.g., `apiGet()`, `fadeSwap()`, `escHtml()`)

**Variables:**
- File-local C globals: prefixed with `s_` (e.g., `s_confirm_timer`, `s_http_server_retry`)
- Multi-core shared states: prefixed with `g_` or `s_` depending on scope (e.g., `g_cancel_rollback_timer`)
- Log tag strings: defined as `static const char *TAG = "file_basename";`

**Types:**
- Enum/Struct declarations: lowercase snake_case ending with `_t` (e.g., `wifi_scan_entry_t`, `nvs_store_pump_settings_t`)
- Enum values: UPPERCASE with prefix (e.g., `NVS_STORE_PUMP_SETTINGS_LOADED`, `PUMP_CONTROL_PHASE_ON`)

## Code Style

**C Formatting:**
- Indentation: 4 spaces
- Semicolons: Required
- Braces: Egyptian brackets style (inline open, new-line close)
- C++ Compatibility: Public headers wrapped in `extern "C"` guards and `#pragma once`

**Frontend Web:**
- Indentation: 2 spaces or 4 spaces depending on file context
- CSS Colors: Named design system CSS tokens (e.g., `var(--surface-container-low)`, `var(--radius-lg)`)
- Accessibility: Touch targets require a minimum of 44px in width/height.
- Motion: Respect user accessibility preferences via `@media (prefers-reduced-motion: reduce)` block resetting transitions to `0s`.

## Import Organization

**C Headers:**
1. Component header (e.g., `web_server.h` first inside `web_server.c`)
2. Local component dependencies (e.g., `app_config.h`, `nvs_store.h`)
3. Standard C library headers (`<stdbool.h>`, `<stdint.h>`, `<stddef.h>`)
4. ESP-IDF driver and system headers (`"esp_log.h"`, `"esp_wifi.h"`, `"esp_err.h"`)
5. FreeRTOS specific headers (`"freertos/FreeRTOS.h"`, `"freertos/task.h"`)

## Error Handling

**Firmware (C):**
- **Boundary conversion:** Convert ESP-IDF `esp_err_t` return codes to boolean `true` (success) / `false` (failure) at component boundaries to simplify caller logic.
- **Fail-safe bootloader:** If NVS data is corrupt, erase NVS partition, reload standard presets, and continue without crashing.

**REST APIs (JSON):**
- Errors return a JSON payload with `ok: false` and a localized/technical description, responding with `401 Unauthorized` for auth failure and `400 Bad Request` or `500 Internal Error` for others.

## Memory Management

- **Heap allocations:** Highly restricted. Allocate and release buffers within the scope of the same handler block. Avoid long-lived dynamic allocations.
- **Static buffers:** Static arrays are sized at compile-time using limits defined in `app_config.h` (e.g., `WIFI_SCAN_MAX` set to 20, 4 session slots).

## HTTP & Security Patterns

- **Static serving:** Served in chunks using `serve_static()` to save heap RAM.
- **Session cookie:** Uses a non-HttpOnly cookie named `session` allowing frontend JS to check status before drawing panels.
- **CSRF/CORS checks:** Incoming state-modifying POST requests are validated by comparing `Origin` or `Referer` headers against the SoftAP IP, station IP, or mDNS domain.
- **XSS Protection:** Frontend must clean any dynamic variables using `escHtml()` and `escJs()` before DOM injections.

---

*Convention analysis: 2026-06-14*
*Update when patterns change*
