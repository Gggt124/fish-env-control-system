---
phase: 06-hardware-contract-and-persistent-map-foundation
status: passed
verified: 2026-05-22
requirements:
  - HW-01
  - HW-02
  - HW-03
  - HW-04
  - HW-05
---

# Phase 06 Verification: Hardware Contract And Persistent Map Foundation

## Result

status: passed

Phase 6 achieved its phase goal: firmware now has a typed safe hardware contract,
active and pending hardware map persistence, independent relay polarity storage,
cooling defaults storage, boot-time consumption of the active map, and read-only
API/status exposure without adding dual-relay pump behavior, DS18B20 runtime, or
cooling relay control.

## Evidence

| Must Have | Evidence | Result |
|---|---|---|
| HW-01 safe GPIO roles exist | `components/hardware_map/hardware_map.h` defines `hardware_role_t` roles for float input, pump Relay 1, pump Relay 2, DS18B20 data, and cooling relay. | PASS |
| HW-02 conservative defaults exist | `components/app_config/app_config.h` defines GPIO32, GPIO26, GPIO27, GPIO33, and GPIO25 defaults; `hardware_map.c` validates role-specific option lists. | PASS |
| Output roles reject unsafe pins | Relay and DS18B20 option lists exclude GPIO34-GPIO39; flash, UART0, and strapping pins are not offered in normal output lists. | PASS |
| Duplicate role assignments rejected | `hardware_map_validate()` compares all active role GPIOs and rejects duplicates. | PASS |
| HW-03 independent polarity storage exists | `nvs_store_pump_settings_t` stores Relay 1 and Relay 2 polarity independently; `nvs_store_cooling_settings_t` stores cooling relay polarity separately. | PASS |
| HW-04 DS18B20 wiring documented | `docs/hardware.md` documents powered mode, DQ default GPIO33, 3.3 V rail, GND, 4.7 kOhm pull-up, and connector labels. | PASS |
| HW-05 active and pending map storage exists | `nvs_store.c` owns `hw_cfg` active keys and pending keys plus reboot-required computation. | PASS |
| Legacy pump settings remain compatible | `relay_low` is still read and written as the Relay 1 compatibility alias; `r1_low`, `r2_low`, `t1_start`, and `t2_start` are additive. | PASS |
| Cooling schema defaults safe off | `cool_cfg` defaults to force off, auto-enable false, 30.0 C threshold, 1.0 C hysteresis, 10 second test timeout, 300 second minimum off-time. | PASS |
| Boot consumes active map safely | `main/app_main.c` loads the active hardware map, maps float/Relay 1 into current runtime, logs Relay 2/DS18B20/cooling pins, and suppresses auto-start on invalid persisted hardware or pump settings. | PASS |
| Existing API remains compatible | `main/web_server.c` preserves existing `float_gpio`, `relay_gpio`, and `relay_polarity` fields while adding read-only foundation fields. | PASS |
| Mutation scope unchanged | No new hardware/cooling mutation route was added; `/api/pump/config` rejects new hardware/cooling fields as read-only. | PASS |
| Documentation handoff exists | `docs/components.md` and `docs/development-notes.md` document ownership boundaries and Phase 7-10 handoff rules. | PASS |
| Schema drift gate | `gsd-sdk query verify.schema-drift 06` returned `drift_detected:false`. | PASS |
| Firmware builds | `.\scripts\build.ps1` completed successfully with ESP-IDF 6.1. | PASS |
| Firmware binary exists | `Test-Path build\fish_pump_relay_timer_control.bin` returned `True`. | PASS |

## Automated Checks

- `rg "hardware_role_t|hardware_map_t|hardware_map_defaults|hardware_map_validate" components/hardware_map`
- `rg "APP_TEMPLATE_HW_DEFAULT_FLOAT_GPIO|APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY1_GPIO|APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY2_GPIO|APP_TEMPLATE_HW_DEFAULT_DS18B20_GPIO|APP_TEMPLATE_HW_DEFAULT_COOLING_RELAY_GPIO" components/app_config/app_config.h components/hardware_map`
- `rg "DS18B20|4.7|3.3|powered mode|minimum off|compressor" docs/hardware.md`
- `rg "hardware_map" components/nvs_store/CMakeLists.txt components/nvs_store/nvs_store.h`
- `rg "nvs_store_load_hardware_map|nvs_store_save_hardware_map|nvs_store_load_pending_hardware_map|nvs_store_hardware_reboot_required" components/nvs_store`
- `rg "relay1_active_low|relay2_active_low|timer1_start_phase|timer2_start_phase" components/nvs_store`
- `rg "nvs_store_cooling_settings_t|nvs_store_load_cooling_settings|nvs_store_save_cooling_settings" components/nvs_store`
- `rg "hw_cfg|pump_cfg|cool_cfg|pending|reboot" docs/hardware.md docs/components.md`
- `rg "hardware_map" main/CMakeLists.txt components/nvs_store/CMakeLists.txt`
- `rg "nvs_store_load_hardware_map|hardware_reboot_required|pump_relay1|pump_relay2|ds18b20|cooling" main/app_main.c main/web_server.c`
- `rg "read_only|float_gpio|relay_gpio|pump_relay1_gpio|pump_relay2_gpio|cooling_relay_gpio" main/web_server.c`
- `rg "relay_low|r1_low|r2_low|t1_start|t2_start|pend_valid|threshold_x10|min_off_s" components/nvs_store/nvs_store.c`
- `gsd-sdk query verify.schema-drift 06`
- `.\scripts\build.ps1`
- `Test-Path build\fish_pump_relay_timer_control.bin`

## Requirement Coverage

- **HW-01:** Covered by `hardware_map` roles, option lists, and validation APIs.
- **HW-02:** Covered by conservative ESP32 DevKit V1 defaults and role-specific safe GPIO lists.
- **HW-03:** Covered by independent Relay 1, Relay 2, and cooling relay polarity storage/reporting.
- **HW-04:** Covered by DS18B20 powered-mode wiring documentation in `docs/hardware.md`.
- **HW-05:** Covered by `hw_cfg`, `pump_cfg`, and `cool_cfg` typed NVS wrappers and schema documentation.

## Notes

- The first `.\scripts\build.ps1` invocation exceeded the 120 second tool timeout while the ESP-IDF process continued. A second incremental build completed successfully with captured output.
- Build output reported `fish_pump_relay_timer_control.bin` size `0xf4f00`, leaving `0xfb100` bytes free in the smallest app partition.

## Human Verification

None required for Phase 6. Hardware flashing and real relay/float/cooling behavior remain later phase validation scope because this phase only adds contract, persistence, boot consumption, and read-only reporting.

## Gaps

None.
