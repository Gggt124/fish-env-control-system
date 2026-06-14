---
phase: 03-hardware-recovery-anti-lockout
plan: 02
subsystem: nvs_store
tags:
  - nvs
  - staging
  - factory-reset
dependency_graph:
  requires: []
  provides:
    - nvs_store_get_staging_type
    - nvs_store_stage_wifi
    - nvs_store_stage_creds
    - nvs_store_rollback_staging
    - nvs_store_commit_staging
    - nvs_store_factory_reset_credentials
  affects:
    - components/nvs_store/nvs_store.h
    - components/nvs_store/nvs_store.c
tech_stack:
  added: []
  patterns:
    - NVS staging area using prefixed keys (stg_sta_ssid, stg_sta_pass, stg_admin_user, stg_admin_pass)
    - Artificially mocking profile lists in nvs_store_load_wifi_profiles when stg_type == 1
key_files:
  created: []
  modified:
    - components/nvs_store/nvs_store.h
    - components/nvs_store/nvs_store.c
key_decisions:
  - Staging credentials and Wi-Fi networks in a distinct NVS area to allow try-before-you-commit rollback capability.
  - Overriding nvs_store_load_wifi_profiles to return only the staged Wi-Fi network as auto-connect candidate when staging is active.
metrics:
  duration_minutes: 10
  tasks_completed: 2
  lines_of_code_changed: 180
  completed_at: 2026-06-13T20:18:00+07:00
---

# Phase 03 Plan 02: NVS Staging Area and Factory Reset Summary

**Implemented the NVS staging area for Wi-Fi and administrator credentials, providing try-before-you-commit capability, staging rollback, and staging commit hooks, along with a factory reset credentials interface.**

## Implementation Details

- **Task 1 (Staging APIs):**
  - Added `stg_type`, `stg_sta_ssid`, `stg_sta_pass`, `stg_admin_user`, and `stg_admin_pass` keys to the `wifi_cfg` namespace.
  - Implemented `nvs_store_get_staging_type()`, defaulting the type to `0` (none) when not present in NVS.
  - Implemented `nvs_store_stage_wifi()` to save wifi staging values and set `stg_type` to `1`.
  - Implemented `nvs_store_stage_creds()` to save credential staging values and set `stg_type` to `2`.
  - Implemented `nvs_store_rollback_staging()` to set `stg_type` to `0` and discard staging.
  - Implemented `nvs_store_commit_staging()` to commit staging values to active values and set `stg_type` to `0`.
  - Modified `nvs_store_get_credentials()` to check `stg_type` and return staged credentials when `stg_type == 2`.
  - Modified `nvs_store_load_wifi_profiles()` to check `stg_type` and return only the staged profile as profile 0 and set auto_idx to 0 when `stg_type == 1`.
- **Task 2 (Factory Reset):**
  - Implemented `nvs_store_factory_reset_credentials()` to reset the user credentials to default values (`admin`/`admin123`) and reset any active staging configuration (`stg_type` = 0).

## Deviations from Plan

None - plan executed exactly as written.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED
