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

**Justification:**
*   The system defaults to a safe state (Rollback) rather than a "Locked" state.
*   Hardware constraints are explicitly managed (GPIO 0).
*   State transitions are delegated to the most stable task context (`app_main`).
