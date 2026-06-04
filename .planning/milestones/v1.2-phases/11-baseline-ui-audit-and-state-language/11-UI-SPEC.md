---
phase: 11
slug: baseline-ui-audit-and-state-language
status: approved
reviewed_at: 2026-06-02T17:55:02+07:00
shadcn_initialized: false
preset: none
created: 2026-06-02
---

# Phase 11 - UI Design Contract

> Visual and interaction contract for the baseline audit and the Phase 12-13 implementation briefs. Generated from `PRODUCT.md`, `DESIGN.md`, `11-CONTEXT.md`, and `11-RESEARCH.md`.

---

## Phase Boundary

Phase 11 audits the shipped embedded UI and writes implementation briefs. It does not modify `main/static/*`, redesign the visual direction, change relay/timer/cooling behavior, rewrite Wi-Fi/APSTA behavior, add remote assets, or implement TFT support.

The contract applies to Login, App Shell, Dashboard, Hardware/Install, Wi-Fi, and Status. Evidence must cover desktop `1440px` and narrow mobile `375px` where browser capture is practical.

---

## Design System

| Property | Value |
|----------|-------|
| Tool | Manual embedded design system from `DESIGN.md` |
| Preset | Not applicable |
| Component library | None - plain embedded HTML/CSS/vanilla JS |
| Icon library | None - local Unicode icons already accepted by `DESIGN.md`, with text or accessible-name fallback for controls |
| Font | Local system UI stack: `-apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif` |
| Register | Product UI: calm, clear, trustworthy local appliance control |
| Asset rule | Embedded local assets only: no CDN, remote font, remote icon package, framework migration, large image, or heavy animation |

### Existing Source Of Truth

- Preserve the committed `DESIGN.md` semantic palette, compact radii, restrained elevation, responsive shell, and system fonts.
- Audit current CSS outliers before implementation. Do not add new ad hoc values.
- Keep Thai user-facing guidance concise. Retain technical nouns such as `GPIO`, `STA`, `AP`, `IP`, `Relay`, and `Wi-Fi` where they improve precision.

---

## Visual Hierarchy

| Surface | Primary Focal Point | Secondary Information | Audit Question |
|---------|---------------------|-----------------------|----------------|
| Login | Credential form and `เข้าสู่ระบบ` action | Short purpose and recovery guidance | Can a first-time user identify purpose, required fields, loading, and login failure recovery immediately? |
| App Shell | Current location and route access | Device online context and logout | Does mobile navigation expose Dashboard, Hardware/Install, Wi-Fi, Status, and logout as consistently as desktop? |
| Dashboard | Pump running state, selected timer/relay, phase, and countdown | Float state, cooling channel, configuration, quick diagnostics | Can the owner understand current pump safety within seconds without scanning equal-weight panels? |
| Hardware/Install | Active GPIO map and pending-reboot distinction | Safe dropdown editor and technical pinout | Can an installer see what is active now versus saved for reboot before changing wiring? |
| Wi-Fi | STA connection state and scan/connect recovery | SoftAP fallback context and static-IP options | Are loading, empty, disconnected, connected, error, and disabled states explicit? |
| Status | Scannable system, memory, uptime, STA, AP, and services groups | Technical identifiers | Do narrow widths preserve readable diagnostics without hidden values or horizontal overflow? |

Daily operation must remain visually distinct from configuration and secondary diagnostics. Hardware installation remains an advanced surface rather than a dashboard panel.

---

## Spacing Scale

Declared values for new Phase 12-13 work and Phase 11 audit recommendations:

| Token | Value | Usage |
|-------|-------|-------|
| xs | 4px | Micro gaps, status-dot spacing |
| sm | 8px | Compact inline spacing |
| md | 16px | Default control and content spacing |
| lg | 24px | Card padding and section gaps |
| xl | 32px | Desktop gutters and login-card padding |
| 2xl | 48px | Rare major section separation |
| 3xl | 64px | Mobile topbar height and rare page-level spacing |

Exceptions: none for new spacing values. Existing `12px` spacing in `main/static/style.css` is a baseline audit item: reuse only when a Phase 12-13 brief explicitly preserves it for compatibility or migrate it to this scale.

Touch targets are dimensions, not spacing tokens: interactive mobile areas must be at least `44x44px`.

---

## Typography

Use a constrained four-size core for new work:

| Role | Size | Weight | Line Height |
|------|------|--------|-------------|
| Label | 12px | 600 | 16px |
| Body | 16px | 400 | 24px |
| Heading | 20px | 600 | 28px |
| Display | 32px | 600 | 40px |

Rules:

- Use only weights `400` and `600` for new work.
- Preserve monospace formatting for countdowns, IP addresses, MAC addresses, RSSI, and machine-readable values.
- Treat existing dense `13px`, `14px`, `18px`, `24px`, and countdown-specific numeric styles as baseline audit items. Do not introduce additional sizes without a brief-level justification.
- Use short labels and avoid decorative display typography.

---

## Color

| Role | Value | Usage |
|------|-------|-------|
| Dominant (60%) | `#f2f2f7` / `#faf9fe` | Body background and quiet shell surfaces |
| Secondary (30%) | `#ffffff` / `#f4f3f8` | Cards, sidebar, panels, selected rows, summary tiles |
| Accent (10%) | `#0058bc` | Primary action, active navigation, keyboard focus ring, Wi-Fi setup emphasis, and progress only |
| Success semantic | `#006e28` | Connected, active, completed, and healthy states with text or icon support |
| Warning semantic | `#9e3d00` | Weak signal, unsaved state, and attention states with text support |
| Destructive | `#ba1a1a` | Errors, failed actions, and destructive actions only |

Accent reserved for: primary CTA, active navigation, visible keyboard focus ring, Wi-Fi setup emphasis, and progress. Do not use blue as decorative body text or as a substitute for hierarchy.

Semantic green, orange, and red are justified state colors, not decorative accents. Never communicate state by color alone.

---

## Copywriting Contract

### Voice

- Use short Thai action text for owner and installer guidance.
- Retain precise English technical terms when natural: `GPIO`, `STA`, `AP`, `IP`, `Relay`, `Wi-Fi`, `Start`, `Stop`, `Auto`, `Force OFF`, and `Test ON`.
- Keep state wording independent of web layout so it can be reused by a future small TFT interface.

### Action And Recovery Patterns

| Element | Copy |
|---------|------|
| Primary CTA | `เข้าสู่ระบบ` for Login; action-specific labels such as `บันทึกค่า Timer`, `บันทึกค่า Cooling`, `บันทึก GPIO สำหรับรีบูต`, and `เชื่อมต่อ Wi-Fi` downstream |
| Empty state heading | `ไม่พบเครือข่าย Wi-Fi` |
| Empty state body | `ลองสแกนอีกครั้ง หรือเข้าใกล้เราเตอร์ก่อนเชื่อมต่อ` |
| Error state | `เชื่อมต่อ Wi-Fi ไม่สำเร็จ ตรวจสอบรหัสผ่านแล้วลองอีกครั้ง` |
| Unavailable state | `ยังสั่งงานไม่ได้: รอข้อมูลล่าสุดจากอุปกรณ์` |
| Pending reboot | `บันทึก GPIO แล้ว ต้องรีบูต ESP32 ก่อนค่าชุดใหม่จะทำงาน` |
| Destructive confirmation | `ตัดการเชื่อมต่อและลืมเครือข่าย`: ask for explicit confirmation before clearing saved STA credentials |

Avoid generic labels such as `Submit`, `OK`, `Save`, or `Cancel`. Use verb-plus-object wording when Phase 12-13 changes labels.

---

## Shared State Language

| State | Meaning | Visible Treatment | Assistive Treatment |
|-------|---------|-------------------|---------------------|
| `loading` | A request is in flight. | Short action-specific text, relevant control disabled, small spinner where useful | `aria-live="polite"` status update |
| `success` | The requested action completed. | Contextual confirmation near the initiating area and a short toast where useful | `aria-live="polite"` status update |
| `error` | The action failed or data is unsafe. | Message near the relevant control or status area with a brief recovery instruction | `role="alert"` |
| `empty` | The request succeeded but returned no usable items. | Explain the empty result and provide the next action | `aria-live="polite"` when populated dynamically |
| `disabled` | An action cannot run. | Semantic `disabled`, reduced emphasis, and a reason when it is not obvious | Preserve disabled semantics and adjacent explanatory text |
| `unavailable` | A prerequisite or safe runtime condition is absent. | State the reason, such as stale status, sensor fault, or no selected SSID | `aria-live="polite"` when changed dynamically |
| `pending reboot` | Saved hardware values are not active yet. | Show active and pending GPIO values separately and state that reboot is required | Announce the durable status politely after save |
| `disconnected` | STA is not connected. | Preserve SoftAP fallback context and offer scan/connect recovery | Status text must remain programmatically recognizable |

Three unavailable conditions must remain distinct:

1. `processing`: an action is currently running.
2. `unavailable`: a prerequisite or safe state is absent, with a reason.
3. `pending reboot`: saved values exist but are not active.

---

## Accessibility Contract

- Show a clear `2-4px` blue `:focus-visible` ring on every interactive control: buttons, links, inputs, selects, checkboxes, navigation items, scan controls, and selectable network rows.
- Use visible labels and concise helper text where needed. Placeholder text is not a label.
- Keep mobile interactive areas at least `44x44px`.
- Add `aria-live="polite"` to informational and success status regions.
- Add `role="alert"` to errors requiring immediate action.
- Give icon-only mobile controls an accessible name or visible text fallback.
- Ensure selectable Wi-Fi networks are keyboard operable with recognizable selected and disabled states.
- Never use color as the only indicator; pair state colors with words or meaningful icons.
- Add `prefers-reduced-motion: reduce` handling for nonessential pulse, toast slide-in, spinner, and progress transitions.

---

## Responsive Contract

| Viewport | Required Behavior |
|----------|-------------------|
| `375px` mobile | One-column content where needed, no horizontal overflow, primary actions reachable, `44x44px` touch targets, readable active/pending GPIO state, scannable diagnostics |
| `640px` small tablet | Two-column status and summary layouts only where content remains readable |
| `768px` tablet | Header actions may align horizontally; contextual bars remain readable |
| `1024px+` desktop | Desktop sidebar appears, main content shifts right, status and Wi-Fi grids use wider layouts |
| `1440px` desktop evidence | Capture the full operational hierarchy without introducing decorative whitespace |

Long SSIDs, IP addresses, MAC addresses, firmware versions, and Thai status text must wrap or remain readable without overlapping adjacent controls.

---

## Audit Evidence Contract

Phase 11 must create a Markdown report and an organized screenshot folder. Each finding records:

| Field | Required Content |
|-------|------------------|
| Priority | `Must fix`, `Should fix`, or `Enhancement` |
| Surface | Login, App Shell, Dashboard, Hardware/Install, Wi-Fi, or Status |
| Problem | Specific hierarchy, responsive, accessibility, loading, error, empty, disabled, or pending-reboot gap |
| Evidence | Screenshot path where practical plus source file and selector or line reference |
| User impact | Owner, installer, keyboard user, assistive-technology user, or mobile user consequence |
| Acceptance criteria | Observable condition for Phase 12 or Phase 13 |
| Target phase | Phase 12 or Phase 13 |

Evidence states to capture where safe: default, loading, success, error, empty, disabled, disconnected, connected, sensor fault, and pending reboot. Label screenshots as device-backed, locally simulated, or not-run with reason.

---

## Phase 12 Brief Contract

Phase 12 receives Login, App Shell, and Dashboard findings. The brief must preserve:

- Login purpose, visible labels, loading state, authentication-error recovery, and mobile layout.
- Consistent authenticated navigation across Dashboard, Hardware/Install, Wi-Fi, Status, and logout.
- Pump-first hierarchy: running state, selected timer/relay, phase, countdown, and float state understood within seconds.
- Cooling as a separate operational channel: temperature, mode, relay state, and sensor fault.
- Daily operation controls visually distinct from settings and diagnostics.
- Shared focus, live-region, touch-target, state-language, reduced-motion, local-asset, and footprint rules.

---

## Phase 13 Brief Contract

Phase 13 receives Hardware/Install, Wi-Fi, Status, and applicable shared-state findings. The brief must preserve:

- Explicit active GPIO, pending GPIO, and reboot-required distinctions.
- External `4.7 kOhm` DS18B20 DQ-to-`3.3 V` pull-up guidance.
- Safe GPIO dropdowns and reboot-confirmation behavior.
- Wi-Fi loading, error, empty, disconnected, and connected states without changing APSTA behavior.
- Dashboard and Hardware/Install loading, success, error, disabled, and pending-reboot treatments.
- Scannable system, memory, uptime, STA, AP, and service diagnostics on desktop and mobile.
- Shared focus, live-region, touch-target, state-language, reduced-motion, local-asset, and footprint rules.

---

## ESP32 Footprint Contract

- Keep all frontend assets embedded and local.
- Do not add CDN URLs, remote fonts, remote icon packages, framework bundles, large images, or heavy animation libraries.
- Record baseline embedded frontend assets as `159,436` bytes from `11-RESEARCH.md`.
- Record the latest available firmware binary baseline as `1,111,616` bytes from `build/fish_pump_relay_timer_control.bin`.
- Re-run the full ESP-IDF build and partition-footprint comparison after implementation changes in Phase 14.

---

## Registry Safety

| Registry | Blocks Used | Safety Gate |
|----------|-------------|-------------|
| None | None | Not applicable - embedded plain HTML/CSS/JS uses no component registry |

---

## Checker Sign-Off

- [x] Dimension 1 Copywriting: PASS
- [x] Dimension 2 Visuals: PASS
- [x] Dimension 3 Color: PASS
- [x] Dimension 4 Typography: PASS
- [x] Dimension 5 Spacing: PASS
- [x] Dimension 6 Registry Safety: PASS

**Approval:** approved 2026-06-02
