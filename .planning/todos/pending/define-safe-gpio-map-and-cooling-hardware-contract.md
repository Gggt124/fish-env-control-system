---
title: "Define safe GPIO map and cooling hardware contract"
date: "2026-05-22"
priority: "high"
resolves_phase: 6
---

# Define Safe GPIO Map And Cooling Hardware Contract

Before implementing the dual timer relays, DS18B20 cooling channel, or editable web GPIO map, define the hardware contract that the firmware and UI will enforce.

## Tasks

- Define safe GPIO enum options per role: float input, Relay 1, Relay 2, DS18B20 data, and cooling relay.
- Choose conservative default GPIO recommendations for ESP32 DevKit V1 30-pin.
- Define relay polarity defaults and whether each relay channel can configure polarity independently.
- Define DS18B20 wiring assumptions: powered mode vs parasite power, pull-up resistor expectation, voltage rail, and connector labeling.
- Define cooling compressor protection defaults, including minimum off-time duration and whether it can be disabled.
- Define NVS schema for active GPIO map, pending GPIO map, relay polarity, cooling settings, and timer start phases.
- Define API fields for active vs pending GPIO values and reboot-required status.
- Supersede the old pump mapping assumption where one relay followed the selected timer; the new design is Float ON -> Timer 1/Relay 1 and Float OFF -> Timer 2/Relay 2.

## Done When

- Firmware-safe GPIO options are documented.
- UI dropdown options can be derived from the same role-based enum.
- Implementation can proceed without inventing pin assignments inside code.
