# Phase 14: Full UI Visual Regression Validation - Research

**Researched:** 2026-06-03  
**Domain:** ESP32 embedded static web UI validation, visual regression evidence, accessibility review, firmware footprint closeout  
**Confidence:** HIGH

<user_constraints>
## User Constraints (from CONTEXT.md)

### Goal

Close v1.2 with repeated full-UI quality review plus screenshot, accessibility, build, footprint, and stable-baseline regression evidence.

### Inputs

- `.planning/ROADMAP.md` Phase 14 success criteria.
- `.planning/phases/11-baseline-ui-audit-and-state-language/11-ESP32-UI-CHECKLIST.md`
- `.planning/phases/13-install-setup-status-ux-and-professional-ui-overhaul/13-UI-REVIEW.md`
- `.planning/quick/260603-88n-pre-phase-14-audit-fixes-for-blockers-an/260603-88n-SUMMARY.md`

### Required Validation Surfaces

- Login
- Dashboard
- Hardware/Install
- Wi-Fi
- Status
- App shell navigation and logout path

### Evidence Required

- Desktop and narrow-mobile screenshot/browser evidence for affected pages.
- Accessibility checks for labels, focus visibility, touch target size, and dynamic state announcements.
- `.\scripts\build.ps1` or equivalent ESP-IDF build gate.
- Embedded asset/firmware footprint check against partition capacity.
- Manual hardware regression notes for relay, timer, cooling, Wi-Fi, and APSTA flows.

### Constraints

- Keep the frontend offline-capable: no CDN, no web fonts, no frontend package dependency.
- Do not claim hardware behavior completion without real ESP32 flash/device evidence.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| A11Y-01 | User can use login, app shell, dashboard, Hardware/Install, Wi-Fi, and Status pages on a narrow mobile viewport without losing access to primary content or actions. | Validate 375px viewport, no horizontal overflow, drawer navigation, focus path, labels, and touch targets. [VERIFIED: .planning/REQUIREMENTS.md] |
| VER-01 | Maintainer can review screenshots of affected UI pages at desktop and narrow mobile viewports before milestone close. | Capture screenshot evidence for login, dashboard, hardware, Wi-Fi, status, and app shell navigation at 1440px and 375px. [VERIFIED: .planning/ROADMAP.md] |
| VER-02 | Maintainer can build valid ESP-IDF firmware after embedded asset changes without introducing unsuitable frontend dependencies or unsafe ESP32 footprint growth. | Run `.\scripts\build.ps1`, dependency grep, static byte sum, binary size and app-slot headroom check. [VERIFIED: scripts/build.ps1; partitions.csv] |
| VER-03 | Maintainer can review the UI with an `impeccable` critique/audit/polish pass and the applicable `ui-ux-pro-max` checklist before milestone close. | Repeat Phase 11 checklist and Phase 13 review criteria against final UI, recording addressed/not-run items. [VERIFIED: 11-ESP32-UI-CHECKLIST.md; 13-UI-REVIEW.md] |
| VER-04 | Maintainer can review representative loading, error, empty, disabled, and pending-reboot screenshots before milestone close. | Use a mock server for simulated states and mark each evidence item as `simulated`, `device-backed`, or `not-run`. [VERIFIED: 14-CONTEXT.md] |
| REG-01 | Owner can continue using the stable v1.1 relay, timer, cooling, Wi-Fi, and APSTA flows without behavioral regression. | Split automated browser regression from manual ESP32 hardware evidence; do not infer relay/timer/cooling behavior from mock screenshots. [VERIFIED: 14-CONTEXT.md; 260603-88n-SUMMARY.md] |
</phase_requirements>

## Summary

Phase 14 should be planned as a validation and evidence phase, not a feature phase. The primary implementation work is to create repeatable browser/mock evidence, run accessibility and offline-footprint checks, build the firmware, then attach manual ESP32 hardware notes for runtime behavior that cannot be proven in a desktop browser. [VERIFIED: .planning/ROADMAP.md; .planning/phases/14-full-ui-visual-regression-validation/14-CONTEXT.md]

The recommended approach is a two-lane validation pass: use an in-app Browser session against a local mock HTTP server for deterministic page screenshots and state simulation, then run a real-device manual checklist for relay, timer, cooling, Wi-Fi, and APSTA evidence. The quick pre-Phase-14 audit already reports browser mock checks for mobile drawer, overflow, Wi-Fi labels, scan rendering, and console cleanliness, but it explicitly leaves hardware flash/manual device testing for Phase 14. [VERIFIED: .planning/quick/260603-88n-pre-phase-14-audit-fixes-for-blockers-an/260603-88n-SUMMARY.md]

**Primary recommendation:** Build Phase 14 as three executor slices: browser/mock visual and accessibility evidence, build/offline/footprint evidence, and manual ESP32 stable-baseline regression evidence. [VERIFIED: codebase read; 14-CONTEXT.md]

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|--------------|----------------|-----------|
| Static page screenshot capture | Browser / Client | Frontend Server mock | The rendered HTML/CSS/JS behavior is the client artifact under review; the mock server only supplies deterministic API responses. [VERIFIED: main/static/*.html; main/static/app.js] |
| Simulated UI states | Frontend Server mock | Browser / Client | Loading/error/empty/pending-reboot states require API response shaping; the browser validates the rendered result. [VERIFIED: main/static/app.js] |
| Accessibility checks | Browser / Client | Static source audit | Focus visibility, labels, live regions, and touch targets are DOM/CSS concerns; source grep finds missing hooks before visual inspection. [VERIFIED: main/static/style.css; main/static/*.html] |
| Offline dependency check | Static assets | Build system | CDN/font/script searches apply to `main/static`; CMake embeds those files into firmware. [VERIFIED: main/CMakeLists.txt] |
| Firmware footprint | Build system | Partition table | `scripts/build.ps1` produces the firmware binary; `partitions.csv` defines two app slots of `0x1F0000` bytes each. [VERIFIED: scripts/build.ps1; partitions.csv] |
| Stable relay/timer/cooling behavior | ESP32 firmware/runtime | Manual hardware evidence | Browser mocks cannot prove GPIO relay output, float switch behavior, DS18B20 sensor behavior, or APSTA persistence on hardware. [VERIFIED: 14-CONTEXT.md; .planning/STATE.md] |

## Project Constraints (from AGENTS.md)

- Use ESP-IDF only for firmware; do not switch to Arduino or PlatformIO. [VERIFIED: AGENTS.md]
- Target is classic ESP32 / ESP32 DevKit V1. [VERIFIED: AGENTS.md]
- Preserve Wi-Fi setup, SoftAP fallback, login/session, captive DNS, and status routes. [VERIFIED: AGENTS.md]
- Frontend must remain plain HTML/CSS/JS with no CDN links, remote fonts, or remote icon assets. [VERIFIED: AGENTS.md; DESIGN.md]
- Before `idf.py` commands on Windows, run code page 65001 and export ESP-IDF; `scripts/build.ps1` automates path resolution, UTF-8 setup, export, version print, optional fullclean, and build. [VERIFIED: AGENTS.md; scripts/build.ps1]
- `EMBED_FILES` symbols strip the `static/` prefix; C symbol checks should use filename-based symbols if linker issues appear. [VERIFIED: AGENTS.md; main/CMakeLists.txt]
- Session cookie is intentionally readable by JavaScript for local-prototype login redirects; do not convert this into a Phase 14 feature change. [VERIFIED: AGENTS.md]
- Do not claim hardware validation without flash/device evidence. [VERIFIED: 14-CONTEXT.md]

## Standard Stack

### Core

| Tool / Library | Version | Purpose | Why Standard |
|----------------|---------|---------|--------------|
| ESP-IDF | 6.0.1 locked in `dependencies.lock`; local export script exists at `C:\esp\v6.0.1\esp-idf\export.ps1` | Firmware build and size validation | Project framework and target are ESP-IDF-only. [VERIFIED: dependencies.lock; env probe] |
| PowerShell | 7.6.2 available | Run Windows build, file-size, grep, and evidence commands | Existing build script is PowerShell. [VERIFIED: env probe; scripts/build.ps1] |
| in-app Browser plugin | Available through Browser skill and `node_repl` discovery | Desktop/mobile screenshots, DOM snapshots, console logs, viewport checks | User requested in-app browser strategy; Browser skill exposes screenshot and Playwright-style APIs for local targets. [VERIFIED: Browser skill read; tool_search node_repl] |
| Node.js | v24.15.0 available | Lightweight local mock HTTP server for static files and API states | Node is present and can serve deterministic JSON without installing frontend packages. [VERIFIED: env probe] |
| ripgrep | Available at Codex local `rg.exe` path | CDN/dependency/static source audits | Fast codebase grep is already available. [VERIFIED: env probe] |

### Supporting

| Tool / Library | Version | Purpose | When to Use |
|----------------|---------|---------|-------------|
| Python | 3.13.13 available | Fallback local server or small evidence scripts | Use only if Node mock server is not practical. [VERIFIED: env probe] |
| ESP-IDF `idf.py size` / `size-components` | Provided by ESP-IDF tools after export | Firmware memory and image-size reporting | Use after `.\scripts\build.ps1` when the binary grows unexpectedly. Espressif documents these commands for size summaries. [CITED: https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/performance/size.html] |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| In-app Browser + local mock | Real ESP32 web server only | Real-device screenshots are valuable but slower and less deterministic for error/empty/fault states. Use real device for behavior evidence, not every simulated visual state. [VERIFIED: 14-CONTEXT.md] |
| Node mock server | Python HTTP server | Python is available, but static-only serving cannot simulate API responses without custom code. [VERIFIED: env probe] |
| Browser screenshots | Static source audit only | Source audit finds hooks but cannot prove layout at 1440px and 375px. [VERIFIED: 11-ESP32-UI-CHECKLIST.md] |

**Installation:** No npm, Python, or frontend package installation is recommended for Phase 14. [VERIFIED: AGENTS.md; DESIGN.md]

```powershell
# Use existing tools only.
.\scripts\build.ps1
rg -n "https?://|cdn|fonts\.googleapis|unpkg|jsdelivr" main/static
```

## Package Legitimacy Audit

No external packages should be installed in this phase. The frontend is embedded plain HTML/CSS/JS, and Phase 14 is a validation/closeout phase. [VERIFIED: AGENTS.md; 14-CONTEXT.md]

| Package | Registry | Age | Downloads | Source Repo | slopcheck | Disposition |
|---------|----------|-----|-----------|-------------|-----------|-------------|
| N/A | N/A | N/A | N/A | N/A | Not run | No package install planned. [VERIFIED: codebase read] |

**Packages removed due to slopcheck [SLOP] verdict:** none. [VERIFIED: no package install planned]  
**Packages flagged as suspicious [SUS]:** none. [VERIFIED: no package install planned]

## Architecture Patterns

### System Architecture Diagram

```text
Phase 14 validation request
  |
  v
Read locked context + prior evidence
  |
  +--> Local mock server serves /login /dashboard /hardware /wifi /status + JSON API fixtures
  |       |
  |       v
  |    In-app Browser at 1440px and 375px
  |       |
  |       +--> screenshots: baseline pages
  |       +--> screenshots: loading/error/empty/disabled/disconnected/connected/sensor-fault/pending-reboot states
  |       +--> DOM checks: labels, focus, live regions, touch target sizes, no horizontal overflow
  |       +--> console checks: no warnings/errors
  |
  +--> Static source and footprint checks
  |       |
  |       +--> rg for CDN/remote assets
  |       +--> sum main/static bytes vs Phase 11 baseline
  |       +--> .\scripts\build.ps1
  |       +--> binary size vs ota_0/ota_1 partition size
  |
  +--> Manual ESP32 hardware regression
          |
          +--> flash/boot/login
          +--> relay/timer/float behavior
          +--> cooling/sensor-fault behavior
          +--> Wi-Fi scan/connect/disconnect and APSTA fallback
          +--> mark evidence device-backed or not-run
```

### Recommended Evidence Structure

```text
.planning/phases/14-full-ui-visual-regression-validation/
├── 14-RESEARCH.md
├── 14-PLAN.md
├── evidence/
│   ├── screenshots/
│   │   ├── desktop/
│   │   ├── mobile-375/
│   │   └── states/
│   ├── browser-checks.md
│   ├── accessibility-checks.md
│   ├── footprint.md
│   └── hardware-regression.md
└── 14-VERIFICATION.md
```

### Pattern 1: Deterministic Mock API Server

**What:** Serve the embedded static files and stub API routes from a local server so browser screenshots can cover states that are hard or unsafe to force on hardware. [VERIFIED: main/static/app.js]

**When to use:** Use for visual states, navigation, form feedback, and status rendering. Do not use as proof of relay GPIO, float switch, DS18B20, or Wi-Fi radio behavior. [VERIFIED: 14-CONTEXT.md]

**Example:**

```javascript
// Source: codebase-verified API surface in main/static/app.js and main/static/*.html
// Keep this as throwaway validation support, not production firmware code.
const routes = {
  "/api/status": { ok: true, sta_connected: false, ap_enabled: true, ap_ip: "192.168.4.1" },
  "/api/wifi/scan": { ok: true, networks: [] },
  "/api/hardware/map": { ok: true, reboot_required: true, pending_valid: true }
};
```

### Pattern 2: Evidence Classification

**What:** Every screenshot and checklist row must be classified as `simulated`, `device-backed`, or `not-run`. [VERIFIED: 11-ESP32-UI-CHECKLIST.md]

**When to use:** Use for all visual states and manual regression items so Phase 14 does not blur mock evidence into hardware proof. [VERIFIED: 14-CONTEXT.md]

**Example:**

```markdown
| State | Page | Evidence | Classification | Notes |
|-------|------|----------|----------------|-------|
| pending reboot | Hardware/Install | screenshots/states/hardware-pending-reboot-mobile.png | simulated | Mock `/api/hardware/map` returned `reboot_required: true`. |
| APSTA fallback | Device | hardware-regression.md | device-backed | ESP32 remained reachable at `192.168.4.1`. |
```

### Pattern 3: Separate Automated Browser Checks From Manual Device Checks

**What:** Browser checks validate layout and UI state rendering; manual hardware checks validate v1.1 runtime behavior. [VERIFIED: 14-CONTEXT.md]

**When to use:** Always for REG-01. Browser mock may show the labels for relay/timer/cooling states, but only manual ESP32 evidence can close relay/timer/cooling/Wi-Fi/APSTA regression. [VERIFIED: 260603-88n-SUMMARY.md]

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Screenshot workflow | A new screenshot framework or dependency | In-app Browser plugin | The Browser skill already supports local targets, screenshots, DOM snapshots, and console logs. [VERIFIED: Browser skill read] |
| Firmware size inspection | Custom map parser | `idf.py size`, `idf.py size-components`, binary file length, and partition CSV arithmetic | Espressif documents size commands and the build already emits a `.bin`. [CITED: https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/performance/size.html] |
| API response simulation | Production firmware changes | Local validation-only mock server | Phase 14 should not add feature behavior to create screenshots. [VERIFIED: 14-CONTEXT.md] |
| Accessibility model | Decorative custom controls | Native controls and existing labels/focus CSS | The app already uses `button`, `input`, `select`, labels, and a global `:focus-visible` rule. [VERIFIED: main/static/*.html; main/static/style.css] |

**Key insight:** The validation surface is mixed: layout and state rendering can be automated; GPIO, sensor, relay, and APSTA behavior must remain manual and device-backed. [VERIFIED: 14-CONTEXT.md]

## Browser And Screenshot Validation Strategy

| Surface | Desktop Evidence | Mobile Evidence | State Evidence | Notes |
|---------|------------------|-----------------|----------------|-------|
| Login | 1440px screenshot, invalid login error, loading button | 375px screenshot, no horizontal overflow, visible submit | loading, error | Login error region has `role="alert"`. [VERIFIED: main/static/login.html] |
| App shell | Sidebar active states and logout link | Drawer open/close, Escape close, overlay, nav links reachable | disabled/not applicable | Drawer button has `aria-label` and `aria-expanded`; quick audit says drawer fixes passed in mock browser. [VERIFIED: main/static/*.html; 260603-88n-SUMMARY.md] |
| Dashboard | Pump, cooling, timer config, quick status | One-column/stacked cards and controls at 375px | loading, error, disabled, sensor fault | Pump/cooling action state spans use `aria-live="polite"`; fault labels are rendered by JS. [VERIFIED: dashboard.html; app.js rg] |
| Hardware/Install | Active/pending summary, DS18B20 pull-up guidance | GPIO selects and reboot checkbox reachable at 375px | loading, error, disabled, pending reboot | Reboot banner is populated by JS when `reboot_required` is true. [VERIFIED: hardware.html; app.js] |
| Wi-Fi | Scan list, connection panel, static IP fields | Long SSID wrap and buttons at 375px | loading, error, empty, disconnected, connected | Network list has `aria-live="polite"`; reconnect banner has `role="alert"`. [VERIFIED: wifi.html] |
| Status | System/memory/uptime/STA/AP/services cards | Diagnostics wrap without overlap | disconnected/connected | Status fields are populated from `/api/status`. [VERIFIED: status.html; app.js] |

Recommended executor method:

1. Start a local mock server from the repo root that serves `main/static` and returns deterministic JSON fixtures for `/api/status`, `/api/pump/config`, `/api/pump/status`, `/api/cooling/config`, `/api/cooling/status`, `/api/hardware/map`, and `/api/wifi/scan`. [VERIFIED: main/static/app.js]
2. Use the in-app Browser at `1440x900` and `375x812` equivalent viewports where available; capture full-page screenshots after each page reaches a stable state. [VERIFIED: Browser skill read]
3. For each screenshot, record page, viewport, state, fixture name, classification, and whether console warnings/errors were present. [VERIFIED: Browser skill read]
4. Keep hardware screenshots optional; hardware manual evidence is required for behavior, while mock screenshots are acceptable for representative visual states when clearly marked. [VERIFIED: 14-CONTEXT.md]

## Accessibility Validation Scope

| Area | Required Check | Source Basis |
|------|----------------|--------------|
| Focus visibility | Tab through login, nav links, buttons, inputs, selects, checkboxes, Wi-Fi network rows; confirm visible `:focus-visible` ring is not obscured. | WCAG includes Focus Visible, Focus Not Obscured, and Focus Appearance criteria. [CITED: https://www.w3.org/TR/WCAG22/] |
| Labels/instructions | Confirm every form control has a visible label or accessible name; include login, pump timers, cooling config, hardware selects, Wi-Fi password/static IP fields. | WCAG includes Headings and Labels, Label in Name, and Labels or Instructions. [CITED: https://www.w3.org/TR/WCAG22/] |
| Live regions/alerts | Confirm dynamic informational updates use polite live regions where appropriate and urgent errors use alert/status patterns. | W3C status-message guidance references alert/live-region techniques for errors and progress. [CITED: https://w3c.github.io/wcag/understanding/status-messages; https://www.w3.org/WAI/WCAG21/Techniques/aria/ARIA19] |
| Mobile 375px | Validate no horizontal overflow, hidden primary actions, clipped long Thai text, clipped SSIDs, clipped IP/MAC strings, or inaccessible drawer controls. | Phase 11 checklist requires 375px review and long-value wrapping. [VERIFIED: 11-ESP32-UI-CHECKLIST.md] |
| Touch targets | Check `.btn`, `.nav-item`, `input`, `select`, hamburger button, Wi-Fi rows, and checkboxes for at least 44px intended target height where practical. | Phase 11 checklist requires 44x44px mobile interactive areas; CSS currently sets global min-height for `.btn`, `.nav-item`, `input`, and `select`. [VERIFIED: 11-ESP32-UI-CHECKLIST.md; main/static/style.css] |
| Reduced motion | Confirm `prefers-reduced-motion: reduce` disables or minimizes spinner/pulse/toast/progress transitions. | CSS includes a reduce-motion media query. [VERIFIED: main/static/style.css] |

Known source hooks:

- `main/static/style.css` contains a top-level `:focus-visible` outline and global min-height primitives for buttons, nav items, inputs, and selects. [VERIFIED: rg main/static/style.css]
- `main/static/wifi.html` has `aria-live="polite"` on `#network-list` and `role="alert"` on `#reconnect-banner`. [VERIFIED: main/static/wifi.html]
- `main/static/login.html` has `role="alert"` on `#login-error`. [VERIFIED: main/static/login.html]
- `main/static/dashboard.html` has `aria-live="polite"` on pump and cooling action state spans. [VERIFIED: main/static/dashboard.html]

## State Evidence Matrix

| State | Primary Page(s) | How To Produce | Evidence Classification | Required Result |
|-------|------------------|----------------|-------------------------|-----------------|
| loading | Login, Dashboard, Wi-Fi, Hardware | Delay mock API responses or trigger scan/connect/save actions | simulated | Button text/spinner/state copy appears without layout shift; controls disabled where appropriate. [VERIFIED: app.js] |
| error | Login, Dashboard, Wi-Fi, Hardware | Mock failed login, API error, scan failure, invalid save | simulated | Error copy appears inline or alert region; no color-only meaning. [VERIFIED: login.html; app.js] |
| empty | Wi-Fi | Mock `/api/wifi/scan` with `networks: []` | simulated | Empty list explains no networks found and scan can be retried. [VERIFIED: app.js] |
| disabled | Dashboard, Wi-Fi, Hardware | Initial state before config/status loads, no selected SSID, unchecked reboot confirmation | simulated | Disabled controls have visible reason or nearby helper, not only opacity. [VERIFIED: wifi.html; hardware.html; app.js] |
| disconnected | Wi-Fi, Status, Dashboard summary | Mock `/api/status` with `sta_connected: false`, `ap_enabled: true` | simulated; device-backed if tested on ESP32 | UI shows disconnected STA while AP fallback remains visible. [VERIFIED: app.js; AGENTS.md] |
| connected | Wi-Fi, Status, Dashboard summary | Mock `/api/status` and scan row with `connected: true`; manual STA connection on device | simulated + device-backed for REG-01 | Current SSID/IP/RSSI displayed and connected network row is disabled. [VERIFIED: app.js; 260603-88n-SUMMARY.md] |
| sensor fault | Dashboard Cooling | Mock cooling status with `fault: true` or `sensor_state: "fault"` | simulated; device-backed only if real DS18B20 fault is reproduced | Cooling channel shows sensor fault and relay forced-off language. [VERIFIED: app.js] |
| pending reboot | Hardware/Install | Mock hardware map with `reboot_required: true`, `pending_valid: true` | simulated; device-backed if pending map saved on ESP32 | Banner and active/pending distinction are visible at desktop and 375px. [VERIFIED: hardware.html; app.js] |
| APSTA fallback | Wi-Fi, Status, device | Manual ESP32 boot and STA disconnect/connect cycle | device-backed or not-run | AP remains reachable at `192.168.4.1`; do not accept mock-only evidence for this row. [VERIFIED: AGENTS.md; 14-CONTEXT.md] |

## Offline Dependency And Footprint Checks

Current local measurements before the Phase 14 build:

| Metric | Current | Baseline | Delta | Interpretation |
|--------|--------:|---------:|------:|----------------|
| Embedded static assets | 166,718 bytes | 159,436 bytes | +7,282 bytes | Growth exists but is modest; rerun after any final edits. [VERIFIED: `Get-ChildItem main/static`] |
| Existing build binary | 1,122,240 bytes | 1,111,616 bytes | +10,624 bytes | Existing binary still leaves substantial headroom; rerun after `.\scripts\build.ps1`. [VERIFIED: build/fish_pump_relay_timer_control.bin] |
| OTA app slot | 2,031,616 bytes | N/A | N/A | `ota_0` and `ota_1` are each `0x1F0000` bytes. [VERIFIED: partitions.csv] |
| Existing binary headroom | 909,376 bytes | N/A | 44.76% free | Calculated from current binary and slot size; final build should recalculate. [VERIFIED: local calculation] |

Required commands:

```powershell
rg -n "https?://|cdn|fonts\.googleapis|unpkg|jsdelivr" main/static
Get-ChildItem -File -LiteralPath "main\static" | Measure-Object -Property Length -Sum
.\scripts\build.ps1
Get-Item -LiteralPath "build\fish_pump_relay_timer_control.bin" | Select-Object Name,Length
```

If the binary grows unexpectedly, run:

```powershell
idf.py size
idf.py size-components
```

Espressif documents `idf.py size`, `size-components`, and `size-files` as the standard way to measure firmware memory/image-size contributors. [CITED: https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/performance/size.html]

## Stable Baseline Regression Checks

### Automated Browser Checks

| Flow | Automated Browser Coverage | Limit |
|------|----------------------------|-------|
| Login | Load page, invalid login, loading state, successful mock redirect, mobile form fit. [VERIFIED: login.html; app.js] | Does not prove server auth/session behavior on ESP32. |
| Dashboard pump/timer UI | Render running/stopped, Timer 1/Timer 2, phase, countdown, disabled/action states. [VERIFIED: dashboard.html; app.js] | Does not prove GPIO relay output or float input switching. |
| Cooling UI | Render temperature, mode, relay, lockout, test remaining, sensor fault. [VERIFIED: dashboard.html; app.js] | Does not prove DS18B20 reads or cooling relay GPIO output. |
| Wi-Fi setup UI | Render scan loading/error/empty/connected/disconnected, long SSIDs, password/static IP controls. [VERIFIED: wifi.html; app.js] | Does not prove RF scan/connect or APSTA behavior. |
| Hardware/Install UI | Render active/pending maps, reboot banner, DS18B20 pull-up guidance, disabled save until confirmation. [VERIFIED: hardware.html; app.js] | Does not prove NVS save/commit or reboot activation on device. |
| Status UI | Render system/memory/uptime/STA/AP/services fields and mobile wrapping. [VERIFIED: status.html; app.js] | Does not prove live telemetry values. |

### Manual ESP32 Hardware Evidence

| Flow | Required Manual Evidence | Closeout Rule |
|------|--------------------------|---------------|
| Relay/timer/float | Flash current firmware, login, verify Float OFF -> Timer 1 / Relay 1 and Float ON -> Timer 2 / Relay 2 with countdown/relay behavior. [VERIFIED: .planning/STATE.md; 260603-88n-SUMMARY.md] | Mark `device-backed`; otherwise mark `not-run` with reason. |
| Pump Start/Stop/config | Start/stop pump, save timer config, confirm UI status and relay behavior remain coherent. [VERIFIED: main/static/app.js; components/pump_control files present] | Browser-only evidence is insufficient for REG-01. |
| Cooling | Verify cooling temperature/channel display, mode commands, test ON, and safe-off sensor-fault behavior if hardware setup permits. [VERIFIED: components/cooling_control; main/static/app.js] | If DS18B20 fault cannot be safely reproduced, document as simulated plus not-run device fault. |
| Wi-Fi STA | Scan, connect to STA, show connected SSID/IP/RSSI, disconnect/forget. [VERIFIED: AGENTS.md; wifi.html] | Device-backed required for Wi-Fi regression. |
| APSTA fallback | Confirm AP `FishPump-Setup` remains active at `192.168.4.1` during setup and after STA changes. [VERIFIED: AGENTS.md; 260603-88n-SUMMARY.md] | Device-backed required; preserve passed soak baseline. |
| Captive DNS / status | Confirm `http://192.168.4.1` and status page remain reachable in SoftAP mode. [VERIFIED: AGENTS.md] | Device-backed required if claiming captive/SoftAP readiness. |

## Common Pitfalls

### Pitfall 1: Treating Mock Screenshots As Hardware Proof

**What goes wrong:** A simulated dashboard can show relay/timer/cooling states while real GPIO, float, sensor, or APSTA behavior remains untested. [VERIFIED: 14-CONTEXT.md]  
**Why it happens:** The UI can render arbitrary JSON fixtures from a mock server. [VERIFIED: app.js]  
**How to avoid:** Mark every evidence row `simulated`, `device-backed`, or `not-run`; close REG-01 only with ESP32 evidence. [VERIFIED: 11-ESP32-UI-CHECKLIST.md]  
**Warning signs:** Final report says "validated relay behavior" but only includes browser screenshots. [VERIFIED: 14-CONTEXT.md]

### Pitfall 2: Missing State Screenshots Because Normal APIs Are Too Happy

**What goes wrong:** Only default loaded pages are captured, leaving loading/error/empty/disabled/pending-reboot states unreviewed. [VERIFIED: .planning/ROADMAP.md]  
**Why it happens:** Many edge states require failing, delayed, empty, or pending API responses. [VERIFIED: app.js]  
**How to avoid:** Use fixture scenarios per state and page; capture representative state screenshots before manual hardware pass. [VERIFIED: 11-ESP32-UI-CHECKLIST.md]  
**Warning signs:** Evidence folder has page screenshots but no `states/` captures. [VERIFIED: 14-CONTEXT.md]

### Pitfall 3: Running `idf.py` Without ESP-IDF Export

**What goes wrong:** `idf.py` is not found in the current shell, even though ESP-IDF exists locally. [VERIFIED: env probe]  
**Why it happens:** `IDF_PATH` is not set in the current environment and `idf.py` is not globally available before export. [VERIFIED: env probe]  
**How to avoid:** Use `.\scripts\build.ps1`, which resolves `C:\esp\v6.0.1\esp-idf`, sets UTF-8, exports ESP-IDF, and runs `idf.py build`. [VERIFIED: scripts/build.ps1]  
**Warning signs:** `Get-Command idf.py` returns no command before export. [VERIFIED: env probe]

### Pitfall 4: Footprint Check Uses Static Assets Only

**What goes wrong:** Static bytes look safe, but firmware binary or rodata has grown enough to threaten app-slot headroom. [CITED: https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/performance/size.html]  
**Why it happens:** Embedded files contribute to final image size through the build, not just filesystem size. [VERIFIED: main/CMakeLists.txt]  
**How to avoid:** Check both static byte sum and `build/fish_pump_relay_timer_control.bin` against `0x1F0000` app slot after the final build. [VERIFIED: partitions.csv]  
**Warning signs:** Static delta is recorded but no binary/headroom line exists in final evidence. [VERIFIED: 11-ESP32-UI-CHECKLIST.md]

### Pitfall 5: Accessibility Audit Stops At Source Grep

**What goes wrong:** CSS and labels exist, but focus is visually obscured, drawer controls are unreachable, or long text overlaps at 375px. [VERIFIED: 11-ESP32-UI-CHECKLIST.md]  
**Why it happens:** Source hooks do not prove actual rendered layout. [VERIFIED: Browser skill read]  
**How to avoid:** Combine source grep, DOM checks, keyboard traversal, and screenshots at 375px. [CITED: https://www.w3.org/TR/WCAG22/]  
**Warning signs:** Final report says focus styles exist but includes no keyboard/focus evidence. [VERIFIED: 11-ESP32-UI-CHECKLIST.md]

## Code Examples

### CDN And Static Footprint Audit

```powershell
# Source: Phase 11 checklist and codebase measurement pattern.
rg -n "https?://|cdn|fonts\.googleapis|unpkg|jsdelivr" main/static
Get-ChildItem -File -LiteralPath "main\static" |
    Measure-Object -Property Length -Sum
```

### Binary Headroom Calculation

```powershell
# Source: partitions.csv uses 0x1F0000 app slots.
$slot = [Convert]::ToInt32("1F0000", 16)
$bin = (Get-Item -LiteralPath "build\fish_pump_relay_timer_control.bin").Length
[pscustomobject]@{
    AppSlotBytes = $slot
    BinBytes = $bin
    HeadroomBytes = $slot - $bin
    HeadroomPct = [math]::Round((($slot - $bin) / $slot) * 100, 2)
}
```

### Evidence Row Template

```markdown
| Page | Viewport | State | Evidence | Classification | Result |
|------|----------|-------|----------|----------------|--------|
| Wi-Fi | 375px | empty scan | evidence/screenshots/states/wifi-empty-mobile.png | simulated | PASS |
```

### Browser Check Pseudocode

```javascript
// Source: Browser skill API supports local navigation, screenshots, DOM snapshots, and console logs.
await tab.goto("http://127.0.0.1:PORT/wifi");
// Set 375px viewport using the available Browser/Playwright capability if exposed in the session.
const snapshot = await tab.playwright.domSnapshot();
const image = await tab.screenshot({ fullPage: true });
const logs = await tab.dev.logs({ levels: ["warn", "error"], limit: 50 });
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Static source audit only | Source audit plus browser screenshots and DOM/console checks | Phase 14 closeout requirement | Better evidence for responsive and dynamic states. [VERIFIED: .planning/ROADMAP.md] |
| Browser mock evidence treated broadly | Explicit `simulated` / `device-backed` / `not-run` classification | Phase 11 checklist and Phase 14 context | Prevents overclaiming hardware behavior. [VERIFIED: 11-ESP32-UI-CHECKLIST.md; 14-CONTEXT.md] |
| Asset size only | Static bytes plus firmware binary and partition headroom | Phase 14 footprint requirement | Embedded UI growth is checked against the actual OTA app slot. [VERIFIED: partitions.csv; scripts/build.ps1] |

**Deprecated/outdated:**

- Treating mock UI as proof of relay/timer/cooling runtime behavior is invalid for this phase. [VERIFIED: 14-CONTEXT.md]
- Adding frontend packages, CDN links, remote fonts, or remote icons is out of scope and conflicts with offline embedded operation. [VERIFIED: AGENTS.md; DESIGN.md]
- Final validation without manual ESP32 flash/device notes cannot close REG-01. [VERIFIED: 14-CONTEXT.md]

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | A local mock server can be created without installing packages by using Node.js built-ins. [ASSUMED] | Standard Stack / Browser Strategy | Executor may need to use Python built-ins instead, but validation strategy remains the same. |
| A2 | The in-app Browser session will expose practical viewport control for 1440px and 375px checks. [ASSUMED] | Browser Strategy | If viewport control is unavailable, executor must use available screenshots plus DOM overflow checks and record limitation. |

## Open Questions (RESOLVED)

1. **Will hardware be available during Phase 14 execution?** — RESOLVED
   - What we know: Phase 14 context requires manual hardware regression notes and forbids hardware claims without real ESP32 evidence. [VERIFIED: 14-CONTEXT.md]
   - Decision: Plan 04 must treat hardware access as a manual `checkpoint:human-action` / precondition. If the executor cannot flash or interact with the board, REG-01 rows must be recorded as `not-run` with reason, and hardware completion must not be claimed. [RESOLVED: 14-04-PLAN.md]
   - Execution rule: `device-backed` is required for relay, timer, float, cooling, Wi-Fi STA, and APSTA hardware pass rows.

2. **Should final evidence include real-device screenshots or only manual notes for hardware?** — RESOLVED
   - What we know: Desktop/mobile screenshot evidence is required; hardware behavior requires device evidence. [VERIFIED: .planning/ROADMAP.md; 14-CONTEXT.md]
   - Decision: Simulated browser screenshots are sufficient for deterministic visual and state evidence when labeled `simulated`. Real ESP32 screenshots are preferred when practical but not required for every visual state. Hardware behavior evidence may be manual notes, photos, screenshots, or serial/build context, but pass rows for REG-01 must be `device-backed`. [RESOLVED: 14-01-PLAN.md; 14-04-PLAN.md]
   - Execution rule: Real-device screenshots may supplement hardware notes; they do not replace manual relay/timer/cooling/Wi-Fi/APSTA checks.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|-------------|-----------|---------|----------|
| PowerShell | Build and evidence commands | Yes | 7.6.2 | Windows PowerShell with same commands, if needed. [VERIFIED: env probe] |
| `rg` | CDN/source audit | Yes | Codex local binary | PowerShell `Select-String`, slower. [VERIFIED: env probe] |
| Node.js | Mock server | Yes | v24.15.0 | Python custom HTTP server. [VERIFIED: env probe] |
| Python | Mock server fallback / ESP-IDF tooling | Yes | 3.13.13 | Node.js for mock server. [VERIFIED: env probe] |
| ESP-IDF export script | Firmware build | Yes | v6.0.1 path exists | Set `IDF_PATH`/`IDF_TOOLS_PATH` manually if script resolution fails. [VERIFIED: env probe; scripts/build.ps1] |
| `idf.py` before export | Direct build command | No | N/A | Use `.\scripts\build.ps1`, which exports ESP-IDF first. [VERIFIED: env probe; scripts/build.ps1] |
| In-app Browser plugin | Screenshots and DOM checks | Yes | Browser skill available; `node_repl` discovered | If unavailable during execution, record blocker and use static audit plus manual screenshots. [VERIFIED: Browser skill read; tool_search] |
| ESP32 device / COM port | REG-01 hardware validation | Unknown | N/A | Add human/device checkpoint; do not mark hardware behavior complete without evidence. [VERIFIED: 14-CONTEXT.md] |

**Missing dependencies with no fallback:**

- Real ESP32 hardware access is required to close REG-01 as `device-backed`; if unavailable, final evidence must mark hardware rows `not-run`. [VERIFIED: 14-CONTEXT.md]

**Missing dependencies with fallback:**

- `idf.py` is not globally available before export, but `.\scripts\build.ps1` resolves and exports ESP-IDF. [VERIFIED: env probe; scripts/build.ps1]

## Validation Architecture

### Test Framework

| Property | Value |
|----------|-------|
| Framework | No unit/e2e framework detected; validation is browser/manual evidence plus ESP-IDF build. [VERIFIED: rg --files] |
| Config file | none detected for Jest/Vitest/Playwright. [VERIFIED: rg --files] |
| Quick run command | `rg -n "https?://|cdn|fonts\.googleapis|unpkg|jsdelivr" main/static` and mock browser smoke where available. [VERIFIED: 11-ESP32-UI-CHECKLIST.md] |
| Full suite command | `.\scripts\build.ps1` plus Phase 14 evidence checklist. [VERIFIED: scripts/build.ps1; 14-CONTEXT.md] |

### Phase Requirements To Test Map

| Req ID | Behavior | Test Type | Automated Command / Evidence | File Exists? |
|--------|----------|-----------|------------------------------|--------------|
| A11Y-01 | Narrow mobile page usability | Browser visual/accessibility | In-app Browser screenshots and DOM checks at 375px | No automated test file; Wave 0 evidence doc needed. [VERIFIED: 14-CONTEXT.md] |
| VER-01 | Desktop/mobile screenshots | Browser visual | In-app Browser screenshots for each page at 1440px and 375px | No automated test file; evidence folder needed. [VERIFIED: .planning/ROADMAP.md] |
| VER-02 | Build/offline/footprint | Build/static audit | `.\scripts\build.ps1`, static byte sum, CDN grep, binary headroom | Build script exists. [VERIFIED: scripts/build.ps1] |
| VER-03 | Closing UI critique/checklist | Manual/review | Repeat Phase 11 checklist and Phase 13 review criteria | Checklist/review files exist. [VERIFIED: required reads] |
| VER-04 | State screenshots | Browser visual | Mock server fixtures and screenshots for required states | No automated test file; evidence folder needed. [VERIFIED: 11-ESP32-UI-CHECKLIST.md] |
| REG-01 | Stable relay/timer/cooling/Wi-Fi/APSTA | Manual hardware | Flash/device testing notes and optional real-device screenshots | Hardware availability unknown. [VERIFIED: 14-CONTEXT.md] |

### Sampling Rate

- **Per task commit:** Run CDN grep and targeted browser smoke for touched/validated page. [VERIFIED: 11-ESP32-UI-CHECKLIST.md]
- **Per wave merge:** Run full mock screenshot set for affected surfaces and `.\scripts\build.ps1`. [VERIFIED: 14-CONTEXT.md]
- **Phase gate:** `.\scripts\build.ps1` green, footprint recorded, all required screenshot/state rows marked `PASS`, `addressed`, or `not-run with reason`, and REG-01 hardware rows classified. [VERIFIED: .planning/ROADMAP.md]

### Wave 0 Gaps

- [ ] `evidence/browser-checks.md` - records URLs, viewport, screenshot file, console logs, overflow/focus checks. [VERIFIED: no existing phase evidence files]
- [ ] `evidence/accessibility-checks.md` - records focus, labels, live regions/alerts, touch targets, 375px results. [VERIFIED: 14-CONTEXT.md]
- [ ] `evidence/footprint.md` - records CDN grep, static bytes, build result, binary size, partition headroom. [VERIFIED: 11-ESP32-UI-CHECKLIST.md]
- [ ] `evidence/hardware-regression.md` - records manual ESP32 relay/timer/cooling/Wi-Fi/APSTA results or not-run blockers. [VERIFIED: 14-CONTEXT.md]

## Security Domain

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---------------|---------|------------------|
| V2 Authentication | Yes | Validate login UI and existing server auth behavior manually; no auth redesign in Phase 14. [VERIFIED: AGENTS.md] |
| V3 Session Management | Yes | Preserve current JavaScript-readable local prototype session cookie decision; do not alter as validation scope. [VERIFIED: AGENTS.md] |
| V4 Access Control | Yes | Manual/browser smoke should confirm protected pages redirect without session where practical. [VERIFIED: AGENTS.md; app.js] |
| V5 Input Validation | Yes | Validate UI error states for login, timer durations, cooling config, Wi-Fi/static IP, and hardware GPIO map. [VERIFIED: app.js] |
| V6 Cryptography | No new scope | No HTTPS, crypto, or secret handling change planned in this validation phase. [VERIFIED: REQUIREMENTS.md Out of Scope] |

### Known Threat Patterns for This Stack

| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| Remote asset dependency in SoftAP UI | Availability / Information disclosure | CDN grep and no external frontend dependencies. [VERIFIED: AGENTS.md; 11-ESP32-UI-CHECKLIST.md] |
| Cross-origin POST abuse | Tampering | Existing server checks Origin/Referer for local AP/STA/mDNS hosts; Phase 14 should regression-smoke API flows but not redesign CSRF. [VERIFIED: AGENTS.md] |
| Overclaiming unauthenticated access safety from browser mock | Spoofing / Elevation | Verify redirect/auth behavior against device or firmware server where practical; mark mock-only auth checks. [VERIFIED: AGENTS.md; 14-CONTEXT.md] |
| Sensitive screenshots | Information disclosure | Privacy-review screenshots before final artifact; avoid leaking real SSID/password/MAC if not needed. [ASSUMED] |

## Recommended Plan Slicing For GSD Executor

| Slice | Scope | Outputs | Risk |
|-------|-------|---------|------|
| 1. Evidence harness and browser matrix | Create validation-only mock server approach, run in-app Browser screenshots for pages and states at desktop/mobile, record console/overflow/focus observations. | `evidence/screenshots/**`, `browser-checks.md`, `accessibility-checks.md` | Medium: viewport/tooling limitations may require not-run notes. [VERIFIED: Browser skill read] |
| 2. Offline/build/footprint gate | Run CDN grep, static byte sum, `.\scripts\build.ps1`, binary size/headroom calculation, optional `idf.py size` if growth is concerning. | `footprint.md` and build output summary | Low: build script already passed in quick task, but must rerun after final state. [VERIFIED: 260603-88n-SUMMARY.md] |
| 3. Manual ESP32 regression | Flash/current firmware if needed; test login, dashboard, pump relay/timer/float, cooling, Wi-Fi STA, APSTA fallback, status/captive reachability. | `hardware-regression.md` with `device-backed` or `not-run` classification | High: requires physical board/access and cannot be replaced by mock evidence. [VERIFIED: 14-CONTEXT.md] |
| 4. Closeout review and requirement trace | Repeat Phase 11 checklist + Phase 13 review criteria, map evidence to A11Y-01/VER-01..04/REG-01, write verification. | `14-VERIFICATION.md`, updated requirement status if workflow expects it | Medium: avoid claiming unresolved hardware rows as complete. [VERIFIED: .planning/ROADMAP.md] |

## Sources

### Primary (HIGH confidence)

- `.planning/ROADMAP.md` - Phase 14 goal, requirements, success criteria, milestone context. [VERIFIED: codebase read]
- `.planning/REQUIREMENTS.md` - A11Y-01, VER-01..04, REG-01 definitions and out-of-scope boundaries. [VERIFIED: codebase read]
- `.planning/STATE.md` - current Phase 14 position and hardware follow-up context. [VERIFIED: codebase read]
- `.planning/phases/14-full-ui-visual-regression-validation/14-CONTEXT.md` - validation surfaces, evidence requirements, constraints. [VERIFIED: codebase read]
- `.planning/phases/11-baseline-ui-audit-and-state-language/11-ESP32-UI-CHECKLIST.md` - repeated Phase 14 checklist, asset/binary baselines, state labels. [VERIFIED: codebase read]
- `.planning/phases/13-install-setup-status-ux-and-professional-ui-overhaul/13-UI-REVIEW.md` - post-Phase-13 review status and resolved UI issues. [VERIFIED: codebase read]
- `.planning/quick/260603-88n-pre-phase-14-audit-fixes-for-blockers-an/260603-88n-SUMMARY.md` - pre-Phase-14 fixes and remaining hardware caveat. [VERIFIED: codebase read]
- `main/static/*.html`, `main/static/style.css`, `main/static/app.js` - current UI routes, state hooks, accessibility hooks, responsive CSS. [VERIFIED: codebase read]
- `scripts/build.ps1`, `partitions.csv`, `main/CMakeLists.txt` - build, app slots, embedded assets. [VERIFIED: codebase read]
- Browser plugin skill `control-in-app-browser` - local Browser/screenshot/DOM/log capability. [VERIFIED: local skill read]

### Primary Documentation (HIGH confidence)

- W3C WCAG 2.2 - focus, labels, target size, status messages criteria list. https://www.w3.org/TR/WCAG22/ [CITED: official W3C]
- W3C Understanding SC 4.1.3 Status Messages - status/progress/error messaging techniques. https://w3c.github.io/wcag/understanding/status-messages [CITED: official W3C]
- W3C ARIA19 - role alert/live-region error notification technique. https://www.w3.org/WAI/WCAG21/Techniques/aria/ARIA19 [CITED: official W3C]
- Espressif ESP-IDF Partition Tables - custom CSV and app/OTA partition concepts. https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/partition-tables.html [CITED: official Espressif]
- Espressif ESP-IDF Binary Size guide - `idf.py size`, `size-components`, `size-files`. https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/performance/size.html [CITED: official Espressif]

### Secondary (MEDIUM confidence)

- None needed; research is primarily codebase and official documentation backed. [VERIFIED: research process]

### Tertiary (LOW confidence)

- None used as authoritative support. [VERIFIED: research process]

## Metadata

**Confidence breakdown:**

- Standard stack: HIGH - verified from codebase, environment probes, Browser skill, and ESP-IDF lockfile. [VERIFIED: dependencies.lock; env probe]
- Architecture: HIGH - Phase 14 is validation-only and the evidence split is explicitly constrained by CONTEXT.md. [VERIFIED: 14-CONTEXT.md]
- Pitfalls: HIGH - derived from Phase 11 checklist, Phase 14 context, quick audit caveat, and current code hooks. [VERIFIED: required reads]
- Accessibility scope: HIGH - mapped to W3C criteria plus local CSS/HTML hooks. [CITED: W3C; VERIFIED: main/static]
- Hardware regression scope: HIGH - device-backed evidence requirement is explicit. [VERIFIED: 14-CONTEXT.md]

**Research date:** 2026-06-03  
**Valid until:** 2026-07-03 for codebase validation approach; recheck environment/tool availability at execution time. [ASSUMED]
