---
phase: 06
slug: hardware-contract-and-persistent-map-foundation
status: verified
nyquist_compliant: true
wave_0_complete: true
created: 2026-05-23
---

# Phase 06 - Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | ESP-IDF build plus command-based source/documentation checks |
| **Config file** | `scripts/build.ps1`, root `CMakeLists.txt`, component `CMakeLists.txt` files |
| **Quick run command** | Requirement-specific `rg` checks from each plan task |
| **Full suite command** | `.\scripts\build.ps1`; `Test-Path build\fish_pump_relay_timer_control.bin` |
| **Estimated runtime** | ~55 seconds |

---

## Sampling Rate

- **After every task commit:** Run the task's planned `rg` verification command.
- **After every plan wave:** Run `.\scripts\build.ps1` when the wave changes buildable firmware code.
- **Before `$gsd-verify-work`:** `.\scripts\build.ps1` and requirement coverage checks must be green.
- **Max feedback latency:** ~60 seconds for full build validation; <10 seconds for focused source checks.

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 06-01-01 | 01 | 1 | HW-01 | T-06-01 | Hardware roles are typed and scoped to firmware-owned metadata. | source check | `rg "hardware_role_t|HARDWARE_ROLE_FLOAT_INPUT|HARDWARE_ROLE_PUMP_RELAY_1|HARDWARE_ROLE_PUMP_RELAY_2|HARDWARE_ROLE_DS18B20_DATA|HARDWARE_ROLE_COOLING_RELAY|hardware_map_t|hardware_relay_polarity" components/hardware_map` | N/A command | green |
| 06-01-02 | 01 | 1 | HW-02 | T-06-01 | Safe GPIO defaults and option lists avoid unsafe ESP32 pins for output roles. | source check | `rg "APP_TEMPLATE_HW_DEFAULT_FLOAT_GPIO|APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY1_GPIO|APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY2_GPIO|APP_TEMPLATE_HW_DEFAULT_DS18B20_GPIO|APP_TEMPLATE_HW_DEFAULT_COOLING_RELAY_GPIO" components/app_config/app_config.h components/hardware_map` | N/A command | green |
| 06-01-03 | 01 | 1 | HW-01, HW-02 | T-06-01 | Map validation rejects unsafe role/GPIO combinations and duplicate active assignments. | source check | `rg "hardware_map_defaults|hardware_map_validate|hardware_map_gpio_allowed_for_role|hardware_map_equal|hardware_map_reboot_required|hardware_map_role_name" components/hardware_map` | N/A command | green |
| 06-01-04 | 01 | 1 | HW-04 | T-06-06 | Installer wiring assumptions and relay polarity warnings are documented before runtime use. | documentation check | `rg "hardware_map|GPIO32|GPIO26|GPIO27|GPIO33|GPIO25|DS18B20|4.7|3.3|minimum off|compressor" docs README.md` | N/A command | green |
| 06-02-01 | 02 | 2 | HW-03, HW-05 | T-06-03 | Storage structs expose independent relay polarity, timer phases, cooling settings, and hardware map APIs. | source check | `rg "hardware_map|relay1_active_low|relay2_active_low|timer1_start_phase|timer2_start_phase|nvs_store_cooling_settings_t|nvs_store_hardware" components/nvs_store` | N/A command | green |
| 06-02-02 | 02 | 2 | HW-05 | T-06-01, T-06-03 | Active and pending maps are distinct and validated before use. | source check | `rg "NVS_HW_NAMESPACE|act_float|act_r1|act_r2|act_ds|act_cool|pend_valid|pend_float|nvs_store_load_hardware_map|nvs_store_save_hardware_map|nvs_store_load_pending_hardware_map|nvs_store_hardware_reboot_required" components/nvs_store` | N/A command | green |
| 06-02-03 | 02 | 2 | HW-03, HW-05 | T-06-03 | Legacy Relay 1 settings remain compatible while Relay 2 polarity and timer phases are additive. | source check | `rg "r1_low|r2_low|t1_start|t2_start|relay_low|timer1_start_phase|timer2_start_phase|relay1_active_low|relay2_active_low" components/nvs_store/nvs_store.c components/nvs_store/nvs_store.h` | N/A command | green |
| 06-02-04 | 02 | 2 | HW-03, HW-05 | T-06-03, T-06-04 | Cooling settings default safe-off and remain persistence-only in Phase 6. | source check | `rg "NVS_COOL_NAMESPACE|threshold_x10|hyst_x10|auto_en|test_tmo_s|min_off_s|cooling_settings|nvs_store_load_cooling_settings|nvs_store_save_cooling_settings" components/nvs_store` | N/A command | green |
| 06-02-05 | 02 | 2 | HW-05 | T-06-06 | NVS schema, pending-map semantics, and legacy key migration are documented. | documentation check | `rg "hw_cfg|pump_cfg|cool_cfg|pending|reboot|required|relay_low|r1_low|r2_low|t1_start|t2_start" docs/hardware.md docs/components.md` | N/A command | green |
| 06-03-01 | 03 | 3 | HW-01, HW-02, HW-05 | T-06-01 | Build graph includes only required hardware map dependencies. | source check | `rg "hardware_map" main/CMakeLists.txt components/nvs_store/CMakeLists.txt` | N/A command | green |
| 06-03-02 | 03 | 3 | HW-01, HW-02, HW-03, HW-05 | T-06-03, T-06-04 | Boot consumes active/default map safely and suppresses auto-start on invalid persisted state. | source check | `rg "nvs_store_load_hardware_map|hardware_reboot_required|pump_relay1|pump_relay2|ds18b20|cooling|auto_start" main/app_main.c main/web_server.c` | N/A command | green |
| 06-03-03 | 03 | 3 | HW-03, HW-05 | T-06-02, T-06-05 | Existing pump API remains compatible and hardware foundation fields are read-only. | source check | `rg "read_only|float_gpio|relay_gpio|pump_relay1_gpio|pump_relay2_gpio|cooling_relay_gpio" main/web_server.c` | N/A command | green |
| 06-03-04 | 03 | 3 | HW-03, HW-05 | T-06-03 | Storage migration and invalid-data fail-closed behavior are source-verified. | source check | `rg "relay_low|r1_low|r2_low|t1_start|t2_start|hardware_map_validate|pend_valid|threshold_x10|min_off_s" components/nvs_store/nvs_store.c` | N/A command | green |
| 06-03-05 | 03 | 3 | HW-01, HW-02, HW-03, HW-04, HW-05 | T-06-01..T-06-06 | Firmware builds with the complete hardware foundation and documentation handoff exists. | build + documentation check | `.\scripts\build.ps1`; `Test-Path build\fish_pump_relay_timer_control.bin`; `rg "Phase 6|hardware_map|active hardware map|pending hardware map|Relay 2|cooling relay" docs/components.md docs/development-notes.md` | N/A command | green |

---

## Wave 0 Requirements

Existing command-based infrastructure covers all Phase 6 requirements.

No generated test files are required for this phase because:

- Phase 6 adds firmware contracts, NVS schemas, boot integration, API read-only fields, and documentation.
- Each plan task included an automated `rg` or ESP-IDF build command.
- Phase verification, UAT, and security artifacts are already complete and green.

---

## Manual-Only Verifications

All Phase 6 requirement behaviors have automated command coverage.

Hardware flash and physical relay/float/cooling behavior remain later-phase validation scope because Phase 6 intentionally does not add dual-relay, DS18B20, or cooling runtime behavior.

---

## Validation Audit 2026-05-23

| Metric | Count |
|--------|-------|
| Requirements audited | 5 |
| Plan tasks audited | 14 |
| Gaps found | 0 |
| Resolved | 0 |
| Escalated | 0 |

Commands rerun during this audit:

- `rg "hardware_role_t|hardware_map_t|hardware_map_defaults|hardware_map_validate" components\hardware_map`
- `rg "APP_TEMPLATE_HW_DEFAULT_FLOAT_GPIO|APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY1_GPIO|APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY2_GPIO|APP_TEMPLATE_HW_DEFAULT_DS18B20_GPIO|APP_TEMPLATE_HW_DEFAULT_COOLING_RELAY_GPIO" components\app_config\app_config.h components\hardware_map`
- `rg "DS18B20|4\.7|3\.3|powered mode|minimum off|compressor" docs\hardware.md`
- `rg "nvs_store_load_hardware_map|nvs_store_save_hardware_map|nvs_store_load_pending_hardware_map|nvs_store_hardware_reboot_required|relay1_active_low|relay2_active_low|timer1_start_phase|timer2_start_phase|nvs_store_cooling_settings_t" components\nvs_store`
- `.\scripts\build.ps1`
- `Test-Path build\fish_pump_relay_timer_control.bin`

---

## Validation Sign-Off

- [x] All tasks have automated verify commands.
- [x] Sampling continuity: no 3 consecutive tasks without automated verify.
- [x] Wave 0 covers all missing references; no Wave 0 files required.
- [x] No watch-mode flags.
- [x] Feedback latency < 60s for full suite.
- [x] `nyquist_compliant: true` set in frontmatter.

**Approval:** approved 2026-05-23
