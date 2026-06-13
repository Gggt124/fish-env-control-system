---
phase: 03-hardware-recovery-anti-lockout
verified: 2026-06-13T20:33:00Z
status: human_needed
score: 21/21 must-haves verified
overrides_applied: 0
overrides: []
gaps: []
deferred: []
human_verification:
  - test: "Verify Staging Confirmation UI Banner"
    expected: "When config staging is active, a yellow warning banner 'โหมดทดลองใช้การตั้งค่าใหม่...' is visible at the top of the dashboard, wifi, and status sections, with Confirm and Cancel buttons."
    why_human: "Grep cannot verify visual HTML rendering and UI toggle behavior in browser."
  - test: "Verify Hardware Button Recovery AP"
    expected: "Holding the boot/recovery button for 2-5 seconds triggers slow blinking LED. Upon release, LED stays solid ON, and 'FishPump-Setup' SoftAP is visible on Wi-Fi scans."
    why_human: "Requires physical interaction with hardware GPIO buttons and verification of SoftAP RF signal."
  - test: "Verify Hardware Button Factory Reset"
    expected: "Holding the boot/recovery button for more than 5 seconds triggers fast blinking LED. Upon release, credentials reset back to admin/admin123 and memory sessions are invalidated."
    why_human: "Requires physical interaction with hardware GPIO buttons and verification of credentials reset."
  - test: "Verify Auto-Rollback on Connection Failure"
    expected: "Staging a wrong Wi-Fi connection reboot and waiting 30 seconds triggers auto-rollback: the device reboots back to previous known-good Wi-Fi configuration."
    why_human: "Requires Wi-Fi environment and physical reboot monitoring."
---

# Phase 3: Hardware Recovery & Anti-Lockout Verification Report

**Phase Goal:** Provide physical hardware fail-safes for lost credentials and connectivity loss.
**Verified:** 2026-06-13T20:33:00Z
**Status:** human_needed
**Re-verification:** No — initial verification

## Goal Achievement

The codebase successfully implements the hardware and software layers for recovery and rollback:
1. Physical button polling task debounces and measures holds to trigger Recovery AP at 2 seconds and Factory Reset at 5 seconds.
2. Timing constants and SoftAP idle/recovery timeouts reset dynamically on client events and HTTP heartbeat requests.
3. Anti-lockout validation staging saves transient configurations under `stg_` keys in NVS, checking and timing out unconfirmed setups after 3 minutes.
4. Complete SPA confirmation banner lets the user confirm or cancel staged changes directly in the browser interface.

## Observable Truths

| #   | Truth   | Status     | Evidence       |
| --- | ------- | ---------- | -------------- |
| 1   | User can hold the recovery button to reset credentials to `admin`/`admin123` without losing other settings (RECOV-01) | ✓ VERIFIED | `hardware_ui_task` triggers `s_trigger_factory_reset` which calls `nvs_store_factory_reset_credentials()` and `session_invalidate_all()`. |
| 2   | SoftAP fallback only opens when user physically presses the recovery button (RECOV-02) | ✓ VERIFIED | `wifi_manager_init()` sets default mode to `WIFI_MODE_STA` unless staging is active. Button hold triggers `wifi_manager_start_recovery_ap()`. |
| 3   | If new credentials or Wi-Fi settings fail validation, the device reboots and rolls back to previous known-good settings (RECOV-03) | ✓ VERIFIED | Boot-time check starts confirm timer for 3 minutes (and wifi check timer for 30s). On timeout, it rolls back staged keys and restarts. |
| 4   | D-08: SoftAP MUST stop automatically if no clients are connected after 10 minutes. | ✓ VERIFIED | `ap_timeout_timer_cb` stops AP if client count is 0. |
| 5   | D-09: Timer MUST reset when new clients connect so active users are not dropped. | ✓ VERIFIED | `WIFI_EVENT_AP_STACONNECTED` and `WIFI_EVENT_AP_STADISCONNECTED` trigger `wifi_manager_reset_ap_timeout()`. |
| 6   | Manual trigger MUST be idempotent (can be called repeatedly without crashing). | ✓ VERIFIED | `wifi_manager_start_recovery_ap` is guarded and idempotent. |
| 7   | NVS operations MUST successfully isolate Wi-Fi and credential changes under `stg_` keys. | ✓ VERIFIED | Namespace keys `stg_type`, `stg_sta_ssid`, `stg_sta_pass`, `stg_admin_user`, and `stg_admin_pass` are used. |
| 8   | D-11: Unconfirmed staging MUST be rolled back after the 3-minute timeout. | ✓ VERIFIED | confirm timer callback sets `s_trigger_rollback` to `true` which triggers rollback and restart. |
| 9   | Rollback and NVS operations MUST occur in the main loop, NOT inside timer daemon context. | ✓ VERIFIED | `app_main()` main loop handles `s_trigger_rollback` and NVS writes. |
| 10  | `/api/auth/credentials` MUST stage and prompt a reboot rather than saving directly. | ✓ VERIFIED | `handle_api_auth_credentials()` stages new creds and schedules delayed reboot. |
| 11  | The UI MUST display a banner with a Confirm button when staging is active. | ✓ VERIFIED | `index.html` has `.staging-banner` layout, shown/hidden dynamically in `app.js` using `/api/status` response. |
| 12  | D-10: Wi-Fi validation relies on the 3-minute manual confirmation. SoftAP MUST remain open when staging is active. | ✓ VERIFIED | `wifi_manager_init()` forces SoftAP ON if `stg_type > 0`, and `ap_timeout_timer_cb()` skips shut down. |
| 13  | D-01: Supports 2 button inputs (internal Boot and configurable external) with internal pull-up. | ✓ VERIFIED | GPIO 0 and 4 configured as inputs with internal pull-up enabled. |
| 14  | D-02: Mutual Exclusion applies to button reads. | ✓ VERIFIED | `hardware_ui_task()` ignores read if both buttons are pressed. |
| 15  | D-03: Holding the button for 2 seconds MUST trigger `wifi_manager_start_recovery_ap()`. | ✓ VERIFIED | Hold duration checked at 2000ms to start recovery AP. |
| 16  | D-04: Holding the button for 5 seconds MUST trigger a credentials factory reset. | ✓ VERIFIED | Hold duration checked at 5000ms to trigger factory reset. |
| 17  | D-05: Supports 2 LEDs (onboard blue and configurable external). | ✓ VERIFIED | GPIO 2 and 5 configured as outputs. |
| 18  | D-06: Device MUST show distinct blink patterns for hold states. | ✓ VERIFIED | Slow blink (500ms) for AP, fast blink (100ms) for factory reset. |
| 19  | D-07: LED remains solid ON when SoftAP is active and user has released button. | ✓ VERIFIED | On button release, LED is set to 1 if `wifi_manager_is_ap_active()` is true. |
| 20  | Bootloader veto: GPIO 0 interactions MUST NOT trigger if the button was held down prior to booting. | ✓ VERIFIED | Veto flag set on task start if buttons are LOW, only cleared when buttons go HIGH. |
| 21  | Device MUST show a double blink LED pattern while waiting for user confirmation of staged changes. | ✓ VERIFIED | Staging active drives LED double blink pattern (ON 100ms, OFF 100ms, ON 100ms, OFF 700ms). |

**Score:** 21/21 truths verified

### Required Artifacts

| Artifact | Expected    | Status | Details |
| -------- | ----------- | ------ | ------- |
| New constants in `app_config.h` | Timing timeouts and button/LED GPIO mappings | ✓ VERIFIED | Added `APP_CONFIG_AP_RECOVERY_TIMEOUT_MS`, `APP_CONFIG_BOOT_BTN_GPIO`, etc. |
| `wifi_manager_start_recovery_ap` | Trigger SoftAP fallback manually with mutex safety | ✓ VERIFIED | Implemented with `s_wifi_mutex` acquisition. |
| `wifi_manager_reset_ap_timeout` | Restart idle timeout timer | ✓ VERIFIED | Restarts `s_ap_timeout_timer` to 10 minutes. |
| `wifi_manager_is_ap_active` | Return AP active status | ✓ VERIFIED | Checks mode and AP enabled state. |
| Staging functions in `nvs_store.h` | Functions to stage, commit, and rollback config changes | ✓ VERIFIED | `nvs_store_stage_wifi`, `nvs_store_stage_creds`, `nvs_store_commit_staging`, etc. |
| `nvs_store_factory_reset_credentials` | Revert credentials to `admin`/`admin123` | ✓ VERIFIED | Overwrites active credentials in NVS. |
| Staged routes in `web_server.c` | `/api/confirm` endpoints (POST/DELETE), staged credentials/wifi save | ✓ VERIFIED | Handlers registered and configured to reboot cleanly. |
| Rollback timers in `app_main.c` | Staging confirm and wifi check timers | ✓ VERIFIED | Created and handled in loop. |
| UI staging banner | Warning banner in dashboard, wifi, status pages | ✓ VERIFIED | Built inside `index.html` and toggled in `app.js`. |
| `hardware_ui_task` | FreeRTOS task to poll buttons and drive LEDs | ✓ VERIFIED | Polling every 50ms at priority `tskIDLE_PRIORITY + 1`. |

### Key Link Verification

| From | To  | Via | Status | Details |
| ---- | --- | --- | ------ | ------- |
| `app_main.c` | `wifi_manager.c` | `wifi_manager_start_recovery_ap` | ✓ WIRED | Called when button hold duration passes 2 seconds. |
| `web_server.c` | `nvs_store.c` | `nvs_store_stage_creds` / `nvs_store_stage_wifi` | ✓ WIRED | Called in `/api/auth/credentials` and `/api/wifi/profiles/save`. |
| `web_server.c` | `nvs_store.c` | `nvs_store_commit_staging` / `nvs_store_rollback_staging` | ✓ WIRED | Called in POST and DELETE `/api/confirm` handlers. |
| `web_server.c` | `wifi_manager.c` | `wifi_manager_reset_ap_timeout` | ✓ WIRED | Called in route instrument wrapper `handle_instrumented_route()`. |
| `app_main.c` | `session.c` | `session_invalidate_all` | ✓ WIRED | Called on rollback execution or factory reset. |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
| -------- | ------------- | ------ | ------------------ | ------ |
| `main/web_server.c` | `stg_type` | `nvs_store_get_staging_type` | Yes | ✓ FLOWING |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
| -------- | ------- | ------ | ------ |
| Compilation check | `powershell -File .\scripts\build.ps1` | `Project build complete.` | ✓ PASS |

### Probe Execution

| Probe | Command | Result | Status |
| ----- | ------- | ------ | ------ |

*No automated test probes exist for this phase. Automated verification is compile-only.*

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
| ----------- | ---------- | ----------- | ------ | -------- |
| **RECOV-01** | Plan 04 | Hardware Factory Reset (credentials default after 5s hold) | ✓ SATISFIED | `s_trigger_factory_reset` handled in `app_main` loop to write defaults. |
| **RECOV-02** | Plan 01, 04 | Timed Setup with Physical Consent (AP fallback on 2s hold) | ✓ SATISFIED | AP mode is STA on boot. 2s hold starts AP with 5 min timeout. |
| **RECOV-03** | Plan 02, 03 | Anti-Lockout Validation Rollback (3 min rollback staging) | ✓ SATISFIED | Rollback timers boot-active when `stg_type > 0`, verified, committed via `/api/confirm`. |

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
| ---- | ---- | ------- | -------- | ------ |
| None | | | | |

*No debt markers (TODO, FIXME, XXX) or empty implementations found in the modified codebase files.*

### Human Verification Required

### 1. Verify Staging Confirmation UI Banner
**Test:** Open the dashboard or status page when staging is active (e.g. after changing credentials or Wi-Fi settings, before confirming).
**Expected:** A yellow warning banner "โหมดทดลองใช้การตั้งค่าใหม่:..." is visible at the top, containing "ยืนยัน (Confirm)" and "ยกเลิก (Cancel)" buttons.
**Why human:** Grep cannot verify visual HTML rendering and UI toggle behavior in browser.

### 2. Verify Hardware Button Recovery AP
**Test:** Press and hold the Boot/Recovery button for 2-5 seconds.
**Expected:** LED starts slow blinking (500ms). Upon release, the LED stays solid ON, and the "FishPump-Setup" SoftAP becomes visible on Wi-Fi scans.
**Why human:** Requires physical interaction with hardware GPIO buttons and verification of SoftAP RF signal.

### 3. Verify Hardware Button Factory Reset
**Test:** Press and hold the Boot/Recovery button for more than 5 seconds.
**Expected:** LED blinks fast (100ms). Release the button. Verify in UI/serial log that credentials reset back to `admin`/`admin123`.
**Why human:** Physical interaction with hardware buttons.

### 4. Verify Auto-Rollback on Connection Failure
**Test:** Enter a wrong Wi-Fi password to stage Wi-Fi connection. Wait 30 seconds.
**Expected:** The device fails to get an IP address, times out, rolls back automatically, and reboots back to the previous known-good Wi-Fi.
**Why human:** Requires active Wi-Fi environment and physical device reboot observation.

### Gaps Summary

No programmatic gaps were found. The codebase builds successfully and matches the required specifications for hardware recovery, SoftAP timers, and configuration staging/rollback logic.

---

_Verified: 2026-06-13T20:33:00Z_
_Verifier: the agent (gsd-verifier)_
