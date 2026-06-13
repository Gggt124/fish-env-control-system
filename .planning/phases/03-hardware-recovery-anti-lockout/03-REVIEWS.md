---
phase: 3
reviewers: [gemini, opencode, antigravity]
reviewed_at: 2026-06-13T12:44:00Z
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

## OpenCode Review

### 1. Summary
The implementation plans are structurally robust and leverage FreeRTOS concurrency and ESP-IDF NVS correctly to achieve safe configuration staging. The approach of using a daemon task for hardware UI and delegating system-critical resets to the main loop demonstrates high-quality embedded design.

### 2. Strengths
*   **Staging Isolation:** Isolating Wi-Fi and credentials in `stg_` keys ensures zero corruption risk to the active profile during try-outs.
*   **Thread Safety Mindset:** Using `atomic_bool` for `s_trigger_rollback` across tasks prevents the most common concurrency bugs when dealing with `esp_timer` callbacks.

### 3. Concerns
*   **HTTP Response Delivery Race [HIGH]:** In `03-03-PLAN.md`, sending an HTTP response and then calling `esp_restart()` via a short timer often results in the client receiving a socket hangup before the TCP packet is fully ACK'd. The reboot must either happen via the `esp_http_server`'s close hook, or explicitly delay long enough (e.g. `vTaskDelay(pdMS_TO_TICKS(1000))`) to flush the socket.
*   **Mutex Acquisition in AP Start [HIGH]:** In `03-04-PLAN.md`, `wifi_manager_start_recovery_ap()` is called from the `hardware_ui_task`. Because `wifi_manager` state transitions are asynchronous, this function MUST acquire `s_wifi_mutex` to safely interact with Wi-Fi event state.
*   **Factory Reset Scope [MEDIUM]:** `03-02-PLAN.md` implements `nvs_store_factory_reset_credentials()`. This is good, but does it clear the current Wi-Fi STA credentials? The requirements explicitly say "without affecting Wi-Fi or pump configurations", so the plan is correct, but users might expect a full factory reset to also wipe Wi-Fi.

### 4. Suggestions
*   Explicitly mention `s_wifi_mutex` in the `wifi_manager_start_recovery_ap()` implementation steps to prevent concurrency bugs.
*   Increase the reboot delay to at least 1-2 seconds after HTTP endpoints that return `reboot_pending`.

### 5. Risk Assessment
**Overall Risk: LOW**
The architecture is solid and the requirements are well mapped.

---

## Antigravity Review

### 1. Summary
The Phase 3 plans provide strong autonomous recovery features, ensuring physical override and secure configuration staging. The use of deferred execution via the main loop is an excellent pattern for embedded resilience and aligns with best practices for FreeRTOS architectures.

### 2. Strengths
*   **Deferred Execution Pattern:** Correctly moving `esp_restart()` and heavy NVS operations out of the timer/interrupt contexts into the `while(1)` `app_main` loop significantly improves runtime stability.
*   **Bootloader Veto:** The logic to ignore GPIO 0 drops that originated before boot resolves a very common physical edge case with ESP32 DevKits.

### 3. Concerns
*   **Staging Profile Emulation [HIGH]:** In `03-02-PLAN.md`, `nvs_store_load_wifi_profiles` artificially returns a single profile when `stg_type == 1`. This could cause `wifi_manager` to misbehave if it expects to manage or cycle through standard multiple networks. The plan must ensure that `auto_idx` cleanly overrides without corrupting the active profile search logic.
*   **HTTP AP Timeout Reset Overhead [MEDIUM]:** `03-01-PLAN.md` suggests calling `wifi_manager_reset_ap_timeout()` on every HTTP request. If the web server handles many rapid requests (like status polling every 1 second), repeatedly resetting the timer could cause unnecessary overhead. It should only restart if the remaining time is below a certain threshold or be rate-limited.
*   **Cancel Rollback Ordering [LOW]:** In `03-03-PLAN.md`, setting `s_cancel_rollback` must happen atomically and before any NVS commits to ensure that the `app_main` task doesn't trigger rollback simultaneously.

### 4. Suggestions
*   Add a rate-limit or minimum threshold check in `wifi_manager_reset_ap_timeout()` to avoid resetting the esp_timer on every single polling request.
*   Ensure that all atomic variables use proper memory ordering (e.g. `memory_order_relaxed` or `memory_order_seq_cst`) when checked/set in the `while(1)` loop.

### 5. Risk Assessment
**Overall Risk: LOW**
The recovery mechanics are deeply integrated and well-isolated, reducing the risk of side-effects on the core pump control logic.

---

## Consensus Summary

The plans are highly robust and defensive, employing strong patterns like NVS staging, deferred execution to the main loop, and bootstrapping constraints awareness.

### Agreed Strengths
*   **Defensive Staging:** Using `stg_` keys isolates trial configurations cleanly.
*   **Safe Task Contexts:** Delegating system actions (reboots, NVS commits) to `app_main` instead of executing them inside timer or HTTP callbacks.
*   **Hardware Awareness:** Implementing Bootloader Veto for GPIO 0 strapping.

### Agreed Concerns
*   **HTTP Response Delivery Race [HIGH]**: (Gemini, OpenCode) Rebooting immediately after an API call can cause socket closure before the client receives the `200 OK`.
*   **Mutex Acquisition in AP Start [HIGH]**: (OpenCode) Calling `wifi_manager_start_recovery_ap()` from the new `hardware_ui_task` might cause race conditions if `s_wifi_mutex` isn't acquired properly.
*   **Staging Profile Emulation [HIGH]**: (Antigravity) Modifying `nvs_store_load_wifi_profiles` to artificially return a single profile might break `wifi_manager` iteration logic.

### Divergent Views
*   OpenCode notes that factory reset scope strictly preserving Wi-Fi is correct per requirements but might be unexpected by users. Gemini and Antigravity accept this boundary as stated.
