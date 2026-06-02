# Feature Research

**Domain:** Embedded offline ESP32 owner dashboard UI polish
**Researched:** 2026-06-02
**Confidence:** HIGH

## Table Stakes

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Dashboard state hierarchy | Owner must recognize whether pump and cooling are safe and active within seconds. | MEDIUM | Promote current state and primary actions; reduce equal-weight status noise. |
| Explicit loading, error, and empty states | APSTA and sensor/network operations can be temporarily unavailable. | MEDIUM | Use existing alert and renderer patterns; preserve last-known state only when clearly labeled. |
| Mobile-first responsive flow | Installer may use a phone while wiring or configuring Wi-Fi. | MEDIUM | Verify narrow viewport reflow and touch-friendly controls. |
| Accessible controls and status updates | Buttons, forms, and live feedback must be understandable beyond visual color changes. | MEDIUM | Add visible labels, focus treatment, and programmatically recognizable status messages. |
| Guided Hardware/Install readiness | Installer needs active vs pending map clarity, reboot warning, and DS18B20 pull-up guidance. | MEDIUM | Keep safe dropdown enums and pending reboot contract unchanged. |
| Screenshot verification | Visual polish is not validated by firmware build alone. | LOW | Capture desktop and mobile states for affected pages. |

## Differentiators

| Feature | Value | Complexity | Notes |
|---------|-------|------------|-------|
| Owner-first operational confidence | Daily dashboard clearly separates "what is happening now" from settings and diagnostics. | MEDIUM | Aligns with local appliance use rather than generic admin UI. |
| Installer-safe wiring confidence | Hardware page explains the external `4.7 kOhm` DS18B20 pull-up before the next hardware cycle. | LOW | Documentation and UI guidance only. |
| Stable-baseline discipline | UI quality improves without rewriting relay/timer/cooling state machines. | LOW | Explicit roadmap constraint and review gate. |

## Anti-Features

| Feature | Why Requested | Why Problematic | Alternative |
|---------|---------------|-----------------|-------------|
| Dashboard charts and history | Looks more advanced. | Adds scope, payload, rendering work, and storage questions without improving immediate control. | Clear current-state presentation. |
| Large UI framework migration | Promises reusable components. | Adds tooling and flash cost and risks offline behavior. | Focused shared CSS and renderer cleanup. |
| New control modes during polish | Seems convenient while editing UI. | Risks stable hardware behavior and expands hardware validation burden. | Preserve v1.1 behavior. |
| Treat scanner bookkeeping rows as firmware work | Rows remain visible in state. | Mis-scopes completed quick tasks as missing features. | Keep them documented as bookkeeping debt only. |

## Dependencies

```text
Visual hierarchy
    -> requires existing runtime state inventory
    -> guides loading/error/empty presentation

Responsive layout
    -> requires screenshot verification at desktop and mobile widths

Hardware readiness guidance
    -> requires DS18B20 pull-up documentation
    -> preserves active/pending GPIO and reboot safety flow
```

## Priority

| Feature | Priority |
|---------|----------|
| Dashboard hierarchy and operator confidence | P1 |
| Hardware/Install setup guidance and pull-up documentation | P1 |
| Responsive, accessible, loading/error/empty states | P1 |
| Screenshot verification and ESP32 build regression | P1 |
| Event logging, export, charts, and analytics | Deferred |

## Sources

- Local UI shape note: `.planning/notes/owner-installer-ui-shape.md`
- W3C WAI Understanding Status Messages: https://www.w3.org/WAI/WCAG22/Understanding/status-messages
- W3C WAI Understanding Labels or Instructions: https://www.w3.org/WAI/WCAG22/Understanding/labels-or-instructions
- W3C WAI Understanding Target Size Minimum: https://www.w3.org/WAI/WCAG22/Understanding/target-size-minimum

---
*Feature research for: embedded offline ESP32 owner dashboard UI polish*
*Researched: 2026-06-02*
