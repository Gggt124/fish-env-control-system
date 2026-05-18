---
generated: 2026-05-18
type: project-research
dimension: pitfalls
---

# Research: Pitfalls

## Treating Float Switch As Analog

The provided design explicitly says the float switch is binary only. Do not expose real firmware requirements as 0-100% water level; that belongs only to the old simulation UI.

## Relay Energized During Boot

Relay modules can be active-high or active-low. If the output level is wrong before configuration, the pump can turn on unexpectedly. Initialize relay GPIO early and make stop force inactive output.

## Bad Pin Choices

Avoid flash pins, UART programming pins, and boot strapping pins for first hardware defaults. Espressif documents strapping pins including GPIO0, GPIO2, MTDI, MTDO, and GPIO5 in the ESP32 datasheet: https://documentation.espressif.com/esp32_datasheet_en.html

## Input Pull-Up Assumptions

GPIO34-39 do not have software-enabled pull-up/pull-down on ESP32, so they are poor choices for the planned switch-to-GND float input. Espressif documents this in the ESP-IDF GPIO guide.

## Zero Or Tiny Durations

Zero ON/OFF durations can create rapid relay chatter. Enforce minimum durations and reject invalid config in both API and UI.

## Blocking HTTP Handlers

Avoid running the pump countdown from HTTP handlers. The controller should run in a component task/timer, and handlers should only update settings or return status.

## Lost AP Access

Existing Wi-Fi foundation must remain available while pump control runs. Avoid pump-control work that starves the HTTP server, Wi-Fi event loop, or watchdog.

