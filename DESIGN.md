---
name: Fish-Pump-Control-design-system
description: A clean, local-web operational control panel for the ESP32 Fish Pump controller using a Flat-by-Default layout, structured SVG status signals, and a high-contrast semantic color palette.
colors:
  primary: "#0097a7"
  primary-hover: "#007b85"
  primary-container: "rgba(0, 151, 167, 0.10)"
  on-primary: "#ffffff"
  secondary: "#0e9a5c"
  on-secondary: "#ffffff"
  tertiary: "#c47d00"
  on-tertiary: "#ffffff"
  error: "#c53030"
  error-hover: "#b71c1c"
  on-error: "#ffffff"
  bg-body: "#f0f2f7"
  surface: "#ffffff"
  surface-dim: "#f0f2f7"
  surface-container: "#f0f2f7"
  surface-container-highest: "#d1d8e4"
  on-surface: "#0d1320"
  on-surface-variant: "#4a5568"
  outline: "#d1d8e4"
  outline-variant: "#e8ecf3"
typography:
  font-ui: "'IBM Plex Sans Thai', system-ui, -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif"
  font-mono: "'JetBrains Mono', 'Courier New', Consolas, monospace"
  display:
    fontFamily: "{typography.font-ui}"
    fontSize: "1.953rem"
    fontWeight: 700
    lineHeight: "2.5rem"
  headline:
    fontFamily: "{typography.font-ui}"
    fontSize: "1.563rem"
    fontWeight: 700
    lineHeight: "2rem"
  title:
    fontFamily: "{typography.font-ui}"
    fontSize: "1.25rem"
    fontWeight: 600
    lineHeight: "1.75rem"
  body:
    fontFamily: "{typography.font-ui}"
    fontSize: "1rem"
    fontWeight: 400
    lineHeight: "1.5rem"
  label:
    fontFamily: "{typography.font-ui}"
    fontSize: "0.75rem"
    fontWeight: 600
    lineHeight: "1rem"
    letterSpacing: "0.05em"
    textTransform: "uppercase"
rounded:
  sm: "4px"
  md: "8px"
  lg: "12px"
  full: "9999px"
spacing:
  xs: "4px"
  base: "8px"
  sm: "12px"
  md: "16px"
  lg: "24px"
  xl: "32px"
components:
  button-primary:
    backgroundColor: "{colors.primary}"
    textColor: "{colors.on-primary}"
    rounded: "{rounded.md}"
    padding: "12px 16px"
    boxShadow: "inset 0 1px 0 rgba(255, 255, 255, 0.15), 0 2px 4px rgba(0, 0, 0, 0.1)"
  button-outline:
    backgroundColor: "transparent"
    textColor: "{colors.primary}"
    rounded: "{rounded.md}"
    padding: "12px 16px"
  card:
    backgroundColor: "{colors.surface}"
    rounded: "{rounded.lg}"
    padding: "{spacing.lg}"
---

# Design System: Fish Pump Control

## 1. Overview

**Creative North Star: "The Clear Console"**

The Fish Pump Control design system is engineered to provide local operators with an immediate, unambiguous understanding of the device's state. It rejects SaaS dashboards, complex decorative gradients, and marketing fluff in favor of a clean, industrial hardware-style control panel. The interface is optimized to perform reliably in offline SoftAP environments, utilizing only embedded static assets, system fonts, and vector graphics.

A primary principle of "The Clear Console" is the **SVG & Text-Reduction Directive**. Rather than describing status and transitions in verbose paragraphs, the interface prioritizes structured, vector-based status signals and high-contrast semantic colors to communicate operational health instantly. 

### Key Characteristics:
- **SVG-First State Signaling**: Rely on icon symbols and status dots for quick status scans.
- **Flat-by-Default Visuals**: Depth is suggested through color steps (tonal layering) rather than drop shadows.
- **Embedded-Friendly Weight**: Zero reliance on CDNs, external fonts, or online asset packages.
- **Dual-Language Context**: Interface copy uses Thai for user action controls and English for precise technical nouns.

## 2. Colors

Colors in this system represent physical appliance states and command structures. We use an **Operational Semantics** approach to define color roles, mapping them to standard hardware console alerts.

### Primary
- **Teal** (`#0097a7` Light, `#00c8a0` Dark): Used for primary action buttons, active navigation markers, input focus rings, and positive progress indicators. It draws the operator's eye to active settings and confirmations.

### Secondary
- **Active Green** (`#0e9a5c` Light, `#22d47a` Dark): Indicates active, energized, healthy, and successful operations. Used for active pump relays, healthy sensor connection states, and successful Wi-Fi connections.

### Tertiary
- **Warning Amber** (`#c47d00` Light, `#f0a830` Dark): Represents transient or warning states, such as pending configurations, timer cooldowns, or temporary stabilization periods.

### Neutral
- **Alarm Red** (`#c53030` Light, `#f06060` Dark): Indicates critical faults, offline state, emergency stop alerts, or invalid configurations.
- **Muted Slate** (`#8fa0b4`): Used for units of measure, reference text, inactive options, and label descriptions.
- **Canvas Background** (`#f0f2f7` Light, `#08090d` Dark): The low-saturation blue-gray backplane resembling physical plastic console panels.
- **Card Surface** (`#ffffff` Light, `#0f1117` Dark): The high-contrast container color for organizing controls.

### Named Rules
**The Color-as-State Rule.** Saturated color must never be used decoratively. Green, orange, and red must strictly map to active, warning, and alarm status respectively. If a color is visible, it carries functional meaning that the operator must trust.

## 3. Typography

This system uses a single clean font family to handle display, labels, and text data, ensuring readability and consistency across all screen densities.

**Display Font:** IBM Plex Sans Thai, system-ui
**Body Font:** IBM Plex Sans Thai, system-ui
**Label/Mono Font:** JetBrains Mono, Courier New, Consolas, monospace (used for MAC addresses, IP addresses, and GPIO configurations)

### Hierarchy
- **Display** (Bold, 31px / 1.953rem, Line-height: 2.5rem): Used for main page headers (e.g. Wi-Fi Setup, Dashboard) to orient the operator.
- **Headline** (Bold, 25px / 1.563rem, Line-height: 2rem): Used for main control section headers.
- **Title** (Semibold, 20px / 1.25rem, Line-height: 1.75rem): Used for status card headers and configuration group labels.
- **Body** (Regular, 16px / 1rem, Line-height: 1.5rem): Used for form inputs, table items, and status indicators. Line length is capped at 65ch.
- **Label** (Semibold, 12px / 0.75rem, Line-height: 1rem, Letter-spacing: 0.05em, Uppercase): Used for metadata labels, column headers, and sub-card categories.

### Named Rules
**The Mono-for-Data Rule.** Every raw hardware metric, IP address, MAC address, and GPIO pin number must be typeset in the monospace font (`--font-mono`). Monospace indicates raw, unfiltered machine state.

## 4. Elevation

In order to maintain a fast, distraction-free environment, the visual depth is flat-by-default, relying on tonal contrast.

Surfaces use a tiered hierarchy of light-gray to white background layers to separate sections. Ambient shadows are restrained and reserved for specific overlay elements.

### Shadow Vocabulary
- **Card Ambient** (`box-shadow: 0px 4px 12px rgba(0, 0, 0, 0.05)`): A very soft, subtle shadow applied to main operational control cards to lift them slightly off the background canvas.
- **Login Box** (`box-shadow: 0px 8px 24px rgba(0, 0, 0, 0.08)`): Used on the centered login card to draw focus during authentication.
- **Overlay/Toast** (`box-shadow: 0px 4px 12px rgba(0, 0, 0, 0.2)`): High-density shadow to separate floating elements (like toasts or modals) from background content.

### Named Rules
**The Flat-Surface Rule.** All background panels, data lists, and sidebars must remain flat (no box shadow). Depth is conveyed by contrasting white surfaces (`#ffffff`) against the light gray canvas (`#f1f5f9`).

## 5. Components

### Buttons
- **Shape:** Gently rounded (8px / `--radius-md`).
- **Primary:** Teal fill, white text, 12px 16px padding, subtle inset shadow for tactility. Active state uses `#006064`.
- **Outline:** Transparent background, Teal border and text, 12px 16px padding. Hover state uses `rgba(0, 151, 167, 0.05)`.
- **Danger:** Alarm Red fill, white text, 12px 16px padding. Used for disconnect/reset.
- **Scan/Refresh:** Pill-shaped (9999px / `--radius-full`), light teal background, 6px 12px padding.
- **Theme Toggle:** Pill switch (14px radius), toggles between Light and Dark mode.

### Inputs / Fields
- **Style:** 1px solid border (`--outline-variant`), white background, 8px radius. Inputs have a left padding of 40px to accommodate absolute-positioned vector icons.
- **Focus:** Border switches to Console Blue with a soft outer ring: `box-shadow: 0 0 0 2px var(--focus-ring-primary)`.
- **Error:** Border switches to Alarm Red with a soft red outer ring: `box-shadow: 0 0 0 2px var(--focus-ring-error)`.

### Cards
- **Corner Style:** Rounded (12px / `--radius-lg`).
- **Background:** Solid white (`#ffffff`).
- **Shadow:** Restrained `Card Ambient` shadow.
- **Internal Spacing:** 24px padding (`--space-6`).

### Navigation Sidebar
- **Desktop Layout:** Fixed left sidebar, width 280px, flat surface, 1px right border (`--outline-variant`).
- **Mobile Navigation:** Sticky bottom navigation bar, height 64px, with absolute z-index (`--z-index-bottom-nav`) and shadow.
- **Active Navigation:** Active item uses Console Blue text and a background highlight.

### Toasts
- **Style:** Compact floating notification at top-right, z-index 1020 (`--z-index-toast`).
- **Color Coding:** Success toast uses Active Green, error uses Alarm Red, info uses Console Blue.

## 6. Do's and Don'ts

### Do
- **Do** map all transparency values (rgba) and layer depths to CSS custom variables (e.g. `--focus-ring-primary`, `--z-index-sticky`).
- **Do** use consolidated SVG symbols via `<svg><use href="#icon-..."></use></svg>` to display system state and reduce inline SVG bloat.
- **Do** use `lang="en"` attributes on inline English technical terms (such as Status, SSID, MAC, IP) to ensure accessibility parser compliance.
- **Do** respect the user's reduced-motion preference: disable standard animations, but allow pulsing loaders (`spinner-pulse`) to pulse opacity so the operator knows the system is loading.
- **Do** use the canonical `.text-muted` class for all secondary gray text.

### Don't
- **Don't** use raw, hardcoded rgba colors or z-index integers in stylesheets.
- **Don't** add CDN stylesheets, external web fonts, or remote illustration packages.
- **Don't** use fluid typography clamps; typography scale must remain stable and predictable.
- **Don't** duplicate utility classes (do not use `.text-on-surface-variant` or `.text-outline` as they are deprecated).
- **Don't** use side-stripe borders (e.g., `border-left: 4px solid var(--primary)`) as decoration.
