---
phase: 03
reviewers: [gemini, opencode, antigravity]
reviewed_at: 2026-06-13T12:05:00Z
plans_reviewed: [03-01-PLAN.md, 03-02-PLAN.md, 03-03-PLAN.md, 03-04-PLAN.md]
---

# Cross-AI Plan Review — Phase 03

## Gemini Review

This review evaluates the four implementation plans for **Phase 3: Hardware Recovery & Anti-Lockout**.

### 1. Summary
The overall plan is technically sound and demonstrates a high degree of maturity, particularly in its approach to **NVS staging** and **hardware-software coordination**. The use of a temporary staging namespace in NVS (`stg_` prefix) is a robust pattern for implementing "try-before-you-commit" logic. The plans successfully address the core requirements of physical recovery (RECOV-01, RECOV-02) and autonomous rollback (RECOV-03). The client-aware SoftAP timeout and the mutual exclusion logic for hardware buttons show attention to real-world edge cases.

### 2. Strengths
*   **Robust Staging Pattern:** Implementing a dedicated `staging_type` and prefixed keys ensures that the system always knows whether it's running on a "trial" configuration or a "trusted" one.
*   **Client-Aware Timeout:** In Plan 01, checking for active clients (`esp_wifi_ap_get_sta_list`) before shutting down the SoftAP prevents a poor user experience where the setup portal disappears while a user is actively typing.
*   **Hardware-Software Synergy:** The integration of LED feedback (Blink codes) with internal system states (AP active, Recovery triggered) provides essential "blind" UI for a headless device.
*   **Clean API Separation:** New APIs like `wifi_manager_start_recovery_ap()` and `nvs_store_factory_reset_credentials()` keep the `app_main` logic clean and delegative.

### 3. Concerns
*   **Requirement Deviation (RECOV-03) [MEDIUM]:** Plan 01 Task 2 states that `nvs_store_commit()` is called immediately upon `WIFI_EVENT_STA_GOT_IP`. However, **RECOV-03** explicitly requires the device to wait 3 minutes for a user **"Confirm" API call** for *both* Credentials and Wi-Fi updates. Auto-committing on IP acquisition bypasses this manual confirmation step. If the user gets an IP on a network they cannot actually route to, they could still be "locked out" of the management interface.
*   **GPIO 0 Strapping Conflict [LOW]:** D-01 correctly identifies GPIO 0 (BOOT button). However, since this is an ESP32 strapping pin, if the user holds the button *while* powering on or resetting, the device will enter "Download Mode" instead of booting the application. The `recovery_task` only runs after boot. This should be documented or noted in the code as a known hardware limitation.
*   **Timer Race Conditions [LOW]:** In Plan 03, a 500ms timer is used to trigger `esp_restart()` after an API call. While usually sufficient, if the network is slow or the response is large, the socket might close before the client receives the `200 OK`.
*   **Session Invalidation [LOW]:** Plan 02 mentions incrementing `session_gen` for factory resets. Ensure this logic is also applied during `nvs_store_commit()` for credential updates to ensure old sessions are purged once new credentials are confirmed.

### 4. Suggestions
*   **Unify the Confirmation Flow:** Modify Plan 01 Task 2 to **not** auto-commit on `GOT_IP`. Instead, let both Wi-Fi and Credential staging rely on the 3-minute "Confirm" timer. Getting an IP should simply be a prerequisite for the user to reach the `/api/confirm` endpoint.
*   **Add "Staging" LED State:** Consider adding a unique LED pattern (e.g., a "Double Blink") that activates when `nvs_store_get_staging_type() > 0`. This tells the user: *"The device is in a temporary test state; please confirm settings in the UI or I will rollback."*
*   **AP Timeout Reset on Activity:** In Plan 01 Task 1, ensure that the `s_ap_timeout_timer` is **reset** every time a new client connects to the SoftAP, not just checked at the end of 10 minutes.
*   **Improve Reboot Reliability:** Instead of a simple timer for `esp_restart()`, consider using the HTTP server's "Post-registration" or "Cleanup" hooks if available, or simply increase the delay to 1000ms to ensure the TCP stack has flushed the response.

### 5. Risk Assessment
**Overall Risk: LOW**

The logic is highly defensive. Even if the web-based "Confirm" mechanism fails, the hardware button provides a guaranteed "escape hatch" to restore access. The staging mechanism protects the device from the most common failure mode: a mistyped Wi-Fi password or forgotten new admin credentials.

---

## OpenCode Review

### 1. Summary

The four plans collectively define a coherent hardware-assisted recovery system that successfully addresses all three requirements (RECOV-01, RECOV-02, RECOV-03). The architecture correctly priorities physical input, uses NVS as a staging sandbox, and prevents the most common lockout scenarios via automatic rollback. The plans are well-modularized across components, and the APIs introduced for staging, recovery AP, and the hardware task are appropriately scoped. However, the risk is classified **MEDIUM** because the current codebase has an aggressively simple Wi-Fi retry logic (`STA_MAX_RETRY=1`), automatically starts AP at boot, and does not reboot on Wi-Fi changes. The gap between the current "non-rebooting" state and the planned " reboot-on-change" and "rollback" state machine requires careful sequencing to avoid orphaned NVS staging keys or boot loops.

### 2. Strengths

- **Clear Separation of Concerns:** `wifi_manager` manages network state, `nvs_store` manages data persistence, and `main/` handles web/physical signals. This matches the existing codebase architecture.
- **Defensive by Design:** The 10-minute SoftAP timeout and 30-second IP-check / 3-minute credential-rollback timers prevent the device from remaining in a partially-secure state indefinitely.
- **User-friendly Hardware HMI:** The LED feedback mapping (solid, slow blink, fast blink) provides clear, multi-modal physical feedback without requiring a display.
- **Fail-closed Security:** SoftAP only opens when *physically* demanded; it doesn't expose an open network automatically.
- **Session Invalidation:** The credential reset correctly triggers a `session_gen` bump (reusing Phase 2 consecrated patterns) to kill stale sessions, preventing a remote attacker from maintaining access after recovery.

### 3. Concerns

- **HIGH — Boot Lockup on Unconfirmed Staging:** If the user stages Wi-Fi or credential changes but reboots happens due to another reason (e.g. brownout, WDT, watchdog), boot will see `staging_type != 0` and start the 30s or 3-minute rollback timers. If the user has no network access (e.g. they are locked out of their Wi-Fi), they cannot reach `/api/confirm`. This can create a boot loop if a power glitch happens during the rollback/deadline period, potentially corrupting the NVS subsystem.
- **HIGH — "No Free Pages" or NVS Corruption under Stress:** The 10-minute SoftAP timer with re-checking client count, combined with the potential rapid `esp_restart()` calls, could lead to a race condition where NVS commit happens at the same time as an unexpected reset. A `nvs_commit()` or `esp_restart()` during a flash write can corrupt the NVS partition.
- **MEDIUM — GPIO 0 is BOOT Button:** The plans use `GPIO 0` as `APP_CONFIG_BOOT_BTN_GPIO`. On classic ESP32 DevKit V1, GPIO 0 is a strapping pin. Holding it low on boot puts the chip into download mode, not flashed firmware. The intent in 03-04 is a *press during runtime*. While possible, code must verify that the CPU is not booting into download mode. If the device boots *while* the recovery button is accidentally held, the boot will fail. This is a hardware pitfall.
- **MEDIUM — LED Output: GPIO 2 and 5 Conflict Assumptions:** GPIO 2 is fine (commonly onboard Blue LED on DevKits, but also something to be aware of as it is connected to the flash chip on some boards). However, the assumption that GPIO 5 is free is good, yet unverified against the existing `docs/hardware.md` pinout. The pump/cooling hardware map currently occupies GPIO 25, 26, 27, 32, and 33. There is no collision on 5, but a formal check should happen.
- **MEDIUM — No Way to Cancel Rollback Pre-Reboot:** If a user stages changes via the web UI but changes their mind *before* the reboot, there is no `DELETE /api/stage` or cancel mechanism in the plans. They must either reboot and wait for rollback, or reboot and confirm.
- **LOW — "Confirm" API is a New Surface:** The new `/api/confirm` endpoint introduces another authenticated state change API. The web server must ensure the `Origin`/`Referer` check is also applied to this new POST endpoint.

### 4. Suggestions

- **Suggestion 1: Add Staging Cancel API:** Add a `DELETE /api/confirm` (or `POST /api/rollback`) to the API surface in `03-03-PLAN.md` so the user can clean up a staged change without rebooting.
- **Suggestion 2: Bootloader Veto on GPIO 0:** In `03-04-PLAN.md`, document or implement a check. If `GPIO 0` is held at boot, explicitly do a `esp_reset_reason_t` check and if it's `ESP_RST_POWERON` or `ESP_RST_EXT` with BOOT button held, skip credentials and just boot. Alternatively, do not rely on `GPIO 0`; use only `GPIO 4` for the recovery button. This avoids the boot/download ambiguity.
- **Suggestion 3: Single-Source-of-Truth Constant for Timeouts:** Many constants are hardcoded across tasks (10 min, 5 min, 30 sec, 3 min). Consolidate these into `app_config.h` with clear names like `APP_CONFIG_ROLLBACK_TIMEOUT_MS_WIFI`, `APP_CONFIG_ROLLBACK_TIMEOUT_MS_CREDS`, etc.
- **Suggestion 4: Circuit Breaker for Rollbacks:** In `03-01-PLAN.md` and `03-04-PLAN.md`, add logic into the boot sequence to check consecutive rollback counts. If the device restarts more than, say, 3 times in a row without a successful confirmation, auto-clear all pending staging and revert to fully safe defaults.
- **Suggestion 5: Confirmation via SoftAP in Wi-Fi Staging Scenario:** In `03-01-PLAN.md`, when `staging_type == 1` (Wi-Fi), if the new Wi-Fi fails but the SoftAP fallback is *not* manually open, the user is "soft-locked" out. Either document that the user must manually trigger SoftAP during the 30s period, or temporarily enable SoftAP in `wifi_manager_begin_staged_ip_check()` to guarantee a confirmation path.

### 5. Risk Assessment

**Overall Risk Level: MEDIUM**

---

## Antigravity Review

**1. Summary**
The plans provide a robust hardware recovery mechanism, leveraging NVS staging, physical button interactions, and Wi-Fi SoftAP fallback. The architecture successfully isolates risky changes and ensures the device will autonomously recover if misconfigured.

**2. Strengths**
- Decoupling the physical button task from the NVS update logic ensures safety and modularity.
- The 30-second IP wait and 3-minute `/api/confirm` checks provide layered protection against being disconnected forever.
- Hardware feedback via LEDs provides immediate, readable status to the user during recovery.

**3. Concerns**
- **HIGH:** NVS Flash Operations inside FreeRTOS Timers. The rollback loops use `nvs_store_rollback()` inside a FreeRTOS timer callback (Plan 01 Task 2 and Plan 04 Task 2). Timer callbacks run in the context of the FreeRTOS timer daemon task. NVS operations are blocking and can be slow; executing flash writes inside the timer daemon can crash or block the entire system timer tick mechanism. This must be deferred to a proper task.
- **HIGH:** The credentials factory reset (Plan 02 Task 1) is invoked directly upon button release in the polling task. If the button task has a high priority and another task is holding an NVS lock, this could cause priority inversion or watchdog timeouts.
- **MEDIUM:** Repeated invocation of `wifi_manager_start_recovery_ap()`. If the button task continuously calls this function while the button is held between 2-5 seconds, it may leak memory or crash the Wi-Fi stack. The API must be idempotent.

**4. Suggestions**
- Modify the 30-second and 3-minute FreeRTOS timer callbacks to merely set an event group bit or push to a queue, and have a dedicated task (or the main task loop) handle the actual `nvs_store_rollback()` and `esp_restart()`.
- Ensure the button polling task tracks state transitions so `wifi_manager_start_recovery_ap()` is only called once when the duration threshold is crossed, not continuously.

**5. Risk Assessment**
**HIGH**
The logic correctly addresses the requirements, but executing slow, blocking NVS writes and flash erases from inside a FreeRTOS timer daemon task is a severe architectural flaw that will likely cause Guru Meditation panics in ESP-IDF. Moving these operations to a standard task context will reduce the overall risk to LOW.

---

## Consensus Summary

The reviewers agree that the implementation of physical recovery flows and NVS staging is robust and correctly fulfills the hardware recovery and anti-lockout requirements. The modularization across `wifi_manager`, `nvs_store`, and `main/` accurately reflects the project's existing architecture.

### Agreed Strengths
- **Safe Sandboxing:** The NVS staging pattern safely isolates unconfirmed credentials and Wi-Fi configurations.
- **Physical Safety Net:** Physical access recovery via hardware buttons is correctly prioritized as an ultimate fail-safe.
- **Layered Timeout Protection:** The implementation of 10-minute SoftAP timeouts, 30-second IP limits, and 3-minute `/api/confirm` windows prevent indefinite lockout.
- **Hardware Feedback:** Mapping device states to solid, slow, and fast LED blinks provides critical feedback for headless operation.

### Agreed Concerns
- **HIGH — Unconfirmed Staging Confirmation Flow:** The plans currently try to auto-commit on `GOT_IP`, violating the explicit RECOV-03 requirement that a user must call `/api/confirm`. Without SoftAP active during the 30s Wi-Fi wait, the user may be soft-locked out if the new IP is inaccessible.
- **HIGH — NVS Corruption / Timer Context Operations:** Executing `nvs_store_rollback()` and `esp_restart()` from within FreeRTOS Timer callbacks or executing factory resets directly within the button polling task carries a high risk of NVS corruption, race conditions, and system crashes due to blocking operations in high-priority/daemon tasks.
- **MEDIUM — GPIO 0 Boot Strapping:** Utilizing GPIO 0 for the recovery button introduces a physical pitfall where holding the button during device reset or power-on enters Download Mode instead of running the firmware.

### Divergent Views
- **Session Management:** Gemini explicitly highlighted verifying that session generation invalidation (`session_gen`) is also applied correctly during `nvs_store_commit()` for staged updates, not just factory resets.
- **Cancel Staging API:** OpenCode suggested adding an explicit `DELETE /api/confirm` route so users can cancel staged updates without rebooting.
- **Circuit Breakers:** OpenCode suggested tracking consecutive rollback/reboot counts as a circuit breaker, which goes beyond the standard timeout rollback logic.
