---
generated: 2026-05-18
type: project-research
---

# Research Summary

## Stack

Keep ESP-IDF C and the existing component style. Add `components/pump_control/` for timer, float switch, relay, persistence-facing config, and status state. Keep web server and frontend as adapters.

## Table Stakes

- Two timers with ON/OFF durations.
- Binary float switch selects Timer 1 or Timer 2.
- Relay follows active timer phase.
- Start/stop and auto-start setting in web UI.
- Timer settings and auto-start persist.
- Status API/UI reports active timer, phase, countdown, float state, relay state.

## Watch Out For

- Float switch is not a level sensor.
- Relay output must fail inactive at boot and stop.
- Relay polarity must be configurable or centralized.
- Avoid ESP32 strapping pins and input-only pins for default hardware choices.
- Do not block HTTP handlers with control-loop work.

## Recommended First Phase Shape

Use vertical MVP phases rather than pure horizontal layers: first land a safe pump-control core with persisted config, then expose API/UI, then hardware validation and polish.

## Sources

- ESP-IDF GPIO guide: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html
- ESP-IDF esp_timer guide: https://docs.espressif.com/projects/esp-idf/en/v5.1/esp32/api-reference/system/esp_timer.html
- ESP32 datasheet boot/strapping pins: https://documentation.espressif.com/esp32_datasheet_en.html
- Project design references: `ref-file/flow-document-v6.md`, `ref-file/flow-diagram-5.html`

