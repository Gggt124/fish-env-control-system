---
generated: 2026-05-18
type: project-research
dimension: features
---

# Research: Features

## Table Stakes

- Timer 1 and Timer 2 each have ON and OFF durations.
- Durations can be edited from the web UI as minutes and seconds.
- Durations persist across reboot.
- User can start and stop pump control from the web UI.
- Auto-start on boot is enabled by default and configurable.
- Float switch is binary, using pull-up input with switch-to-GND active-low wiring.
- Float OFF selects Timer 1.
- Float ON selects Timer 2.
- Timer switch resets the newly active timer to ON phase.
- Relay output is active only during ON phase.
- UI shows active timer, phase, countdown, float state, relay state, and running state.

## Differentiators For Later

- Configurable float debounce duration.
- Configurable relay GPIO and float GPIO from web UI.
- Run history or event log.
- Manual relay test mode with explicit safety interlock.
- Export/import settings.

## Required Safety Features

- Relay output must initialize to inactive before control starts.
- Relay polarity must be configurable or at least centralized in config.
- Stop must immediately de-energize relay.
- Invalid timer values should be rejected rather than allowing zero-duration loops.
- Firmware should report whether auto-start is enabled and whether the controller is currently running.

## UI Features

- Add a pump control page or extend dashboard with:
  - Timer settings form
  - Start/Stop control
  - Auto-start toggle
  - Countdown clock
  - Float state indicator
  - Relay state indicator
- Preserve Wi-Fi/status navigation.

