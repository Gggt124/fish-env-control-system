# Phase 5 Verification: Hardware Validation And Regression Pass

**Phase:** 05-hardware-validation-and-regression-pass
**Date:** 2026-05-20
**Commit:** be4d547
**Verifier:** User-confirmed manual hardware testing + static regression checks

---

## Scope

Validate the complete ESP32 pump controller on real hardware and confirm existing Wi-Fi/setup behavior still works after pump control integration.

**Requirements under verification:** VAL-02, VAL-03, VAL-04, VAL-05

---

## Build Validation

| Check | Method | Result |
|-------|--------|--------|
| ESP-IDF build | `\.scripts\build.ps1` | PASS |
| Binary generated | `Test-Path build\fish_pump_relay_timer_control.bin` | PASS |
| Partition fit | App `0xf38a0` / Slot `0x1f0000` (51% free) | PASS |

---

## Static Regression Checks

| Surface | Check | Result |
|---------|-------|--------|
| GPIO defaults | `APP_TEMPLATE_PUMP_FLOAT_GPIO` = 32, `APP_TEMPLATE_PUMP_RELAY_GPIO` = 26, both active-low | PASS |
| Pump control logic | `select_timer_for_float_locked`, `set_relay_energized_locked`, `pump_control_stop` present | PASS |
| API routes | `/api/pump/config`, `/api/pump/status`, `/api/pump/start`, `/api/pump/stop` registered | PASS |
| Dashboard UI | Timer 1/2 controls, float/relay status, Start/Stop buttons present | PASS |
| App JS pump calls | API calls and Thai labels (`ลูกลอย`, `รีเลย์`) present | PASS |
| No CDN/external deps | No external URL references in `main/static/*` | PASS |
| Wi-Fi routes | `/api/wifi/scan`, `/api/wifi/connect`, `/api/status` registered | PASS |

---

## Manual Hardware Validation Evidence

### Hardware Under Test

- **Board:** ESP32 DevKit V1 30-pin (classic ESP32)
- **Float switch:** GPIO32, internal pull-up enabled, switch closes to GND (active-low)
- **Relay module:** GPIO26 output, active-low polarity (low energizes relay)
- **Firmware:** Commit `be4d547`

### VAL-02: Float Switch and Timer Selection

| Test | Expected | Actual | Status |
|------|----------|--------|--------|
| Float OFF (open, GPIO32 high) | Selects Timer 1 | Selects Timer 1 | PASS |
| Float ON (closed to GND, GPIO32 low) | Selects Timer 2 | Selects Timer 2 | PASS |

**Evidence:** User-confirmed on real ESP32 hardware: `1234 ผ่านหมด` (all 4 hardware checks passed).

### VAL-03: Relay ON/OFF Phase and Stop Safety

| Test | Expected | Actual | Status |
|------|----------|--------|--------|
| Relay during ON phase | Energized | Energized | PASS |
| Relay during OFF phase | De-energized | De-energized | PASS |
| Stop command | Relay inactive immediately | Relay inactive immediately | PASS |

**Evidence:** User-confirmed relay follows active timer's ON/OFF phase behavior and Stop forces relay inactive.

### VAL-04: Reboot Persistence

| Test | Expected | Actual | Status |
|------|----------|--------|--------|
| Timer settings after reboot | Restored from NVS | Restored from NVS | PASS |
| Auto-start preference after reboot | Restored from NVS | Restored from NVS | PASS |

**Evidence:** User-confirmed persisted timer settings and auto-start behavior survive reboot and take effect.

### VAL-05: Web and Wi-Fi Regression

| Test | Expected | Actual | Status |
|------|----------|--------|--------|
| SoftAP (`FishPump-Setup`) reachable | Yes | Yes | PASS |
| Login page accessible | Yes | Yes | PASS |
| Dashboard accessible | Yes | Yes | PASS |
| Status page accessible | Yes | Yes | PASS |
| Wi-Fi setup page accessible | Yes | Yes | PASS |
| Wi-Fi scan/connect works | Yes | Yes | PASS |
| Pump UI reachable and functional | Yes | Yes | PASS |
| No external dependencies | Yes | Yes | PASS |

**Evidence:** User-confirmed Wi-Fi and web regression passed after hardware validation.

---

## Requirement Coverage

| Requirement | Status | Evidence |
|-------------|--------|----------|
| VAL-02 | CLOSED | Manual hardware test: float OFF -> Timer 1, float ON -> Timer 2 |
| VAL-03 | CLOSED | Manual hardware test: relay follows ON/OFF, Stop forces inactive |
| VAL-04 | CLOSED | Manual reboot test: settings and auto-start persist |
| VAL-05 | CLOSED | Manual access test: SoftAP, login, dashboard, status, Wi-Fi, pump UI all reachable |

---

## Findings

No new defects discovered during final validation pass. Build, static checks, manual hardware evidence, and web/Wi-Fi regression all passed.

---

## Residual Risk

| Risk | Level | Mitigation |
|------|-------|------------|
| Manual evidence may drift if firmware changes | Low | Verification records exact commit (`be4d547`) |
| No automated hardware-in-loop tests | Accepted | Project scope validates through manual device testing |
| Relay module polarity varies | Low | Polarity is configurable in NVS if hardware changes |

---

*Verified: 2026-05-20*
*For full evidence see `.planning/phases/05-hardware-validation-and-regression-pass/05-01-SUMMARY.md`*
