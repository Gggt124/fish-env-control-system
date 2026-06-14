---
phase: 03-hardware-recovery-anti-lockout
plan: 01
subsystem: wifi_manager
tags:
  - recovery
  - timeout
  - wifi
dependency_graph:
  requires: []
  provides:
    - wifi_manager_start_recovery_ap
    - wifi_manager_reset_ap_timeout
    - wifi_manager_is_ap_active
  affects:
    - components/app_config/app_config.h
    - components/wifi_manager/wifi_manager.c
    - components/wifi_manager/wifi_manager.h
    - main/web_server.c
tech_stack:
  added: []
  patterns:
    - ESP-IDF esp_timer for SoftAP timeout management
    - HTTP request route wrapping for activity heartbeats
key_files:
  created: []
  modified:
    - components/app_config/app_config.h
    - components/wifi_manager/wifi_manager.c
    - components/wifi_manager/wifi_manager.h
    - main/web_server.c
key_decisions:
  - Default Wi-Fi mode to STA on boot to prevent SoftAP fallback unless manually triggered.
  - Implement idempotent recovery AP startup using a mutex-protected wrapper and a 5-minute timeout.
  - Restructure HTTP route wrapper to trigger AP idle timeout resets on any request (heartbeat).
metrics:
  duration_minutes: 15
  tasks_completed: 3
  lines_of_code_changed: 115
  completed_at: 2026-06-13T20:12:00+07:00
---

# Phase 03 Plan 01: SoftAP Manual Trigger and Timeout Summary

**Implemented the SoftAP manual trigger and timeout mechanisms required for Secure AP Fallback, defaulting the Wi-Fi mode to STA on boot and resetting the AP idle timeout on client events and HTTP requests.**

## Implementation Details

- **Task 1 (Timing Constants):** Added SoftAP timeout constants (`APP_CONFIG_AP_RECOVERY_TIMEOUT_MS` = 5 mins, `APP_CONFIG_AP_IDLE_TIMEOUT_MS` = 10 mins, `APP_CONFIG_ROLLBACK_WIFI_TIMEOUT_MS` = 30s, `APP_CONFIG_ROLLBACK_CONFIRM_TIMEOUT_MS` = 3 mins) to `components/app_config/app_config.h`.
- **Task 2 (SoftAP Manual Fallback & Timers):**
  - Updated `wifi_manager_init()` to set default mode to `WIFI_MODE_STA` and created `s_ap_timeout_timer`.
  - Implemented `wifi_manager_start_recovery_ap()` with `s_wifi_mutex` safety to transition the interface to `WIFI_MODE_APSTA` and start the timer.
  - Implemented `wifi_manager_reset_ap_timeout()` to restart the timer to 10 minutes when client connect/disconnect events occur.
  - Implemented `wifi_manager_is_ap_active()` to check if the SoftAP is currently running.
- **Task 3 (Activity Heartbeat):** Wrapped `wifi_manager_reset_ap_timeout()` inside `handle_instrumented_route()` in `main/web_server.c` to prevent SoftAP timeout from expiring while an operator is actively navigating or configuring the web dashboard.

## Deviations from Plan

None - plan executed exactly as written.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED
