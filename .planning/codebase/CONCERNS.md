---
generated: 2026-05-18
refreshed: 2026-06-02
last_mapped_commit: 3ef2ac063ebead27920e5e340508c371f689cdca
focus: concerns
scope: .gitignore,.graphify_detect.json,.planning,PRODUCT.md
---

# Concerns

## Summary

The codebase is intentionally a local setup prototype. Most concerns are acceptable for phase 1 if the firmware remains local-only, but they must be revisited before production or internet exposure.

## Security Boundaries

- Default credentials are compile-time constants in `components/app_config/app_config.h`.
- `README.md`, `AGENTS.md`, `main/app_main.c`, and `main/web_server.c` explicitly warn that default credentials must change before production.
- HTTP is plain text and the SoftAP is open, so login and Wi-Fi credentials can be observed by nearby clients during setup.
- Session cookies are intentionally non-HttpOnly and readable by JavaScript.
- NVS encryption, flash encryption, and secure boot are disabled in `sdkconfig.defaults`.
- CSRF mitigation is origin/referer based in `main/web_server.c`, not token based.

## Authentication Limitations

- `components/session/session.c` supports only 4 in-memory sessions.
- Sessions disappear on reboot, which is acceptable for local setup but not durable auth.
- Tokens are 8 random bytes rendered as 16 hex characters; adequate for a local prototype, but short for hardened production exposure.
- Login rate limiting uses static locals in `handle_api_login()` and is global rather than per-client.

## Wi-Fi And Availability Risks

- `components/wifi_manager/wifi_manager.c` uses `STA_MAX_RETRY 1`, so fallback behavior is quick but may be too aggressive on unstable networks.
- `/api/wifi/connect` waits 5 seconds inside the HTTP handler in `main/web_server.c`, tying up a server handler while STA connects.
- The Wi-Fi scan endpoint waits up to 5 seconds on a semaphore in `main/web_server.c`.
- AP auto-stop can make the fallback network unavailable after STA success; disconnect and failure paths restore it, but this behavior should be retested after changes.

## Concurrency And State Risks

- `wifi_manager_get_sta_ip()` and `wifi_manager_get_sta_ssid()` take and release the mutex, then return pointers to shared static buffers in `components/wifi_manager/wifi_manager.c`.
- `dns_server_is_running()` returns whether the task handle is non-null; if the DNS task exits after socket setup failure, the handle may not fully represent service health.
- `dns_server_stop()` can delete the DNS task while it may also be closing the socket, so future changes should be careful around task/socket lifecycle.
- Wi-Fi scan callback state is global in `components/wifi_manager/wifi_manager.c`; concurrent scan requests are not explicitly rejected before overwriting callback/context.

## Input Validation Gaps

- `is_valid_utf8_printable()` in `main/web_server.c` rejects control characters but does not fully validate UTF-8 sequences.
- Static IP input is validated by `ip4addr_aton()` in `components/wifi_manager/wifi_manager.c`, but frontend fields do not send DNS even though NVS supports it.
- DNS query parsing in `main/dns_server.c` handles simple query shape but does not deeply validate label lengths against malformed packets beyond bounds checks.

## Build And Portability Concerns

- The workflow assumes Windows PowerShell and an ESP-IDF install, commonly at `C:\esp-idf`.
- `sdkconfig` is checked in alongside `sdkconfig.defaults`; this can be useful for reproducibility but may drift from intended portable defaults.
- `managed_components/` and `build/` are present locally but should stay out of source review unless intentionally needed.

## Product Scope Risks

- Hardware relay/float behavior still needs manual Phase 5 validation on the ESP32 board even though firmware, APIs, and dashboard controls now build.
- Adding relay GPIO assignments before board variant and pin map are confirmed would violate the documented phase boundary.

## Code Hygiene Notes

- `components/wifi_manager/wifi_manager.c` uses `calloc()` and `free()` but does not explicitly include `<stdlib.h>` in the inspected source; add it if the current ESP-IDF build does not provide it transitively.
- Some HTTP success-like failures return `200 OK` with `{"ok":false,...}` in `main/web_server.c`; this is frontend-friendly but can make automated checks less precise.
- Frontend `innerHTML` is used for network list rendering in `main/static/app.js`; SSID values are escaped, so preserve `escHtml()` and `escJs()` if this UI is refactored.

## Production Hardening Backlog

- Unique credentials or first-boot credential setup.
- TLS or a clear local-only threat model.
- HttpOnly/Secure cookie strategy where JavaScript redirect logic no longer depends on reading the token.
- Flash encryption, NVS encryption, and secure boot.
- Stronger per-client rate limiting.
- Automated tests for session, input validation, Wi-Fi state transitions, and HTTP route behavior.

## Incremental Refresh: Recorded Follow-Up

This section was refreshed from `.planning/`, `.gitignore`,
`.graphify_detect.json`, and `PRODUCT.md` only. Source-level concerns above are
retained from the prior full-map scan and were not re-audited.

- `.planning/debug/wifi-ap-sta-longrun-stuck.md` is resolved after bounded
  diagnostics and a passing `13:38:10` real-use soak. Preserve its APSTA,
  asynchronous-scan, stale-client, and radio evidence as regression context.
- `.planning/milestones/v1.1-MILESTONE-AUDIT.md` retains a hardware follow-up:
  install the documented external `4.7 kOhm` DS18B20 DQ-to-`3.3 V` pull-up
  before the next hardware cycle.
- The same milestone audit records retroactive Nyquist `VALIDATION.md`
  documentation debt for Phases 7-10.
- `.planning/STATE.md` records eight retained quick-task scanner rows as
  bookkeeping debt, not unfinished firmware scope.
- `.planning/phases/11-baseline-ui-audit-and-state-language/11-VERIFICATION.md`
  records browser screenshots and device-backed state capture as `not-run`
  after browser bootstrap failed twice. Phase 14 still owns full visual and
  device-backed validation.
- `.graphify_detect.json` reports a large generated corpus and includes ignored
  managed dependency files. Keep graph tooling scoped so generated inventory
  noise does not drive source-level conclusions.
