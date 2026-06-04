# Pitfalls Research

**Domain:** Embedded offline ESP32 owner dashboard UI polish
**Researched:** 2026-06-02
**Confidence:** HIGH

## Critical Pitfalls

### Rewriting Stable Runtime During Cosmetic Work

**What goes wrong:** relay, timer, cooling, Wi-Fi polling, or APSTA behavior regresses during a UI milestone.

**Avoidance:** keep changes in static assets and docs by default. Allow firmware edits only for a verified UI state bug. Run `idf.py build` and preserve the passed `13:38:10` soak assumptions.

### Equal-Weight Dashboard Noise

**What goes wrong:** the owner sees many cards but cannot quickly answer whether the pump and cooling system are safe.

**Avoidance:** prioritize running/stopped state, active timer/relay, countdown, float state, temperature, cooling mode, and faults. Move configuration and diagnostics below operational state.

### Color-Only Or Visual-Only State

**What goes wrong:** status changes, errors, and focus are difficult to perceive with keyboard use or assistive technology.

**Avoidance:** combine text with color, provide visible focus treatment, preserve labels/instructions, and expose dynamic status messages programmatically.

### Mobile Wiring Flow That Only Works On Desktop

**What goes wrong:** installer cannot read wiring guidance or safely save pending GPIO changes while holding a phone near the board.

**Avoidance:** verify narrow viewport reflow, comfortable controls, readable summaries, and clear reboot confirmation.

### Missing DS18B20 Pull-Up Hardware Readiness

**What goes wrong:** the next hardware cycle starts without the required external resistor, producing unreliable or missing temperature readings.

**Avoidance:** document and surface the external `4.7 kOhm` pull-up between DS18B20 DQ and `3.3 V`.

### Polished Happy Path, Weak Edge States

**What goes wrong:** loading, error, empty scan results, disconnected Wi-Fi, unavailable sensor, and pending reboot states appear accidental or ambiguous.

**Avoidance:** define a state matrix and screenshot representative non-happy-path states.

## Performance Traps

| Trap | Symptoms | Prevention |
|------|----------|------------|
| Adding a frontend framework | Larger embedded assets and new tooling | Keep plain HTML/CSS/JS. |
| Adding aggressive polling | More HTTP traffic and possible long-uptime regression | Preserve existing intervals unless evidence requires a change. |
| Unbounded generated DOM | Increasing browser memory and sluggish updates | Replace bounded regions; keep lists bounded by API results. |
| Decorative assets and animation | Flash growth and lower-end rendering cost | Prefer CSS hierarchy and minimal motion. |

## Looks Done But Is Not

- [ ] Dashboard screenshot shows a clear primary state within three seconds.
- [ ] Dashboard and Hardware/Install screenshots pass at desktop and narrow mobile widths.
- [ ] Loading, error, empty, disabled, and pending reboot states are intentionally presented.
- [ ] Keyboard focus remains visible and controls have understandable labels.
- [ ] DS18B20 wiring documentation explicitly shows DQ -> `4.7 kOhm` pull-up -> `3.3 V`.
- [ ] `idf.py build` succeeds and embedded asset changes remain memory/performance safe.
- [ ] No firmware runtime behavior changed without a verified UI state bug.

## Sources

- W3C WCAG 2.2: https://www.w3.org/TR/WCAG22/
- W3C WAI Understanding Status Messages: https://www.w3.org/WAI/WCAG22/Understanding/status-messages
- W3C WAI Understanding Focus Visible: https://www.w3.org/WAI/WCAG22/Understanding/focus-visible
- W3C WAI Understanding Error Identification: https://www.w3.org/WAI/WCAG22/Understanding/error-identification.html
- Local v1.1 soak record: `.planning/STATE.md`

---
*Pitfalls research for: embedded offline ESP32 owner dashboard UI polish*
*Researched: 2026-06-02*
