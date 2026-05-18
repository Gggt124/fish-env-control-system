---
generated: 2026-05-18
focus: quality
---

# Conventions

## C Style

- Code uses C with ESP-IDF APIs and explicit component boundaries.
- Headers use `#pragma once` and `extern "C"` guards for C++ compatibility.
- Public APIs are small and prefix-scoped by component.
- File-local state is `static` and often uses the `s_` prefix.
- Log tags are `static const char *TAG = "...";`.
- ESP-IDF return codes are converted to booleans at component boundaries where the public API is simple.

## Component API Pattern

- Components expose one header and one implementation file, except `app_config`, which is header-only.
- Component dependencies are declared in each component `CMakeLists.txt`.
- Higher-level code should call wrapper APIs rather than raw storage or Wi-Fi internals. Example: `main/web_server.c` calls `wifi_manager_connect_sta()` instead of configuring ESP Wi-Fi directly for STA connect.

## Error Handling

- Startup uses fail-fast or log-and-continue depending on service criticality in `main/app_main.c`.
- `nvs_store_init()` erases and reinitializes NVS when ESP-IDF reports no free pages or a new NVS version.
- Most NVS wrappers return `false` on open/set/get failure and reset output buffers when a load misses.
- `wifi_manager_init()` uses `ESP_ERROR_CHECK()` for core ESP-IDF initialization steps.
- HTTP handlers return JSON error payloads for API failures and redirects for protected page failures.

## Memory Management

- Dynamic allocation is limited.
- `main/web_server.c` allocates cookie buffers and cJSON strings, then frees them in the same handler.
- `components/wifi_manager/wifi_manager.c` allocates scan results with `calloc()` during `WIFI_EVENT_SCAN_DONE` and frees them after copying into fixed-size scan result storage.
- Session storage is fixed-size: 4 in-memory entries in `components/session/session.c`.
- Wi-Fi scan result storage is fixed-size: `WIFI_SCAN_MAX` entries from `components/app_config/app_config.h`.

## Concurrency

- Shared session state is guarded by `s_session_mutex` in `components/session/session.c`.
- Wi-Fi state is guarded by `s_wifi_mutex` in `components/wifi_manager/wifi_manager.c`.
- Request-scoped Wi-Fi scan waiting uses a FreeRTOS binary semaphore in `main/web_server.c`.
- Captive DNS runs in its own FreeRTOS task in `main/dns_server.c`.

## HTTP Patterns

- Static files are served with `serve_static()` in `main/web_server.c`.
- JSON responses generally go through `send_json()`.
- Protected pages redirect unauthenticated users to `/login`.
- Protected APIs return `401 Unauthorized` JSON.
- Cross-origin POST mitigation checks `Origin` or `Referer` for AP IP, STA IP, or mDNS hostname.

## Frontend Patterns

- JavaScript is framework-free and uses `XMLHttpRequest`.
- `apiGet()` and `apiPost()` centralize transport behavior in `main/static/app.js`.
- Pages initialize based on `window.location.pathname`.
- User-visible UI text is mostly Thai with some English technical labels.
- Dynamic network list HTML uses `escHtml()` and `escJs()` helpers before insertion into `innerHTML`.

## Build And Encoding Practices

- Windows PowerShell sessions should use `chcp 65001` before ESP-IDF commands.
- `scripts/build.ps1` sets `PYTHONUTF8=1` before exporting ESP-IDF.
- Development notes warn against PowerShell regex rewrites on Unicode-heavy JS files.

## Documentation Pattern

- `README.md` is operator-facing and includes build, flash, manual test, project structure, and security notes.
- `docs/components.md` is component-facing and concise.
- `docs/development-notes.md` records concrete lessons from prior implementation/debugging work.
- `REFERENCE.md` should record any external repositories used as architecture or API references.

