# Requirements

## Active

### REQ-001: Pump Control Uses Two Timer/Relay Channels

Pump control shall use two separate timer/relay channels selected by the binary float switch state.

- `Float ON` selects Timer 1 and Relay 1.
- `Float OFF` selects Timer 2 and Relay 2.
- The relay for the non-selected timer shall be forced OFF.
- The UI shall display float state as `Float ON` / `Float OFF` to avoid interpreting the switch as a continuous water-level sensor.
- The float input polarity shall remain configurable so wiring can be corrected without code changes.

### REQ-002: Float Changes Restart The Selected Timer Cycle

When the float switch changes state, the active timer cycle shall be interrupted and the newly selected timer shall start a fresh cycle.

- On float state change, stop the previously active timer and turn its relay OFF immediately.
- Select the new timer/relay from the new float state.
- Start the new timer cycle from that timer's configured start phase.
- Timer 1 and Timer 2 shall each have an independent `start phase` setting: `ON` or `OFF`.
- Timer 1 and Timer 2 shall both default to start with `ON`.
- If pump control is disabled, Relay 1 and Relay 2 shall remain OFF while float status continues to be read and displayed.

### REQ-003: Temperature Cooling Control Uses DS18B20 And Dedicated Relay

The firmware shall add a cooling temperature-control channel separate from the pump timer relays.

- The temperature sensor shall be DS18B20.
- The cooling relay shall be separate from Relay 1 and Relay 2.
- Cooling mode is cooling-only: relay ON when temperature is at or above the threshold, relay OFF after temperature falls below the threshold by the hysteresis amount.
- Default threshold shall be `30°C`.
- Default hysteresis shall be `1°C`.
- With the defaults, cooling relay ON at `>= 30°C` and OFF at `<= 29°C`.
- If DS18B20 is missing, disconnected, or unreadable, the cooling relay shall be forced OFF.
- Sensor fault status shall be visible in API/UI as not connected or unavailable.
- Temperature settings shall not use koi-only limits; validation should reject invalid numeric values while allowing a broad DS18B20-appropriate operating range.

### REQ-004: Cooling Enable, Auto-Enable, Override, And Protection Behavior

Cooling control shall be explicitly enabled by the owner and shall fail safe when sensor state is unknown.

- Cooling control shall default to disabled after boot/reboot.
- When disabled, the cooling relay shall be OFF.
- The owner shall be able to configure `auto-enable on boot`.
- `auto-enable on boot` shall default to OFF.
- If `auto-enable on boot` is ON but DS18B20 is not ready at boot, cooling control shall remain enabled while the relay stays OFF and a sensor fault is shown.
- The system should assume the cooling load might be a compressor or mains-switched chiller unless proven otherwise.
- Compressor protection / minimum off-time shall be part of the cooling design and enabled by default; exact duration remains to be decided before implementation.
- Override modes shall include `Auto`, `Force OFF`, and `Test ON`.
- `Test ON` shall default to 10 seconds, shall not persist across reboot, and shall return to the previous mode after timeout.
- If compressor protection is active, `Test ON` shall respect the protection rules.

### REQ-005: Web Hardware GPIO Map Uses Safe Enum And Pending Reboot

The web UI shall expose GPIO mapping in an owner-accessible Hardware/Install flow with guardrails.

- GPIO selection shall use firmware-defined safe enum/dropdown options, not freeform numeric input.
- The enum shall expose only recommended/safe GPIOs for each role.
- GPIO changes shall be saved to persistent storage but shall not take effect until reboot.
- The UI shall show pending reboot state after GPIO map changes.
- If pending GPIO values differ from active GPIO values, the UI shall show both.
- Saving GPIO changes shall require a confirmation checkbox stating that reboot is required and wiring must match the new GPIO selection.
- The Hardware/Install page shall prioritize mobile wiring visualization for installers.
- The Hardware/Install page shall include a GPIO summary table and may include a technical pinout view as a secondary tab.
- The owner dashboard shall keep daily operation simple while showing temperature, cooling relay state, threshold, and sensor connection status.
