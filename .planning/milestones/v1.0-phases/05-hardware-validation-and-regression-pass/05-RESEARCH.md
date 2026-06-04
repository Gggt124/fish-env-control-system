---
phase: 05-hardware-validation-and-regression-pass
generated: 2026-05-20
mode: inline-codex
sources:
  - .planning/phases/05-hardware-validation-and-regression-pass/05-CONTEXT.md
  - .planning/REQUIREMENTS.md
  - .planning/ROADMAP.md
  - .planning/PROJECT.md
  - .planning/codebase/TESTING.md
  - .planning/codebase/CONCERNS.md
  - .planning/codebase/STRUCTURE.md
  - .planning/phases/01-hardware-safe-pump-control-core/01-VERIFICATION.md
  - .planning/phases/02-pump-settings-persistence-and-boot-behavior/02-VERIFICATION.md
  - .planning/phases/03-authenticated-pump-control-api/03-VERIFICATION.md
  - .planning/phases/04-web-pump-control-ui/04-VERIFICATION.md
---

# Phase 05 Research: Hardware Validation And Regression Pass

## RESEARCH COMPLETE

Phase 5 is a validation and closeout phase. The useful work is to preserve the real-device evidence already captured in `05-CONTEXT.md`, rerun deterministic checks that can be run locally, and produce a concise summary that lets verification close `VAL-02` through `VAL-05`.

## Phase Boundary

This phase should not add relay behavior, GPIO configurability, UI features, OTA, logs, charts, WebSockets, MQTT, HTTPS, or production auth hardening. If repeat validation uncovers an issue, the executor should document it and make the smallest fix only if the defect blocks a Phase 5 requirement and stays within existing component boundaries.

## Evidence Model

The repository has no automated hardware-in-loop test rig. Phase 5 evidence therefore combines:

- Existing user-confirmed manual hardware results from `05-CONTEXT.md`.
- Local build validation through `.\scripts\build.ps1` or the documented `idf.py build` sequence.
- Static route/UI checks for the pump API and local-only dashboard surface.
- A closeout summary that records what was tested, what passed, and any findings.

Build success is necessary but not sufficient. `VAL-02`, `VAL-03`, and `VAL-04` close only through manual hardware evidence. `VAL-05` closes through manual access evidence plus static checks confirming that login, dashboard, Wi-Fi/status routes, and pump UI routes remain present.

## Relevant Implementation Surface

- `components/app_config/app_config.h` defines the hardware defaults under validation: GPIO32 float input, GPIO26 relay output, active-low float, active-low relay, timer defaults, and debounce.
- `components/pump_control/pump_control.c` owns float sampling, timer selection, ON/OFF phase transitions, relay inactive writes, and Stop behavior.
- `components/nvs_store/nvs_store.c` owns persisted pump timer settings, relay polarity, and auto-start.
- `main/app_main.c` loads pump settings before pump init and applies auto-start behavior.
- `main/web_server.c` registers authenticated `/api/pump/config`, `/api/pump/status`, `/api/pump/start`, and `/api/pump/stop`, plus existing system/Wi-Fi/status routes.
- `main/static/dashboard.html`, `main/static/app.js`, and `main/static/style.css` own the local no-CDN pump dashboard experience.
- `scripts/build.ps1` is the preferred build validation wrapper.
- `scripts/flash.ps1` is available for manual flashing when a COM port is known; it should not be run blindly because the port is machine/device-specific.

## Validation Checklist

1. Build the current firmware and confirm `build\fish_pump_relay_timer_control.bin` exists.
2. Record hardware pin map under test: GPIO26 relay output, GPIO32 float input, active-low relay polarity, active-low float switch to GND.
3. Preserve the user-confirmed float evidence: Float OFF selects Timer 1 and Float ON selects Timer 2.
4. Preserve the user-confirmed relay evidence: relay follows ON/OFF phase and Stop forces inactive.
5. Preserve the user-confirmed reboot evidence: timer settings and auto-start preference survive reboot and take effect.
6. Preserve the user-confirmed web/Wi-Fi evidence: SoftAP, login, dashboard, status page, Wi-Fi page, Wi-Fi scan/connect, and pump UI remain reachable.
7. Document any repeated-test findings in the Phase 5 summary or verification notes rather than expanding scope.

## Risks To Keep Visible

- Manual evidence can drift if firmware changes after the user test. The executor should run a final build and note the exact commit/build state in the summary.
- Flashing requires a real ESP32 and correct COM port; the plan should not assume `COM5` is always correct even though the wrapper defaults to it.
- Relay modules vary in polarity. The Phase 5 evidence applies to the current active-low relay wiring captured in context.
- AP fallback and Wi-Fi setup are sensitive to device/network state, so validation notes should distinguish SoftAP reachability from STA success.

## Planning Recommendation

Use one execution plan in wave 1:

- It reads existing phase context and prior verification artifacts.
- It runs deterministic build/static checks.
- It writes `05-01-SUMMARY.md` with explicit requirement traceability for `VAL-02`, `VAL-03`, `VAL-04`, and `VAL-05`.
- It does not introduce new product files unless a blocking validation defect is found.
