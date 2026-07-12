# Hardware Contract

Phase 6 defined the firmware-owned hardware contract for the ESP32 DevKit V1
30-pin target. Runtime code now consumes the float input, pump Relay 1, pump
Relay 2, DS18B20 data, and cooling relay roles. Phase 10 adds the protected
Hardware/Install page at `/hardware` for owner-accessible wiring review and
pending GPIO map edits.

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

## Locked System Pins

The firmware reserves certain pins for system UI (LEDs, buttons). These pins are exposed as `locked_pins` in the `/api/hardware/map` payload and cannot be remapped. Their assignments depend on the target profile:

| Role | ESP32 Classic | ESP32-S3 | Notes |
|------|---------------|----------|-------|
| Status LED | GPIO2 (ON/OFF) | GPIO48 (RGB) | S3 uses WS2812. Classic uses simple ON/OFF. |
| EXT BTN | GPIO14 | GPIO38 | External menu navigation button. |
| EXT LED | GPIO13 | GPIO39 | External status indicator. |

## DS18B20 Powered-Mode Wiring

Phase 6 assumes powered mode for the DS18B20:

- DS18B20 VDD to 3.3 V.
- DS18B20 GND to ESP32 GND.
- DS18B20 DQ to the configured data GPIO, default GPIO33.
- Add an external 4.7 kOhm pull-up resistor from DQ to 3.3 V.
- Label the connector pins as `3V3`, `DQ`, and `GND`.

Do not rely on the ESP32 internal pull-up alone for the one-wire bus. Phase 8
cooling runtime reads this sensor through the `cooling_control` component and
fails with the cooling relay OFF if the sensor is missing or unreadable.

## Compressor Protection Defaults

The storage defaults are defined before runtime enforcement:

| Setting | Default |
|---------|---------|
| Cooling threshold | 30.0 C |
| Cooling hysteresis | 1.0 C |
| Cooling auto-enable | false |
| Cooling mode | force off |
| Test-on timeout | 10 seconds |
| Compressor minimum off-time | 10 seconds |

The minimum off-time is enforced before Auto or Test ON can energize the cooling
relay. Phase 10 exposes it as `Lockout seconds` on the dashboard so local
installations can tune it without reflashing.

## NVS Schema

The `nvs_store` component owns all raw keys.

Namespace `hw_cfg` stores active and pending hardware maps:

| Group | Keys | Intent |
|-------|------|--------|
| Active map | `act_float`, `act_r1`, `act_r2`, `act_ds`, `act_cool` | GPIOs used at boot after validation. |
| Pending map | `pend_valid`, `pend_float`, `pend_r1`, `pend_r2`, `pend_ds`, `pend_cool` | Saved installer changes that require reboot before becoming active. |

Pending GPIO values do not change runtime behavior immediately. Firmware reports
reboot required when a valid pending map exists and differs from the active map.
On boot, firmware promotes a valid pending map to the active map before pump and
cooling runtime initialization, then clears pending state. The Hardware/Install
UI shows active and pending values separately, requires a reboot confirmation
checkbox, and saves pending maps through `/api/hardware/map` with
`confirm_reboot_required: true`.

## Hardware/Install UI Flow

The `/hardware` page is protected by the same local session as the dashboard.
It is intended for setup and service work, not daily pump operation.

1. Review the wiring summary first. It shows the currently active GPIO for each
   role and a pending GPIO when one has been saved but not rebooted into use.
2. Review the active and pending map summaries.
3. Choose new role mappings from dropdowns populated by firmware safe option
   arrays. Do not add freeform numeric GPIO entry.
4. Check the reboot acknowledgement and save. The submitted map is pending only;
   existing runtime GPIOs stay active until reboot.
5. Reboot. The firmware promotes the pending map to active before initializing
   pump and cooling GPIOs, then clears the pending state.
6. After reboot, verify active GPIOs and relay polarity before connecting a real
   pump or compressor load.

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
