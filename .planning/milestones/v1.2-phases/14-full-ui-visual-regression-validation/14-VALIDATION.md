---
phase: 14
slug: full-ui-visual-regression-validation
status: approved
nyquist_compliant: true
wave_0_complete: true
created: 2026-06-03
---

# Phase 14 - Validation Strategy

> Per-phase validation contract for final v1.2 UI visual regression, accessibility,
> footprint, build, and stable hardware-baseline closeout.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | In-app Browser evidence, source `rg` checks, manual accessibility review, ESP-IDF build, and manual ESP32 hardware regression |
| **Config file** | `main/CMakeLists.txt`, `scripts/build.ps1`, `partitions.csv`, `main/static/` |
| **Quick run command** | `rg -n "https?://|cdn|fonts\.googleapis|unpkg|jsdelivr" main/static`; targeted source checks from each plan |
| **Full suite command** | Plan 01 browser matrix, Plan 02 accessibility/UI closeout, `.\scripts\build.ps1`, binary headroom calculation, and Plan 04 hardware-regression checklist |
| **Estimated runtime** | <10 seconds for source checks, ~1 minute for build after ESP-IDF export, manual-duration for browser screenshots and hardware regression |

---

## Sampling Rate

- **After every evidence task:** Update the relevant evidence markdown with pass/fail/not-run rows and source labels.
- **After browser/state capture:** Review screenshots for privacy, 375px usability, primary-action reachability, console errors, and horizontal overflow.
- **After accessibility/UI review:** Re-check any blocker or warning against source and rendered evidence.
- **After build/footprint gate:** Confirm static dependency search, static bytes, firmware binary size, and app partition headroom are recorded.
- **Before `$gsd-verify-work`:** `14-VERIFICATION.md` must map A11Y-01, VER-01, VER-02, VER-03, VER-04, and REG-01 to concrete evidence.
- **Max feedback latency:** <10 seconds for source checks, ~60 seconds for ESP-IDF build, manual-duration for screenshots and hardware checks.

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command / Evidence | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|------------------------------|-------------|--------|
| 14-01-01 | 01 | 1 | A11Y-01, VER-01, VER-04 | T-14-01, T-14-02 | Validation target is source-labeled and limitations are recorded before screenshots are trusted. | browser/manual evidence | Open `/login`, `/dashboard`, `/hardware`, `/wifi`, `/status` in the in-app Browser or device URL; record target in `evidence/README.md`. | evidence file planned | pending |
| 14-01-02 | 01 | 1 | A11Y-01, VER-01 | T-14-01, T-14-02 | Desktop/mobile screenshots are privacy-reviewed and do not overclaim hardware behavior. | browser/manual evidence | Capture 1440px and 375px screenshots; record console, overflow, drawer, navigation, and primary action reachability in `browser-checks.md`. | evidence file planned | pending |
| 14-01-03 | 01 | 1 | VER-04 | T-14-01, T-14-02 | State screenshots distinguish simulated/device-backed/not-run evidence. | browser/manual evidence | Capture or mark not-run for loading, error, empty, disabled, disconnected, connected, sensor-fault, and pending-reboot states. | evidence file planned | pending |
| 14-02-01 | 02 | 2 | A11Y-01, VER-03 | T-14-03 | Source accessibility hooks exist for labels, focus, live regions, alerts, drawer, and Wi-Fi rows. | source check | `rg -n "aria-live|role=\"alert\"|focus-visible|disabled|network-item|hamburger-btn" main/static` | command only | pending |
| 14-02-02 | 02 | 2 | A11Y-01 | T-14-03 | Rendered keyboard, touch, and 375px behavior is checked instead of inferred only from source. | browser/manual evidence | Browser focus/touch walkthrough recorded in `accessibility-checks.md`. | evidence file planned | pending |
| 14-02-03 | 02 | 2 | VER-03 | T-14-04 | Closing `impeccable` and `ui-ux-pro-max` review catches AI-slop and product-dashboard regressions. | manual review | `rg -n "gradient|background-clip|backdrop-filter|border-radius|focus-visible" main/static/style.css` plus screenshot review. | command only | pending |
| 14-03-01 | 03 | 1 | VER-02 | T-14-05 | Embedded static UI has no unsuitable remote frontend dependency and byte growth is recorded. | source/static check | `rg -n "https?://|cdn|fonts\.googleapis|unpkg|jsdelivr" main/static`; static file byte inventory. | command only | pending |
| 14-03-02 | 03 | 1 | VER-02 | T-14-06 | Firmware builds through ESP-IDF after embedded UI changes. | build | `.\scripts\build.ps1`; `Test-Path build\fish_pump_relay_timer_control.bin` | command only | pending |
| 14-03-03 | 03 | 1 | VER-02 | T-14-06 | Binary size fits the `0x1F0000` OTA app slot with recorded headroom. | file/partition check | Compare `build\fish_pump_relay_timer_control.bin` length to `partitions.csv` app slot size. | command only | pending |
| 14-04-01 | 04 | 3 | REG-01 | T-14-07, T-14-08 | Relay, timer, float, cooling, Wi-Fi, APSTA, login/session, and status rows are device-backed or not-run. | manual hardware | ESP32 manual checklist in `hardware-regression.md`; no simulated row may pass REG-01. | evidence file planned | pending |
| 14-04-02 | 04 | 3 | A11Y-01, VER-01, VER-02, VER-03, VER-04, REG-01 | T-14-07 | Phase verification maps every requirement to concrete evidence and does not overclaim hardware. | documentation check | `rg "A11Y-01|VER-01|VER-02|VER-03|VER-04|REG-01|device-backed|not-run" .planning/phases/14-full-ui-visual-regression-validation/14-VERIFICATION.md` | evidence file planned | pending |
| 14-04-03 | 04 | 3 | A11Y-01, VER-01, VER-02, VER-03, VER-04, REG-01 | T-14-08 | Requirement status updates match evidence instead of milestone optimism. | documentation check | Compare `.planning/REQUIREMENTS.md` checkboxes to `14-VERIFICATION.md` status matrix. | N/A command | pending |

---

## Wave 0 Requirements

Existing command-based infrastructure covers Phase 14 setup.

No package-managed test framework is required because:

- Browser screenshots and state capture are evidence artifacts, not automated app tests.
- Accessibility and UI quality require a mix of source inspection and rendered-page review.
- Firmware build validation is already centralized in `scripts/build.ps1`.
- Hardware regression cannot be automated without physical ESP32/relay/float/cooling access and must stay manual/device-backed.

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Review every page at desktop `1440px` and mobile `375px`. | A11Y-01, VER-01 | Visual hierarchy, wrapping, drawer behavior, and touch reachability require rendered-page inspection. | Use the in-app Browser against a mock target or ESP32 URL. Store screenshots or record not-run reasons. |
| Review loading, error, empty, disabled, disconnected, connected, sensor-fault, and pending-reboot states. | VER-04 | Several states require controlled API fixtures or safe device conditions. | Capture simulated or device-backed evidence and label every row. |
| Perform closing `impeccable` and `ui-ux-pro-max` review. | VER-03 | Final visual and UX quality requires design judgment, not just source grep. | Record findings, severity, evidence, and disposition in `ui-review-closeout.md`. |
| Run relay/timer/float/cooling/Wi-Fi/APSTA regression on ESP32 hardware. | REG-01 | GPIO, relay, float switch, DS18B20/cooling, and APSTA behavior cannot be proven by browser mocks. | Flash or use current firmware on the ESP32 test device; mark unavailable rows `not-run`. |

---

## Validation Sign-Off Criteria

- [ ] Every plan task has a quick verify command or documented manual evidence path.
- [ ] Sampling continuity: no three consecutive tasks lack source/build/browser/manual verification.
- [ ] Evidence labels are enforced: `device-backed`, `simulated`, `source-inspected`, or `not-run`.
- [ ] `.\scripts\build.ps1` succeeds and the binary fits within `0x1F0000`.
- [ ] REG-01 is not passed without device-backed hardware rows.
- [x] `nyquist_compliant: true` set in frontmatter.

**Approval:** approved for planning 2026-06-03; execution results remain pending.
