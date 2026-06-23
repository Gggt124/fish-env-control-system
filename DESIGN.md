---
name: Fish-Pump-Control-design-system
description: A clean, local-web operational control panel for the ESP32 Fish Pump controller using a Flat-by-Default layout, structured SVG status signals, and a high-contrast semantic color palette.
colors:
  primary: "#2563eb"
  primary-hover: "#1d4ed8"
  primary-container: "#eff6ff"
  on-primary: "#ffffff"
  secondary: "#10b981"
  on-secondary: "#ffffff"
  tertiary: "#f59e0b"
  on-tertiary: "#ffffff"
  error: "#ef4444"
  error-hover: "#dc2626"
  on-error: "#ffffff"
  bg-body: "#f1f5f9"
  surface: "#ffffff"
  surface-dim: "#f1f5f9"
  surface-container: "#f1f5f9"
  surface-container-highest: "#e2e8f0"
  on-surface: "#0f172a"
  on-surface-variant: "#64748b"
  outline: "#64748b"
  outline-variant: "#e2e8f0"
typography:
  font-ui: "system-ui, -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif"
  font-mono: "'Courier New', Consolas, monospace"
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
    padding: "10px 16px"
  button-outline:
    backgroundColor: "transparent"
    textColor: "{colors.primary}"
    rounded: "{rounded.md}"
    padding: "10px 16px"
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
- **Console Blue** (`#2563eb`): Used for primary action buttons, active navigation markers, input focus rings, and positive progress indicators. It draws the operator's eye to active settings and confirmations.

### Secondary
- **Active Green** (`#10b981` / `#006e28`): Indicates active, energized, healthy, and successful operations. Used for active pump relays, healthy sensor connection states, and successful Wi-Fi connections.

### Tertiary
- **Warning Amber** (`#f59e0b` / `#9e3d00`): Represents transient or warning states, such as pending configurations, timer cooldowns, or temporary stabilization periods.

### Neutral
- **Alarm Red** (`#ef4444` / `#ba1a1a`): Indicates critical faults, offline state, emergency stop alerts, or invalid configurations.
- **Muted Slate** (`#64748b`): Used for units of measure, reference text, inactive options, and label descriptions.
- **Canvas Background** (`#f1f5f9`): The low-saturation gray backplane resembling physical plastic console panels.
- **Card Surface** (`#ffffff`): The high-contrast white container color for organizing controls.

### Named Rules
**The Color-as-State Rule.** Saturated color must never be used decoratively. Green, orange, and red must strictly map to active, warning, and alarm status respectively. If a color is visible, it carries functional meaning that the operator must trust.

## 3. Typography

This system uses a single clean font family to handle display, labels, and text data, ensuring readability and consistency across all screen densities.

**Display Font:** System UI stack (system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif)
**Body Font:** System UI stack
**Label/Mono Font:** Courier New, Consolas, monospace (used for MAC addresses, IP addresses, and GPIO configurations)

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
- **Primary:** Console Blue fill, white text, 10px 16px padding. Active state uses `#004493`.
- **Outline:** Transparent background, Console Blue border and text, 10px 16px padding. Hover state uses `rgba(0, 88, 188, 0.05)`.
- **Danger:** Alarm Red fill, white text, 10px 16px padding. Used for disconnect/reset.
- **Scan/Refresh:** Pill-shaped (9999px / `--radius-full`), light blue background, 6px 12px padding.

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
