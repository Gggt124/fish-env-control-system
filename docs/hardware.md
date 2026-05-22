# Hardware Contract

Phase 6 defines the firmware-owned hardware contract for the ESP32 DevKit V1
30-pin target. It does not add dual-relay pump behavior, DS18B20 readings,
cooling relay control, or installer mutation APIs.

## Board Assumptions

- Target board: classic ESP32 / ESP32 DevKit V1 30-pin.
- Logic rail: 3.3 V ESP32 GPIO.
- Relay modules vary; active-low is the conservative default, but polarity is
  modeled independently for pump Relay 1, pump Relay 2, and the cooling relay.
- Avoid flash pins, programming UART pins, strapping pins, and input-only pins
  for output roles.

## Firmware Roles

The `hardware_map` component exposes `hardware_role_t` values for:

| Role | Default GPIO | Notes |
|------|--------------|-------|
| Float input | GPIO32 | Existing validated switch input, active-low to GND with pull-up. |
| Pump Relay 1 | GPIO26 | Existing validated pump relay output. |
| Pump Relay 2 | GPIO27 | Conservative second pump relay output. |
| DS18B20 data | GPIO33 | Bidirectional one-wire data GPIO. |
| Cooling relay | GPIO25 | Dedicated cooling relay output. |

Safe GPIO choices are role-specific option lists, not freeform numeric input.
Output roles exclude GPIO34-GPIO39 because those ESP32 pins are input-only.
Normal output options also avoid GPIO0, GPIO2, GPIO5, GPIO6-GPIO12, GPIO15, and
GPIO16-GPIO17 to avoid boot-strapping, flash, PSRAM, and common module hazards.

## DS18B20 Powered-Mode Wiring

Phase 6 assumes powered mode for the DS18B20:

- DS18B20 VDD to 3.3 V.
- DS18B20 GND to ESP32 GND.
- DS18B20 DQ to the configured data GPIO, default GPIO33.
- Add an external 4.7 kOhm pull-up resistor from DQ to 3.3 V.
- Label the connector pins as `3V3`, `DQ`, and `GND`.

Do not rely on the ESP32 internal pull-up alone for the one-wire bus. Cooling
runtime belongs to a later phase and must fail with the cooling relay OFF if the
sensor is missing or unreadable.

## Compressor Protection Defaults

The storage defaults are defined before runtime enforcement:

| Setting | Default |
|---------|---------|
| Cooling threshold | 30.0 C |
| Cooling hysteresis | 1.0 C |
| Cooling auto-enable | false |
| Cooling mode | force off |
| Test-on timeout | 10 seconds |
| Compressor minimum off-time | 300 seconds |

The 300 second minimum off-time is a conservative compressor-protection default.
If the downstream hardware is only a fan, later UI/API phases can expose it as a
setting while preserving the safe default.

## NVS Schema

The `nvs_store` component owns all raw keys.

Namespace `hw_cfg` stores active and pending hardware maps:

| Group | Keys | Intent |
|-------|------|--------|
| Active map | `act_float`, `act_r1`, `act_r2`, `act_ds`, `act_cool` | GPIOs used at boot after validation. |
| Pending map | `pend_valid`, `pend_float`, `pend_r1`, `pend_r2`, `pend_ds`, `pend_cool` | Saved installer changes that require reboot before becoming active. |

Pending GPIO values do not change runtime behavior immediately. Firmware reports
reboot required when a valid pending map exists and differs from the active map.
Phase 9 will expose authenticated mutation APIs for this schema.

Namespace `pump_cfg` keeps v1.0 keys and adds v1.1 fields:

| Key | Intent |
|-----|--------|
| `t1_on`, `t1_off`, `t2_on`, `t2_off` | Existing timer durations. |
| `relay_low` | Legacy Relay 1 polarity compatibility alias. |
| `auto_start` | Existing pump auto-start setting. |
| `r1_low`, `r2_low` | Independent Relay 1 and Relay 2 active-low settings. |
| `t1_start`, `t2_start` | Timer start phase, `on` or `off`. |

If `r1_low` is missing, firmware seeds Relay 1 from legacy `relay_low`. Missing
`r2_low` and timer start-phase keys default safely without erasing saved timer
durations.

Namespace `cool_cfg` stores cooling defaults:

| Key | Intent |
|-----|--------|
| `threshold_x10` | Cooling threshold in tenths of a degree C. |
| `hyst_x10` | Hysteresis in tenths of a degree C. |
| `auto_en` | Cooling auto-enable on boot. |
| `mode` | Cooling mode: auto, force off, or test on. |
| `test_tmo_s` | Test-on timeout seconds. |
| `min_off_s` | Compressor minimum off-time seconds. |
| `relay_low` | Cooling relay active-low setting. |
