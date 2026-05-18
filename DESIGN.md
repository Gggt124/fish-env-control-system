---
version: alpha
name: Fish-Pump-Control-design-system
description: A compact ESP32 local-web dashboard design system for Wi-Fi setup, login, and device status. The UI is a light operational control panel with Material-inspired surfaces, soft shadows, small-radius cards, system fonts, Unicode icons, and a restrained blue/green/orange semantic palette that works without internet access or external assets.
colors:
  surface: "#faf9fe"
  surface-dim: "#dad9df"
  surface-bright: "#faf9fe"
  surface-container-lowest: "#ffffff"
  surface-container-low: "#f4f3f8"
  surface-container: "#eeedf3"
  surface-container-high: "#e9e7ed"
  surface-container-highest: "#e3e2e7"
  on-surface: "#1a1b1f"
  on-surface-variant: "#414755"
  inverse-surface: "#2f3034"
  inverse-on-surface: "#f1f0f5"
  outline: "#717786"
  outline-variant: "#c1c6d7"
  surface-tint: "#005bc1"
  primary: "#0058bc"
  on-primary: "#ffffff"
  primary-container: "#0070eb"
  primary-fixed: "#d8e2ff"
  primary-fixed-dim: "#adc6ff"
  secondary: "#006e28"
  on-secondary: "#ffffff"
  secondary-container: "#6ffb85"
  secondary-fixed-dim: "#53e16f"
  tertiary: "#9e3d00"
  on-tertiary: "#ffffff"
  tertiary-container: "#c64f00"
  error: "#ba1a1a"
  on-error: "#ffffff"
  error-container: "#ffdad6"
  bg-body: "#f2f2f7"
typography:
  font-ui: "-apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif"
  font-mono: "'Courier New', Consolas, monospace"
  display-lg:
    fontFamily: "{typography.font-ui}"
    fontSize: 32px
    fontWeight: 700
    lineHeight: 40px
    letterSpacing: -0.02em
  display-md:
    fontFamily: "{typography.font-ui}"
    fontSize: 24px
    fontWeight: 700
    lineHeight: 32px
    letterSpacing: -0.01em
  heading-sm:
    fontFamily: "{typography.font-ui}"
    fontSize: 20px
    fontWeight: 600
    lineHeight: 28px
  heading-xs:
    fontFamily: "{typography.font-ui}"
    fontSize: 16px
    fontWeight: 600
    lineHeight: 24px
  body-md:
    fontFamily: "{typography.font-ui}"
    fontSize: 16px
    fontWeight: 400
    lineHeight: 24px
  body-sm:
    fontFamily: "{typography.font-ui}"
    fontSize: 14px
    fontWeight: 400
    lineHeight: 20px
  label:
    fontFamily: "{typography.font-ui}"
    fontSize: 12px
    fontWeight: 600
    lineHeight: 16px
    letterSpacing: 0.05em
    textTransform: uppercase
  mono:
    fontFamily: "{typography.font-mono}"
    fontSize: 13px
    fontWeight: 400
    lineHeight: 18px
spacing:
  xs: 4px
  base: 8px
  sm: 12px
  md: 16px
  lg: 24px
  xl: 32px
radius:
  sm: 4px
  md: 8px
  lg: 12px
  full: 9999px
shadows:
  card: "0px 4px 12px rgba(0, 0, 0, 0.05)"
  login: "0px 8px 24px rgba(0, 0, 0, 0.1)"
  sm: "0px 1px 3px rgba(0, 0, 0, 0.08)"
components:
  button-primary:
    backgroundColor: "{colors.primary}"
    textColor: "{colors.on-primary}"
    typography: "{typography.body-md}"
    rounded: "{radius.md}"
    padding: "10px 16px"
  button-outline:
    backgroundColor: transparent
    textColor: "{colors.primary}"
    borderColor: "{colors.primary}"
    typography: "{typography.body-md}"
    rounded: "{radius.md}"
    padding: "10px 16px"
  button-danger:
    backgroundColor: "#d32f2f"
    textColor: "#ffffff"
    rounded: "{radius.md}"
    padding: "10px 16px"
  card:
    backgroundColor: "{colors.surface-container-lowest}"
    borderColor: "rgba(255, 255, 255, 0.5)"
    rounded: "{radius.lg}"
    shadow: "{shadows.card}"
    padding: "{spacing.lg}"
  text-input:
    backgroundColor: "{colors.surface-container-lowest}"
    textColor: "{colors.on-surface}"
    borderColor: "{colors.outline-variant}"
    focusBorderColor: "{colors.primary}"
    focusRing: "0 0 0 2px rgba(0, 88, 188, 0.15)"
    rounded: "{radius.md}"
    padding: "10px 12px 10px 40px"
  sidebar:
    width: 280px
    backgroundColor: "{colors.surface-container-lowest}"
    borderColor: "rgba(193, 198, 215, 0.2)"
    shadow: "{shadows.card}"
  mobile-topbar:
    height: 64px
    backgroundColor: "{colors.surface}"
    shadow: "{shadows.sm}"
---

# Fish Pump Control DESIGN.md

## Overview

Fish Pump Control is a local ESP32 setup and operations UI. It is not a marketing site and should not look like one. The design language is quiet, clear, and device-administration oriented: light gray body canvas, white or near-white surfaces, compact status cards, small-radius controls, and direct action buttons.

The UI must work inside an ESP32 SoftAP environment with no internet. Use plain HTML/CSS/JS, system fonts, Unicode symbols, and embedded static assets only. Do not introduce CDN fonts, icon libraries, CSS frameworks, SVG illustration packs, or remote images.

The current product mood is "Connectivity & Control": practical, calm, local-device focused, and easy to scan on a phone while standing near hardware. Data density is moderate. The dashboard should communicate whether the device is reachable, whether AP fallback is active, whether STA is connected, and whether system health looks normal.

## Visual Theme & Atmosphere

- Light operational dashboard, not decorative.
- Surfaces are layered by subtle container tints, small shadows, and thin borders.
- Primary blue means action, navigation, focus, and Wi-Fi setup.
- Secondary green means connected, active, healthy, or completed.
- Tertiary orange means warning or weak signal.
- Error red means failed login, failed scan, failed connect, and destructive status.
- Unicode icons are part of the current voice. They should stay simple and functional.
- Thai UI text is first-class. English labels are acceptable for technical terms like Dashboard, Wi-Fi, STA, AP, RSSI, IP, MAC, and Firmware.

## Colors

### Surfaces

| Token | Hex | Role |
|---|---:|---|
| `surface` | `#faf9fe` | Light page/shell surface and Wi-Fi panels. |
| `bg-body` | `#f2f2f7` | Default app body background behind cards and sections. |
| `surface-container-lowest` | `#ffffff` | Card, login card, input, and sidebar base. |
| `surface-container-low` | `#f4f3f8` | Subtle selected rows, sidebar node panel, summary tiles. |
| `surface-container` | `#eeedf3` | Status banners and AP pills. |
| `surface-container-high` | `#e9e7ed` | Table row separators and progress track variants. |
| `surface-container-highest` | `#e3e2e7` | Stronger border and divider surface. |
| `surface-dim` | `#dad9df` | Dimmed surface utility. |

### Text & Borders

| Token | Hex | Role |
|---|---:|---|
| `on-surface` | `#1a1b1f` | Main text and card values. |
| `on-surface-variant` | `#414755` | Supporting copy and secondary labels. |
| `outline` | `#717786` | Low-emphasis text, metadata, table labels, disabled-ish text. |
| `outline-variant` | `#c1c6d7` | Input borders, light dividers, placeholder text. |
| `inverse-surface` | `#2f3034` | Dark inverse surfaces if needed. |
| `inverse-on-surface` | `#f1f0f5` | Text on inverse surfaces. |

### Semantic Palette

| Token | Hex | Role |
|---|---:|---|
| `primary` | `#0058bc` | Main CTA, nav active state, focus border, progress fill, primary icon color. |
| `primary-container` | `#0070eb` | Primary button hover and stronger blue container. |
| `primary-fixed` | `#d8e2ff` | Login icon background and soft blue emphasis. |
| `primary-fixed-dim` | `#adc6ff` | Login icon border and soft blue edge. |
| `secondary` | `#006e28` | Connected, active, done, strong signal, healthy. |
| `secondary-container` | `#6ffb85` | Bright green container when needed sparingly. |
| `secondary-fixed-dim` | `#53e16f` | AP pulse indicator. |
| `tertiary` | `#9e3d00` | Warnings, weak signal, attention states. |
| `tertiary-container` | `#c64f00` | Strong orange surface if required. |
| `error` | `#ba1a1a` | Form errors and failed operations. |
| `error-container` | `#ffdad6` | Soft error background if an error panel is added. |

## Typography

Use only the system UI stack:

```css
font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
```

Use monospace only for device identifiers, IPs, MACs, RSSI values, and other machine-readable values:

```css
font-family: 'Courier New', Consolas, monospace;
```

| Token | Size | Weight | Line Height | Use |
|---|---:|---:|---:|---|
| `display-lg` | `32px` | `700` | `40px` | Desktop page H1. |
| `display-md` | `24px` | `700` | `32px` | Mobile page H1, login H1, card metric values. |
| `heading-sm` | `20px` | `600` | `28px` | Section card titles and desktop brand name. |
| `heading-xs` | `16px` | `600` | `24px` | Panel titles, table card headings, network SSIDs. |
| `body-md` | `16px` | `400` | `24px` | Inputs and default readable text. |
| `body-sm` | `14px` | `400` | `20px` | Supporting copy, subtitles, nav, card subtext. |
| `label` | `12px` | `600` | `16px` | Uppercase labels, card labels, form labels. |
| `mono` | `13px` | `400` | `18px` | IP, MAC, RSSI, technical values. |

Rules:

- Keep headings compact. Do not use oversized hero typography.
- Labels should be uppercase, 12px, semibold, and slightly tracked.
- Thai copy should be rendered in the same system stack. Avoid decorative Thai fonts.
- Do not use negative letter spacing except the existing `h1` desktop/mobile values.

## Layout Principles

### App Shell

- Desktop width `>= 1024px`: show a fixed left sidebar, `280px` wide.
- Desktop main content: `margin-left: 280px`, `32px` padding.
- Mobile and tablet: hide sidebar and show a sticky `64px` mobile top bar.
- Default page content uses a vertical stack with `24px` gaps.
- Page headers stack on mobile and become horizontal from `768px`.

### Grids

- Status grid: 1 column by default, 2 columns from `640px`, 4 columns from `1024px`.
- Wi-Fi setup grid: 1 column by default, then `8fr 4fr` from `1024px`.
- Summary grid: 2 columns by default, 3 columns from `640px`.
- Keep control panels dense enough for repeated use. Avoid landing-page spacing.

### Login

- Login page uses a full viewport centered card.
- Background is `surface` with a subtle 20px radial dot pattern using `outline-variant`.
- Card width is `100%` with `max-width: 400px`, `32px` padding, `12px` radius.
- Login icon is a 64px square, 8px radius, blue soft container.

## Spacing

Use the existing CSS spacing tokens:

| Token | Value | Use |
|---|---:|---|
| `xs` | `4px` | Micro gaps, status dot text, small margin-top. |
| `base` | `8px` | Button icon gaps, label-to-value gaps. |
| `sm` | `12px` | Sidebar node padding, small buttons, row gaps. |
| `md` | `16px` | Mobile page padding, panel headers, default horizontal control padding. |
| `lg` | `24px` | Card padding, page stack gaps. |
| `xl` | `32px` | Desktop page padding and login card padding. |

Spacing should feel measured and utilitarian. Do not add large hero gaps, decorative whitespace, or marketing-style section breaks.

## Shape & Radius

| Token | Value | Use |
|---|---:|---|
| `radius-sm` | `4px` | Small labels or tight utility elements if needed. |
| `radius-md` | `8px` | Inputs, buttons, nav items, node info, selected SSID display. |
| `radius-lg` | `12px` | Cards, login card, Wi-Fi panels, overlays. |
| `radius-full` | `9999px` | Pills, dots, progress bars, circular Wi-Fi icons. |

Do not make the app overly rounded. Cards stay at `12px`, buttons and inputs at `8px`, and pills only where the shape communicates status.

## Depth & Elevation

| Level | Treatment | Use |
|---|---|---|
| Level 0 | Flat body background `bg-body` | Page canvas. |
| Level 1 | White/near-white surface with subtle border | Cards, panels, sidebar. |
| Level 2 | `0px 1px 3px rgba(0, 0, 0, 0.08)` | Mobile top bar, small raised controls. |
| Level 3 | `0px 4px 12px rgba(0, 0, 0, 0.05)` | Dashboard cards, Wi-Fi panels, sidebar. |
| Level 4 | `0px 8px 24px rgba(0, 0, 0, 0.1)` | Login card. |
| Toast | `0px 4px 12px rgba(0, 0, 0, 0.2)` | Temporary notification above app. |

Elevation must remain soft. Do not add heavy glassmorphism, glowing panels, or dark-mode shadows.

## Components

### Buttons

`btn`:

- Inline-flex, centered, `gap: 8px`.
- Padding `10px 16px`.
- Radius `8px`.
- Font `16px`, weight `600`, line-height `24px`.
- Disabled state uses `opacity: 0.5` and `cursor: not-allowed`.

Variants:

- `btn-primary`: blue fill, white text, small shadow. Hover uses `primary-container`, active uses `#004493`.
- `btn-outline`: transparent fill, primary blue text and border. Hover uses `rgba(0, 88, 188, 0.05)`.
- `btn-danger`: red fill for disconnect/destructive actions. Hover uses `#b71c1c`.
- `btn-sm`: `6px 12px`, 12px semibold text.
- `btn-block`: full width, used on login submit.
- `scan-btn`: pill shape, light blue background, 12px semibold, used only for scan refresh.

### Inputs & Forms

- Form groups stack label and field with `4px` gap and `24px` bottom margin.
- Labels are 12px semibold uppercase.
- Inputs are 16px, `10px 12px 10px 40px`, white background, `outline-variant` border, 8px radius.
- Input icons sit absolutely at `left: 12px`, vertically centered, `18px`, outline color.
- Focus state must use blue border and `0 0 0 2px rgba(0, 88, 188, 0.15)`.
- Checkboxes use `accent-color: primary`.

### Sidebar

- Fixed left shell only on desktop.
- Width `280px`, full viewport height, white surface, soft card shadow.
- Brand title is 20px bold primary blue.
- Node info block uses `surface-container-low`, `12px` padding, 8px radius.
- Nav rows are `10px 16px`, 8px radius, 14px semibold.
- Active nav uses blue text, `surface-container-low` fill, and a `4px` blue border on the right.

### Mobile Top Bar

- Sticky top, height `64px`, `24px` horizontal padding.
- Brand is 20px bold primary blue.
- Icons are Unicode symbols at 20px in primary blue.
- Use this for navigation context on mobile, not for complex controls.

### Cards

- Base card: white background, `24px` padding, 12px radius, soft card shadow.
- Use a subtle white border on dashboard cards.
- Card labels are 12px semibold uppercase with outline color.
- Card values are large and strong: 24px bold on mobile, 20px semibold on desktop where the grid is denser.
- Card subtext is 14px with semantic color classes: `good`, `warn`, `neutral`.
- Cards may contain compact tables, progress bars, and short status lines.

### Section Cards & Summary Tiles

- Section cards use the same white surface and 12px radius.
- Section title is 20px semibold.
- Summary tiles sit inside section cards with `surface-container-low`, `12px` padding, 8px radius.
- Summary labels are 11px uppercase semibold; summary values are 18px bold.

### Info Tables

- Full-width, collapsed borders.
- Rows after the first have a `surface-container-high` top border.
- Labels are 12px semibold outline text, nowrap, left aligned.
- Values are 14px medium, right aligned.
- Technical values use monospace 13px.

### Progress Bar

- 6px height, full-pill radius.
- Track uses `surface-container-high`.
- Fill uses primary blue.
- Animate width changes with `0.3s`.

### Wi-Fi Stepper

- Horizontal, centered, `max-width: 640px`, 32px bottom margin.
- Connector line is 2px `surface-container-highest`.
- Step circle is 32px, circular, 12px semibold.
- Active step uses primary blue.
- Done step uses secondary green.
- Step label is 12px semibold on the body background.

### Network List

- Network panel is a white/light surface with soft shadow and clipped overflow.
- Header is 16px panel title and a scan pill.
- List max height is 400px and scrolls.
- Network item has 16px padding, bottom border, pointer cursor, hover and selected fill `surface-container-low`.
- Left side: circular 40px icon container and SSID/auth/channel text.
- Right side: signal label, dBm value in monospace, and small select button.
- Signal semantics: strong green, good neutral, weak orange.

### Input Panel Overlay

- Disabled Wi-Fi input panel uses `opacity: 0.5` and `pointer-events: none`.
- Overlay hint uses a translucent surface with 1px blur and a centered hint box.
- Hint box uses white/light surface, small shadow, border, 8px radius, 12px semibold text.

### Toasts

- Fixed top-right at `16px`.
- Max width `320px`.
- Padding `12px 16px`, 8px radius, 14px semibold white text.
- Success uses secondary green, error uses error red, info uses primary blue.
- Enter animation slides in from the right over `0.3s`.

## Interaction & State Rules

- Login submit disables the button and swaps text to loading copy.
- Login errors appear inline below the password field in error red.
- Dashboard refresh actions use small buttons and do not change layout.
- Wi-Fi scan button shows the spinner inline and disables itself while scanning.
- Selecting a network removes the input panel disabled state, hides the overlay, marks step 2 active, and highlights the selected network row.
- Connecting marks step 3 active; success marks step 3 done and shows green status text.
- Failed connect returns the stepper to step 2 and shows error red text.
- AP active indicator uses a small pulsing green dot.
- Do not introduce modal dialogs unless the user flow truly requires interruption.

## Responsive Behavior

| Breakpoint | Width | Behavior |
|---|---:|---|
| Mobile | `< 640px` | One-column grids, mobile top bar, sidebar hidden, page padding 16px. |
| Small tablet | `>= 640px` | Status cards become 2 columns; summary grid becomes 3 columns. |
| Tablet | `>= 768px` | Page header becomes horizontal; context bar can appear. |
| Desktop | `>= 1024px` | Sidebar appears, main content shifts right, status cards become 4 columns, Wi-Fi grid becomes 8fr/4fr, page padding 32px. |

Touch targets should remain at least about 40px high. Buttons with icons should not wrap awkwardly. Long SSIDs, IPs, and MAC addresses should be allowed to fit without overlapping adjacent controls.

## Accessibility & Content

- Keep semantic HTML: `main`, `header`, `aside`, `nav`, `table`, `button`, and form labels.
- Focus states must remain visible on every interactive form control.
- Do not rely on color alone for network security or signal status; keep text labels such as auth type, dBm, and status words.
- Preserve Thai copy where it already exists. Use concise Thai for user-facing actions and short English for technical nouns when natural.
- Avoid decorative copy. The app should read as a control interface.

## Do's And Don'ts

### Do

- Use the existing CSS custom properties as the source of truth.
- Keep blue reserved for primary action, navigation, focus, and progress.
- Use green for connected, active, completed, and healthy states.
- Use orange only for warning or weak states.
- Use red only for errors and destructive operations.
- Keep the sidebar, cards, tables, Wi-Fi list, and forms visually consistent.
- Use Unicode icons that already fit the embedded/offline constraint.
- Keep UI dense but readable for phone-based setup.

### Don't

- Do not add CDN dependencies, external fonts, remote icon sets, or remote images.
- Do not create a landing page or marketing hero.
- Do not use dark-mode-first styling unless a full alternate theme is explicitly requested.
- Do not add relay/timer control UI until relay pin map and timing requirements are explicit.
- Do not add oversized cards, decorative gradients, bokeh/orb backgrounds, or illustration-heavy sections.
- Do not turn status cards into nested card stacks.
- Do not use primary blue for long body text.
- Do not remove the SoftAP fallback context from Wi-Fi setup screens.

## Agent Prompt Guide

When implementing UI changes in this project, use this short instruction:

> Build a compact light operational ESP32 dashboard using the Fish Pump Control design system. Use system fonts, no CDN, white/light-gray surfaces, 12px cards, 8px controls, primary blue actions, green healthy states, orange warnings, red errors, Unicode icons, and responsive behavior with mobile topbar plus desktop sidebar.

Quick token reference:

- Page background: `#f2f2f7`
- Main surface: `#ffffff`
- Panel surface: `#faf9fe`
- Primary action: `#0058bc`
- Success/connected: `#006e28`
- Warning/weak: `#9e3d00`
- Error: `#ba1a1a`
- Main text: `#1a1b1f`
- Muted text: `#717786`
- Border: `#c1c6d7`
- Card radius: `12px`
- Control radius: `8px`
- Desktop sidebar: `280px`
