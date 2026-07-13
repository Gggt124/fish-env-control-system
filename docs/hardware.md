# Hardware Contract

The firmware-owned hardware contract supports both the ESP32 DevKit V1 30-pin
target and ESP32-S3-DevKitC-1 WROOM-1-N16R8. Runtime code consumes the float
input, pump Relay 1, pump Relay 2, DS18B20 data, and cooling relay roles. The
protected Hardware/Install page at `/hardware` exposes only the role-specific
application GPIO allowlist.

## Board Assumptions

- Target board profile is selected at build time: classic ESP32 / ESP32 DevKit
  V1 30-pin or ESP32-S3-DevKitC-1 WROOM-1-N16R8.
- Logic rail: 3.3 V ESP32 GPIO.
- Relay modules vary; active-low is the conservative default, but polarity is
  modeled independently for pump Relay 1, pump Relay 2, and the cooling relay.
- Avoid flash pins, programming UART pins, strapping pins, and input-only pins
  for output roles.
- On the S3 profile, GPIO19/20 are reserved for Native USB D-/D+, GPIO43/44
  are reserved for the UART console, and GPIO27-37 are reserved by Octal
  Flash/PSRAM. GPIO0/3/45/46 are strapping pins; GPIO21, 38-42, and 47 are
  reserved because they participate in USB OTG boot/download behavior.
- S3 TFT/system pins are fixed at GPIO9-14, GPIO0, GPIO38, GPIO39, and GPIO48.
  The application allowlist is intentionally limited to GPIO1/2, GPIO5-8, and
  GPIO15-18.

## Firmware Roles

The `hardware_map` component exposes `hardware_role_t` values for:

| Role | ESP32 Classic | ESP32-S3 | Notes |
|------|---------------|----------|-------|
| Float input | GPIO32 | GPIO16 | Active-low to GND with internal pull-up plus external 4.7 kOhm pull-up. |
| Pump Relay 1 | GPIO26 | GPIO5 | Dedicated relay output; polarity is configurable. |
| Pump Relay 2 | GPIO27 | GPIO6 | Dedicated relay output; polarity is configurable. |
| DS18B20 data | GPIO33 | GPIO7 | Bidirectional one-wire data with external 4.7 kOhm pull-up. |
| Cooling relay | GPIO25 | GPIO8 | Dedicated relay output; polarity is configurable. |

Safe GPIO choices are role-specific option lists, not freeform numeric input.
The `hardware_map` component owns the typed pin descriptors, capabilities,
reserved-pin policy, role allowlists, duplicate detection, and validation used
by NVS, the API, and boot-time runtime configuration.
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
| Boot Button | GPIO0 | GPIO0 | Strapping pin. |

## DS18B20 Powered-Mode Wiring

Phase 6 assumes powered mode for the DS18B20:

- DS18B20 VDD to 3.3 V.
- DS18B20 GND to ESP32 GND.
- DS18B20 DQ to the configured data GPIO (GPIO33 on classic ESP32, GPIO7 on
  ESP32-S3).
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
