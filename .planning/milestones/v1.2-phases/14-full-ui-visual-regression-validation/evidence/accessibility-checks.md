# Accessibility Checks

Date: 2026-06-03

## Source Audit

Commands:

```powershell
rg -n 'aria-live|role="alert"|focus-visible|disabled|network-item|aria-label|aria-expanded|tabindex|checkbox|btn-sm|scan' main\static
rg -n '<h1|<h2|<label|<input|<button|role=|aria-' main\static -g '*.html'
```

| Area | Status | Label | Evidence |
|------|--------|-------|----------|
| Login labels | pass | source-inspected | `login.html` uses labels for username/password and `role="alert"` for errors. |
| App shell mobile button | pass in source, fail on stale device | source-inspected/device-backed | Source has `aria-label="Open Navigation"` and `aria-expanded`; device served stale CSS/JS and drawer stayed offscreen. |
| Focus visibility | pass | source-inspected | `style.css` defines focus-visible treatment for links, buttons, inputs, selects, textareas, and tabindex elements. |
| Dynamic status | pass | source-inspected | Dashboard action state spans use `aria-live="polite"`. Wi-Fi network list uses `aria-live="polite"`. |
| Error/alert regions | pass | source-inspected | Login error and Wi-Fi reconnect banner use `role="alert"`. |
| Form labels | pass | source-inspected | Wi-Fi, Hardware/Install, pump timer, and cooling controls have visible labels. |
| Disabled semantics | pass | source-inspected | Runtime buttons, Wi-Fi inputs, connected network rows, and save buttons use `disabled`. |
| Wi-Fi selection keyboard path | pass in source | source-inspected | Generated `.network-item` is a `button`; connected network row becomes disabled. |
| Touch targets | partial | source-inspected/device-backed | Primary buttons and nav rows are about 44px high; hamburger visual glyph is small but hit area is 44px by CSS. Device stale drawer prevents full touch-path proof. |

## Browser Checks

| Page | Viewport | Status | Label | Notes |
|------|----------|--------|-------|-------|
| Login | 1440x900 | pass | device-backed | Form labels visible; login interaction succeeded. |
| Dashboard | 1440x900 | pass | device-backed | Nav and primary actions reachable; no overflow. |
| Hardware/Install | 1440x900 | pass | device-backed | GPIO controls and refresh/back actions reachable; no overflow. |
| Wi-Fi | 1440x900 | pass | device-backed | Scan/connect form visible; no overflow. |
| Status | 1440x900 | pass | device-backed | Refresh and diagnostics visible; no overflow. |
| Required pages | 375x812 | partial | device-backed | No horizontal overflow, but device stale drawer remained offscreen after hamburger click. |
| Keyboard tab walkthrough | not-run | not-run | In-app Browser became unable to resolve `fish-pump.local` before full keyboard traversal could be repeated. |

## A11Y-01 Disposition

A11Y-01 is partial for Phase 14. Current repository source contains the mobile drawer and accessibility hooks needed for narrow mobile usability, but the reachable ESP32 device served stale static assets and therefore could not prove the current repository build on device. A fresh flash of the current build followed by the same 375px drawer test is required before marking A11Y-01 complete.
