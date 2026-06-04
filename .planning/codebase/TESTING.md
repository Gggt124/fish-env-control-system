---
generated: 2026-05-18
refreshed: 2026-06-02
last_mapped_commit: 3ef2ac063ebead27920e5e340508c371f689cdca
focus: quality
scope: .gitignore,.graphify_detect.json,.planning,PRODUCT.md
---

# Testing And Validation

## Current Test State

There are no unit tests in this phase. The project validates primarily through ESP-IDF build success and manual device testing.

## Build Validation

- Primary validation command: `.\scripts\build.ps1`.
- Manual equivalent: set UTF-8 code page, export ESP-IDF, then run `idf.py build`.
- A valid build produces `build\fish_pump_relay_timer_control.bin`.
- `README.md` and `AGENTS.md` both define `idf.py build` as the core validation gate.

## Build Script

- `scripts/build.ps1` accepts optional `-IdfPath`.
- It falls back to `C:\esp-idf` when `IDF_PATH` is unset and that path exists.
- It sources `export.ps1` in the same PowerShell process before running `idf.py`.
- It runs `idf.py --version` before `idf.py build`.

## Manual Test Coverage

Documented manual flow in `README.md`:

- Flash firmware to ESP32.
- Connect to SoftAP `FishPump-Setup`.
- Open `http://192.168.4.1`.
- Log in with configured credentials.
- Use Wi-Fi Settings to scan networks and connect STA.
- Reboot and confirm saved STA credentials load from NVS.
- Confirm AP fallback still works when STA connection fails.

## Functional Areas Covered Manually

- SoftAP boot and fallback reachability.
- HTTP server startup and static page serving.
- Login flow and cookie session creation.
- Protected dashboard/status/wifi access.
- Wi-Fi scan API and frontend rendering.
- STA connect/disconnect behavior.
- NVS persistence for saved station credentials.
- Captive DNS and mDNS are visible operational features, but no automated tests are present.

## Build Lessons

`docs/development-notes.md` records several validation practices:

- Run `idf.py reconfigure` after adding or changing `idf_component.yml`.
- Read `build/log/idf_py_stderr_output_*` and `build/log/idf_py_stdout_output_*` for real build errors.
- Avoid piping `idf.py build` through output truncation because useful errors may be in stderr/logs.
- Run `idf.py fullclean` then `idf.py build` for stale CMake or dependency issues.
- Include exact ESP-IDF headers in files that call APIs; implicit function declarations are build failures.

## Missing Automated Coverage

- No component tests for `nvs_store`.
- No session expiry or slot-replacement tests for `session`.
- No Wi-Fi manager state-machine tests.
- No HTTP handler tests for JSON parsing, auth, rate limiting, or CSRF checks.
- No frontend tests for `main/static/app.js`.
- No automated captive DNS packet tests.

## Practical Next Test Additions

- Add host-side tests for pure parsing/helpers where possible, such as URL decode and input validation.
- Add ESP-IDF Unity component tests for session token lifecycle and NVS wrappers.
- Add manual or scripted serial-command smoke checks for AP boot and status endpoint after flashing.
- Add browser-level smoke tests only if a local mock server is introduced; the current embedded server depends on ESP-IDF runtime.

## Residual Risk

Because the project relies on manual device validation, concurrency regressions in Wi-Fi events, scan timeout handling, AP fallback, and session expiry may only appear on hardware. Build success is necessary but not enough to prove runtime behavior.

## Incremental Refresh: Validation Baseline

This section was refreshed from `.planning/` only. Testing descriptions above
remain from the prior full-map scan unless superseded below.

- `.planning/milestones/v1.1-MILESTONE-AUDIT.md` records green build, syntax,
  source-diff, schema-drift, hardware UAT, and long-uptime evidence for v1.1.
- `.planning/debug/wifi-ap-sta-longrun-stuck.md` records a resolved APSTA,
  asynchronous scan, and HTTP transport investigation with a passing
  `13:38:10` real-use soak.
- `.planning/phases/11-baseline-ui-audit-and-state-language/11-VERIFICATION.md`
  records a documentation-scope Phase 11 pass: JavaScript syntax, offline
  dependency scan, and the `main/static/` no-edit boundary passed.
- Phase 11 browser screenshots and device-backed UI states remain `not-run`
  after browser bootstrap failed twice.
- `.planning/ROADMAP.md` assigns desktop/mobile screenshots, accessibility
  review, closing Impeccable audit, ESP-IDF build, footprint comparison, and
  stable v1.1 hardware regression to Phase 14.
- `.planning/milestones/v1.1-MILESTONE-AUDIT.md` records missing retroactive
  Nyquist `VALIDATION.md` artifacts for Phases 7-10 as documentation debt.
