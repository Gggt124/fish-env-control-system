# Project State

## Project Reference

See: `.planning/PROJECT.md` (updated 2026-05-18)

**Core value:** The pump must switch reliably between Timer 1 and Timer 2 based on the float switch and drive the relay safely according to the selected timer's ON/OFF cycle.  
**Current focus:** Phase 1: Hardware-Safe Pump Control Core

## Workflow

- Mode: yolo
- Granularity: standard
- Parallelization: enabled
- Research: enabled
- Plan check: enabled
- Verifier: enabled
- Project structure: vertical MVP

## Artifacts

- Codebase map: `.planning/codebase/`
- Project context: `.planning/PROJECT.md`
- Config: `.planning/config.json`
- Research: `.planning/research/`
- Requirements: `.planning/REQUIREMENTS.md`
- Roadmap: `.planning/ROADMAP.md`

## Current Phase

### Phase 1: Hardware-Safe Pump Control Core

**Goal:** Add the pump-control component with safe GPIO defaults, relay inactive initialization, timer state machine, and build validation.

**Requirements:** HW-01, HW-02, HW-03, HW-04, HW-05, HW-06, TIME-05, TIME-06, TIME-07, RUN-03, RUN-06, RUN-07, RUN-08, RUN-09, RUN-10, RUN-11, VAL-01

**Status:** Ready for `$gsd-discuss-phase 1` or `$gsd-plan-phase 1`.

## Notes

- User confirmed ESP32 DevKit V1 30-pin.
- Recommended float switch wiring: GPIO32 input pull-up, switch closes to GND.
- Recommended relay default: GPIO26 output, polarity configurable.
- Auto-start defaults enabled but must be persisted and user-toggleable.

---
*State initialized: 2026-05-18*
