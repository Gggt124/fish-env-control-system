---
generated: 2026-05-18
type: project-research
dimension: stack
---

# Research: Stack

## Recommendation

Keep the implementation in ESP-IDF C using the existing component layout. Add a reusable pump-control component rather than putting timer/relay logic directly in `main/web_server.c`.

## Existing Fit

- The existing repo already has clean component boundaries under `components/`.
- `nvs_store` already persists Wi-Fi settings and can be extended or mirrored with a pump settings namespace.
- `web_server` already serves JSON APIs and embedded UI, so timer settings and status endpoints fit the current pattern.
- The static UI has no external dependency, which is correct for SoftAP operation.

## ESP-IDF APIs To Use

- GPIO: use `gpio_config()` from `driver/gpio.h` for float input and relay output. Espressif documents `gpio_config()` as the API for I/O mode and pull-up/pull-down configuration: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html
- Timer: use `esp_timer` or a FreeRTOS task loop for 1-second countdown behavior. Espressif documents `esp_timer_start_periodic()` for periodic callbacks and warns to keep callbacks short: https://docs.espressif.com/projects/esp-idf/en/v5.1/esp32/api-reference/system/esp_timer.html
- Persistence: continue NVS pattern already used by `components/nvs_store/`.

## Pin Guidance

- GPIO32 is a reasonable float input because it is an ESP32 I/O pin and supports pull configuration.
- GPIO26 is a reasonable relay output because it is an ESP32 I/O pin and avoids common boot/programming pins.
- Avoid GPIO34-39 for pull-up switch inputs because Espressif documents them as input-only without software-enabled pull-up/pull-down.
- Avoid boot strapping pins for first hardware defaults: GPIO0, GPIO2, MTDI/GPIO12, MTDO/GPIO15, and GPIO5.

## Implementation Shape

- New component: `components/pump_control/`
- Public API examples:
  - `pump_control_init()`
  - `pump_control_start()`
  - `pump_control_stop()`
  - `pump_control_get_status()`
  - `pump_control_save_config()`
  - `pump_control_load_config()`
- `main/web_server.c` should call the component API and remain HTTP glue.

