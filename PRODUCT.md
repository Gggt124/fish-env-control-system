# Product

## Register

product

## Users

The primary user is the fish-pump system owner operating the controller day to
day. The secondary user is the installer using a phone near the ESP32 board
while checking wiring, Wi-Fi setup, and pending GPIO changes.

Both users need a local interface that remains usable without internet access.
The owner needs fast confidence that pump and cooling behavior are safe. The
installer needs short, explicit hardware guidance that reduces wiring mistakes.

## Product Purpose

Fish Pump Relay Timer Control is a local ESP32 appliance controller. Its web UI
lets the owner monitor and operate two timer-selected pump relays, inspect the
binary float switch state, configure cooling behavior, manage Wi-Fi, and review
hardware mapping without depending on cloud services.

The v1.2 UI work succeeds when current state is understandable within seconds,
setup and install flows are clear on a phone, edge states are intentional, and
the stable v1.1 firmware behavior remains unchanged.

## Brand Personality

Calm, clear, trustworthy.

The interface should feel like a practical local control panel. It should use
concise Thai for owner-facing guidance and retain familiar English technical
terms where they improve precision.

## Anti-references

- Generic SaaS dashboards with equal-weight card grids and decorative metrics.
- Marketing-style hero sections, excessive whitespace, and ornamental copy.
- Decorative animation, gradients, and glass effects. (Note: Dark Mode is intentionally implemented and fully supported for low-light/night operational monitoring, so it is not an anti-reference here).
- External fonts, CDN assets, remote icon packages, and framework migrations
  that weaken offline ESP32 operation.
- Dense installer screens that hide active versus pending GPIO state or reboot
  requirements.

## Design Principles

1. Show the operator's immediate decision first: what is running, what is
   selected, what is safe, and what needs attention.
2. Keep daily operation separate from advanced installation work.
3. Make every temporary or exceptional state explicit: loading, empty, error,
   disabled, pending reboot, sensor fault, and disconnected network.
4. Prefer familiar controls, concise wording, and restrained visual hierarchy
   over decoration.
5. Preserve the offline embedded footprint and stable v1.1 runtime behavior.

## Accessibility & Inclusion

Use WCAG 2.2 AA-oriented practices for the embedded local UI:

- Maintain readable contrast and never rely on color alone for meaning.
- Keep keyboard navigation usable with visible focus treatment.
- Provide clear labels and instructions for controls.
- Expose dynamic status and error messages programmatically.
- Support narrow mobile reflow and comfortable touch targets.
- Respect reduced-motion preferences for any motion added during polish.

