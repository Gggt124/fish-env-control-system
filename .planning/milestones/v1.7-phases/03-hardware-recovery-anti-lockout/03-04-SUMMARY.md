---
phase: 03-hardware-recovery-anti-lockout
plan: 04
subsystem: main
tags:
  - hardware
  - recovery
  - buttons
  - leds
  - factory-reset
dependency_graph:
  requires:
    - 03-01-PLAN.md
    - 03-02-PLAN.md
  provides:
    - hardware_ui_task
  affects:
    - components/app_config/app_config.h
    - main/app_main.c
tech_stack:
  added: []
  patterns:
    - FreeRTOS task for non-blocking button state polling and LED feedback
    - Bootloader veto to prevent strapping pin holds on boot from executing actions
    - Software-based mutual exclusion for dual button polling
key_files:
  created: []
  modified:
    - components/app_config/app_config.h
    - main/app_main.c
key_decisions:
  - Implemented software bootloader veto on strapping GPIO 0 to prevent download-mode holds from causing factory resets.
  - Used software mutual exclusion to ignore simultaneous presses on internal and external recovery buttons.
  - Leveraged dual LEDs to display multi-modal status: solid ON (0-2s hold or SoftAP active), slow blink (2-5s recovery trigger), fast blink (>5s factory reset), and double-blink (staged configuration awaiting confirmation).
metrics:
  duration_minutes: 20
  tasks_completed: 2
  lines_of_code_changed: 146
  completed_at: 2026-06-13T20:30:00+07:00
---

# Phase 03 Plan 04: Hardware Recovery Trigger & LED Feedback Summary

**Implemented the hardware button polling task with mutual exclusion and bootloader veto safety checks, and wired LED state indicators for SoftAP status, staging timeout double-blink, and recovery hold durations.**

## Implementation Details

- **Task 1 (GPIO Pin Mapping):** Added pins `APP_CONFIG_BOOT_BTN_GPIO` (0), `APP_CONFIG_EXT_BTN_GPIO` (4), `APP_CONFIG_LED_GPIO` (2), and `APP_CONFIG_EXT_LED_GPIO` (5) to `components/app_config/app_config.h` with documentation on strapping requirements.
- **Task 2 (Hardware UI Task & Reset Logic):**
  - Created `hardware_ui_task` (running every 50ms at priority `tskIDLE_PRIORITY + 1`) that initializes buttons (internal pull-ups) and LEDs.
  - Implemented bootloader veto to ignore initial LOW levels on boot button (GPIO 0 strapping pin) and external button until released.
  - Implemented mutual exclusion to discard inputs if both buttons are read as LOW.
  - Tracks hold durations to trigger SoftAP recovery at 2 seconds (with 500ms slow blink) and credentials factory reset at 5 seconds (with 100ms fast blink).
  - Drives LEDs with a double-blink pattern when config staging (`stg_type > 0`) is active, solid ON when SoftAP is active, and OFF otherwise.
  - Main loop handles `s_trigger_factory_reset` by factory-resetting credentials and invalidating memory sessions without rebooting the SoftAP.

## Deviations from Plan

None - plan executed exactly as written.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED
