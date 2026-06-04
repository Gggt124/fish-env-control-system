---
phase: 06-hardware-contract-and-persistent-map-foundation
generated: 2026-05-22
mode: inline-codex
context: none - user chose to continue without 06-CONTEXT.md
sources:
  - .planning/ROADMAP.md
  - .planning/REQUIREMENTS.md
  - .planning/STATE.md
  - .planning/codebase/ARCHITECTURE.md
  - .planning/codebase/CONVENTIONS.md
  - components/app_config/app_config.h
  - components/nvs_store/nvs_store.h
  - components/nvs_store/nvs_store.c
  - components/pump_control/pump_control.h
  - components/pump_control/pump_control.c
  - main/app_main.c
  - main/web_server.c
  - docs/components.md
external_sources:
  - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html
  - https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32/schematic-checklist.html
  - https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
  - https://www.analog.com/media/en/technical-documentation/data-sheets/ds18b20.pdf
  - https://support.analog.com/en-US/knowledgebase/article/000094969
---

# Phase 06 Research: Hardware Contract And Persistent Map Foundation

## RESEARCH COMPLETE

Phase 6 is a foundation phase. It should define a typed hardware contract, safe ESP32 DevKit V1 defaults, and durable configuration schemas before Phase 7 changes pump runtime behavior and Phase 8 adds DS18B20 cooling. It should not add the dual-relay pump switching behavior, DS18B20 readings, cooling relay control, or installer UI/API routes yet.

## Phase Boundary

In scope:

- Firmware-owned hardware roles for float input, pump Relay 1, pump Relay 2, DS18B20 data, and cooling relay.
- Role-specific safe GPIO option metadata and conservative defaults for classic ESP32 / ESP32 DevKit V1 30-pin.
- Independent relay polarity representation for pump Relay 1, pump Relay 2, and cooling relay.
- NVS schema and wrapper APIs for active hardware map, pending hardware map, relay polarity, cooling defaults, and timer start phases.
- Documentation of DS18B20 powered-mode wiring assumptions and compressor protection defaults.
- Integration that keeps current single-relay pump behavior equivalent by mapping the existing relay output to Relay 1 until Phase 7.

Out of scope:

- Dual pump relay runtime selection.
- Cooling runtime, one-wire bus transactions, DS18B20 temperature conversion, threshold enforcement, or compressor minimum-off enforcement.
- Hardware/install web UI and new GPIO mutation APIs.
- Relay pin changes that energize outputs unexpectedly during boot.

## Existing Implementation Surface

- `components/app_config/app_config.h` owns current default GPIOs and timer defaults.
- `components/pump_control/pump_control.h/.c` owns current single relay runtime, float input, timer selection, relay polarity, debounce, and status.
- `components/nvs_store/nvs_store.h/.c` owns Wi-Fi, AP, static IP, and pump settings persistence.
- `main/app_main.c` loads pump settings from NVS, merges them into `pump_control_default_config()`, initializes pump runtime, and optionally auto-starts.
- `main/web_server.c` exposes existing pump config/status/start/stop APIs and currently returns read-only `float_gpio`, `relay_gpio`, and `debounce_ms`.
- `docs/components.md` documents component boundaries but does not yet document pump hardware roles, DS18B20 wiring, or the new NVS schemas.

## Hardware Facts To Encode

Official ESP-IDF GPIO documentation says ESP32 has GPIO0-19, GPIO21-23, GPIO25-27, and GPIO32-39 exposed as GPIO-capable pads. It also calls out restrictions that matter here: GPIO0, GPIO2, GPIO5, GPIO12, and GPIO15 are strapping pins; GPIO6-11 and GPIO16-17 are usually connected to SPI flash/PSRAM on modules; GPIO34-39 are input-only and do not have software-enabled pull-up/pull-down; TXD/RXD are normally used for flashing/debugging.

Espressif hardware guidelines also emphasize that strapping pins are sampled at reset and that GPIO0/GPIO2 control boot mode. For an installer-facing product, the safe contract should avoid strapping pins by default and should not offer them as normal relay defaults.

The existing validated v1.0 defaults are worth preserving:

- Float input default: GPIO32, active-low switch-to-GND with internal pull-up.
- Pump Relay 1 default: GPIO26, active-low relay module.

Recommended v1.1 conservative defaults:

| Role | Default | Reason |
|------|---------|--------|
| Float input | GPIO32 | Already validated; supports input and internal pull-up. |
| Pump Relay 1 | GPIO26 | Already validated output; not a strapping pin. |
| Pump Relay 2 | GPIO27 | Adjacent safe output; not a strapping pin. |
| DS18B20 data | GPIO33 | Safe bidirectional GPIO; keeps one-wire separate from relay outputs. |
| Cooling relay | GPIO25 | Safe output near pump relay pins; not a strapping pin. |

Role-specific option lists should be firmware-defined, not freeform numeric input. Output roles should be limited to normal bidirectional GPIOs that avoid flash pins, UART0, strapping pins, and input-only pins. Float input options can include GPIO32/GPIO33 as internal-pull-capable defaults and may include input-only options only if metadata clearly marks that an external pull-up/down is required. DS18B20 data options should require bidirectional/open-drain-capable GPIOs and must not default to input-only GPIOs.

## DS18B20 Wiring Assumptions

The DS18B20 supports parasite power, but the datasheet states externally powered operation avoids the strong pull-up requirement during temperature conversion. Phase 6 should document powered mode as the project assumption:

- DS18B20 VDD to 3.3 V.
- DS18B20 GND to ESP32 GND.
- DS18B20 DQ to the configured data GPIO, default GPIO33.
- External pull-up from DQ to 3.3 V, nominal 4.7 kOhm / approximately 5 kOhm.
- Do not rely on ESP32 internal pull-up alone for the one-wire bus.
- Cooling runtime must treat missing/unreadable sensor as relay OFF in Phase 8.

Analog's DS18B20 datasheet shows the 1-Wire bus uses an external pull-up resistor and that externally powered operation keeps the bus available during temperature conversion. Analog support also confirms the 4.7 kOhm pull-up is required for both 5 V and 3.3 V applications.

## Compressor Protection Defaults

Phase 6 should define constants and storage fields only. The actual enforcement belongs to Phase 8.

Recommended defaults:

- Cooling threshold: 30.0 C.
- Cooling hysteresis: 1.0 C.
- Cooling auto-enable on boot: false.
- Cooling mode: force off / disabled default.
- Test-on timeout: 10 seconds.
- Compressor minimum off-time: 300 seconds.
- Cooling relay polarity: active-low by default, independent from both pump relays.

The 300-second default is deliberately conservative for compressor short-cycle protection. If the downstream hardware is only a fan, Phase 8/9 can keep this configurable while preserving safe defaults.

## Persistence Strategy

Keep the public storage boundary in `nvs_store`. Add typed structs and wrapper functions; do not let `main` or web code manipulate raw NVS keys.

Recommended schema:

- Namespace `hw_cfg`
  - Active map keys: `act_float`, `act_r1`, `act_r2`, `act_ds`, `act_cool`
  - Pending map keys: `pend_valid`, `pend_float`, `pend_r1`, `pend_r2`, `pend_ds`, `pend_cool`
  - Active map loads defaults when missing.
  - Pending map is optional; reboot-required is true when pending exists and differs from active.
- Namespace `pump_cfg`
  - Keep existing keys `t1_on`, `t1_off`, `t2_on`, `t2_off`, `relay_low`, `auto_start` for compatibility.
  - Add `r1_low`, `r2_low`, `t1_start`, `t2_start`.
  - Legacy `relay_low` seeds Relay 1 polarity when `r1_low` is missing and may seed Relay 2 default if needed.
  - Timer start phases default to ON for both timers.
- Namespace `cool_cfg`
  - Add `threshold_x10`, `hyst_x10`, `auto_en`, `mode`, `test_tmo_s`, `min_off_s`, `relay_low`.
  - Defaults are safe OFF and no auto-start.

Use integer storage for floats where practical (`threshold_x10`, `hyst_x10`) to keep NVS values simple and deterministic.

## Planning Recommendation

Use three plans:

1. Add a `hardware_map` component and hardware documentation.
2. Extend `nvs_store` with hardware map, relay polarity, cooling settings, and timer start-phase persistence.
3. Wire the active map into existing boot/status/config surfaces without changing runtime control semantics, update component docs, and build-verify.

This sequencing lets Phase 7 consume the hardware map and Relay 1/Relay 2 polarity cleanly, lets Phase 8 consume the DS18B20/cooling defaults, and keeps Phase 9 focused on authenticated mutation APIs instead of storage foundations.
