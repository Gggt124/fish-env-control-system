---
phase: 3
reviewers: [gemini, antigravity, opencode]
reviewed_at: 2026-06-13T12:33:00Z
plans_reviewed: [03-01-PLAN.md, 03-02-PLAN.md, 03-03-PLAN.md, 03-04-PLAN.md]
---

# Cross-AI Plan Review — Phase 3

## Gemini Review

This review evaluates the four implementation plans for **Phase 3: Hardware Recovery & Anti-Lockout**.

### 1. Summary
The implementation plans are technically mature and demonstrate a deep understanding of the ESP-IDF framework and hardware constraints. The architecture for **NVS Staging** (Plan 02) is particularly well-conceived, providing a robust "sandbox" for configuration changes. The plans successfully address the "Anti-Lockout" requirement by separating the acquisition of connectivity (STA GOT_IP) from the commitment of credentials, ensuring the user can always reach the management interface before changes become permanent. The inclusion of a "Bootloader Veto" for GPIO 0 and the delegation of system-level actions (NVS writes, reboots) to the main loop context are excellent defensive programming practices.

### 2. Strengths
*   **Defensive Staging Pattern:** The use of `stg_` prefixed keys in NVS (Plan 02) ensures that a device rebooting after a credential change is still "aware" it is in a trial state, allowing for reliable autonomous rollback.
*   **Context-Safe Execution:** Plan 03 correctly avoids performing NVS operations or calling `esp_restart()` within `esp_timer` callbacks (daemon task). Delegating these to the `app_main` loop via the `s_trigger_rollback` flag is a best practice.
*   **Hardware Constraint Awareness:** The "Bootloader Veto" logic in Plan 04 shows critical awareness of the ESP32's strapping pins (GPIO 0). This prevents a user holding the button during power-on (entering Download Mode) from accidentally triggering a factory reset upon subsequent boot.
*   **Rich Visual Feedback:** The LED state machine (Plan 04) provides essential "blind" UI for a headless device, especially the "Double Blink" pattern to signal a pending staging confirmation.
*   **Client-Aware AP Timeout:** Resetting the AP timer on client connection (Plan 01) ensures a smooth user experience where the setup portal doesn't disappear while a user is actively configuring the device.

### 3. Concerns
*   **HTTP Response Race Condition [MEDIUM]:** Plan 03 Task 1 mentions scheduling reboots after API calls. If the `esp_restart()` or Wi-Fi disconnection happens too quickly, the HTTP client (browser) may receive a "Network Error" or "Connection Reset" before it can process the `200 OK` response. 
*   **Rollback Timer Cancellation Race [LOW]:** In Plan 03 Task 2, the `s_cancel_rollback` flag is checked in the main loop. There is a tiny race window where the timer fires and sets `s_trigger_rollback = true` just as the user hits "Confirm". The plan addresses this by resetting `s_trigger_rollback` when `s_cancel_rollback` is detected, which is good, but the logic should be atomic or use a mutex if flags are accessed across tasks.
*   **GPIO 2 Strapping Behavior [LOW]:** Plan 04 uses GPIO 2 for LED feedback. On many ESP32 modules, GPIO 2 is a strapping pin (MTDI) that must be LOW during boot to enter the correct flash voltage mode. While most DevKits have internal pull-downs, using it as an output is safe only if the external LED circuit doesn't pull the pin HIGH at boot.
*   **Session Generation Invalidation [LOW]:** When credentials are factory reset (Plan 04), `session_invalidate_all()` is called. Ensure this logic is also triggered when a rollback occurs to prevent "stale" sessions from being valid against the restored older credentials.

### 4. Suggestions
*   **Ensure Reboot Delay:** Explicitly use a `vTaskDelay(pdMS_TO_TICKS(1000))` before calling `esp_restart()` in reboot tasks to allow the TCP stack to flush the response buffers to the client.
*   **AP Activity Heartbeat:** In Plan 01, consider resetting the `s_ap_timeout_timer` not just on `STACONNECTED`, but also on any successful HTTP request to the web server. This ensures the AP stays open even if a user stays connected but takes a long time between configuration steps.
*   **Staging LED Priority:** In the `hardware_ui_task`, ensure the "Double Blink" staging pattern takes precedence over the "Solid ON" AP active state. A user needs to know their changes are temporary more than they need to know the AP is simply "on."
*   **Document GPIO Constraints:** Add a comment in `app_config.h` regarding the strapping nature of GPIO 0 and GPIO 2 to warn future developers against adding heavy pull-up resistors to these lines.

### 5. Risk Assessment
**Overall Risk: LOW**

The design is highly defensive and prioritizes system availability. The combination of hardware-based recovery (Physical Button) and software-based recovery (Rollback Timer) creates a "failsafe-on-failsafe" architecture. The isolation of staging logic within the `nvs_store` component minimizes the footprint of these changes on the core pump control logic.

---

## Antigravity Review

### 1. Summary
The plans for Phase 3 outline a highly defensive and resilient approach to hardware recovery and anti-lockout. The introduction of an NVS staging namespace provides a robust "try-before-you-commit" workflow, isolating potentially fatal credential and Wi-Fi changes until they are verified. The hardware button interactions and SoftAP timeouts effectively balance recovery access with security, while preventing issues such as active client drops.

### 2. Strengths
- The staging namespace (`stg_type`) correctly sandboxes volatile settings, protecting the core device access loop.
- Bootloader veto logic (requiring GPIO 0 to transition from HIGH to LOW) effectively avoids conflicts with ESP32 Download Mode if the button is held during power-up.
- Client-aware SoftAP timeouts (`esp_wifi_ap_get_sta_list`) prevent the access point from dropping users actively engaged in setup.
- Explicit `s_trigger_rollback` delegation to the main loop properly isolates FreeRTOS daemon contexts from blocking NVS operations.

### 3. Concerns
- **Lack of Auto-Commits for Wi-Fi Connectivity in Validation [HIGH]:** In Plan 03-03 Task 2, `APP_CONFIG_ROLLBACK_WIFI_TIMEOUT_MS` rolls back if `wifi_manager_is_sta_connected()` is false. However, if it *is* true (device connected to STA), it never auto-commits or disables the 3-minute `APP_CONFIG_ROLLBACK_CONFIRM_TIMEOUT_MS`. A user setting Wi-Fi credentials won't be able to "Confirm" them via API if the device leaves the SoftAP mode to join the STA network and the user doesn't know its new IP. The Wi-Fi staging should auto-commit upon successful STA connection and IP acquisition, or SoftAP must remain open.
- **Timer Race Conditions with Cancel/Confirm [MEDIUM]:** Plan 03-03 Task 2 handles `s_cancel_rollback`, but setting `s_cancel_rollback` from the HTTP handler might race with the timer firing and setting `s_trigger_rollback`. A strict mutex or atomic operation should be specified to prevent the device from rolling back immediately after a successful commit.
- **NVS Write Wear on Timers [LOW]:** Ensure the `nvs_store_rollback_staging()` is only called if staging was actually present to avoid unnecessary NVS erase/write cycles during every boot.

### 4. Suggestions
- Add an automatic `nvs_store_commit_staging()` call when `WIFI_EVENT_STA_GOT_IP` is triggered and `stg_type == 1`, because reaching the network validates the Wi-Fi credentials.
- In `03-03-PLAN.md`, specify the use of atomics (`stdatomic.h`) or a FreeRTOS mutex for `s_trigger_rollback` and `s_cancel_rollback` flag interactions.
- Provide a clear JSON response payload or HTTP 202 Accepted status for the `/api/confirm` endpoint so the frontend knows the timer was definitely canceled before reloading.

### 5. Risk Assessment
**Overall Risk: MEDIUM**

The logic is largely sound and well-segmented, but the interaction between the Wi-Fi staging rollback timer and the user's ability to reach the `/api/confirm` endpoint introduces a risk of "infinite rollback loops" if the device connects to the STA network but the user cannot locate it to send the confirmation API call. Modifying the Wi-Fi verification to auto-commit on successful connection lowers this risk.

---

## OpenCode Review

OpenCode review failed or timed out.

---

## Consensus Summary

The reviewers agree that the implementation plans are mature, defensive, and well-designed. The NVS staging architecture, main-loop NVS execution delegation, and SoftAP timeout safety mechanisms are noted as significant strengths. 

### Agreed Strengths
- **Staging Namespace Logic:** Isolating settings correctly ensures robust autonomous rollback if changes fail validation.
- **Context-Safe Operations:** Delegating NVS operations to the main loop instead of ESP Timer contexts avoids potential crashes.
- **Hardware/User Fallbacks:** Client-aware SoftAP timeouts and Bootloader Veto features improve both UX and system safety.

### Agreed Concerns
- **Race conditions with HTTP/Timers [MEDIUM]:** Both reviewers identified race conditions: either with the `esp_restart()` executing before HTTP `200 OK` is flushed, or `s_cancel_rollback` racing against `s_trigger_rollback` when the user confirms.
- **Wi-Fi Connectivity Auto-Commit Gap [HIGH]:** A crucial flaw is that Wi-Fi validation doesn't auto-commit or lock open the SoftAP upon successful STA connection, which could lock a user out if they can't find the new STA IP within the 3-minute confirm timer.
- **Strapping Pin/Session Invalidation [LOW]:** GPIO 2 is a strapping pin and needs caution. Session invalidation shouldn't be missed on rollback events.

### Divergent Views
- None. Reviewers aligned perfectly on the strengths of the architecture and flagged different but complementary edge cases in state transitions and networking.
