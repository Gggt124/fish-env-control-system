---
phase: 11
slug: baseline-ui-audit-and-state-language
status: approved
nyquist_compliant: true
wave_0_complete: true
created: 2026-06-02
---

# Phase 11 - Validation Strategy

> Per-phase validation contract for the baseline UI audit and implementation-brief handoff.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | Documentation completeness checks, source `rg` checks, `node --check`, impeccable detector, and manual screenshot review |
| **Config file** | `PRODUCT.md`, `DESIGN.md`, `main/static/`, `C:\Users\Copter\.agents\skills\impeccable\scripts\detect.mjs` |
| **Quick run command** | Requirement-specific `rg` checks from each plan task |
| **Full suite command** | `node --check main/static/app.js`; impeccable detector; offline-dependency search; Phase 11 artifact completeness checks; `git diff --name-only -- main/static` |
| **Estimated runtime** | <10 seconds for source checks, plus browser screenshot capture time |

---

## Sampling Rate

- **After every task commit:** Run the task's planned artifact and source `rg` verification command.
- **After every plan wave:** Verify screenshot/report or brief completeness for the completed deliverables.
- **Before `$gsd-verify-work`:** Run the full source check suite and manually review screenshot evidence or explicit not-run reasons.
- **Max feedback latency:** <10 seconds for source checks; browser evidence remains manual-duration validation.

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 11-01-01 | 01 | 1 | UI-13 | T-11-01 | Audit scope stays read-only and preserves stable v1.1 runtime. | source check | `rg "Login|App Shell|Dashboard|Hardware/Install|Wi-Fi|Status|1440|375|Must fix|Should fix|Enhancement" .planning/phases/11-baseline-ui-audit-and-state-language` | N/A command | pending |
| 11-01-02 | 01 | 1 | UI-13 | T-11-01, T-11-02 | Detector and screenshot limitations are recorded instead of silently passing, and committed screenshots are privacy-reviewed. | source check | `rg "impeccable|detect.mjs|screenshot|not-run|simulated|device-backed|Browser|privacy" .planning/phases/11-baseline-ui-audit-and-state-language` | N/A command | pending |
| 11-02-01 | 02 | 2 | A11Y-02, A11Y-03, UI-13 | T-11-03 | Shared state language requires focus, labels, live regions, alerts, touch targets, and local assets. | source check | `rg "loading|success|error|empty|disabled|unavailable|pending reboot|disconnected|focus|aria-live|role=.alert|44x44|offline|footprint" .planning/phases/11-baseline-ui-audit-and-state-language` | N/A command | pending |
| 11-03-01 | 03 | 3 | UI-13, A11Y-02, A11Y-03 | T-11-04 | Downstream briefs preserve traceability, accessibility, and offline constraints. | source check | `rg "Phase 12|Phase 13|problem|evidence|impact|acceptance|ui-ux-pro-max|ESP32|CDN|remote|framework" .planning/phases/11-baseline-ui-audit-and-state-language` | N/A command | pending |
| 11-03-02 | 03 | 3 | UI-13 | T-11-05 | Existing frontend JavaScript remains syntactically valid and embedded assets stay local. | source check | `node --check main/static/app.js`; `rg "http://|https://|cdn|Tailwind|Google Fonts|Material Symbols" main/static` returns no matches; `git diff --name-only -- main/static` returns no files. | N/A command | pending |

---

## Wave 0 Requirements

Existing command-based infrastructure covers Phase 11.

No generated test files or frontend packages are required because:

- Phase 11 creates audit evidence and implementation briefs without changing runtime UI code.
- Browser screenshots are evidence artifacts, not automated behavioral tests.
- Source checks and the deterministic detector provide fast feedback between documentation tasks.

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Review Login, App Shell, Dashboard, Hardware/Install, Wi-Fi, and Status at desktop `1440px` and mobile `375px`. | UI-13, A11Y-02 | Visual hierarchy, overflow, touch sizing, and screenshot evidence require rendered-page review. | Use the Browser plugin where practical. Store screenshots by page and viewport, or record an explicit not-run reason. |
| Review representative loading, error, empty, disabled, and pending-reboot states. | UI-13, A11Y-03 | Some states require safe simulation or a reachable ESP32. | Capture device-backed or simulated evidence and label the evidence source. Record states that cannot be simulated safely. |
| Confirm report priorities and Phase 12/13 routing are actionable. | UI-13 | Brief quality requires human judgment. | Review each finding for priority, evidence, impact, acceptance criteria, and target phase. |

---

## Validation Sign-Off

- [x] All anticipated tasks have automated artifact checks or documented manual evidence.
- [x] Sampling continuity: no three consecutive tasks without a quick verification command.
- [x] Existing infrastructure covers Wave 0; no new files are required.
- [x] No watch-mode flags.
- [x] Source-check feedback latency is <10 seconds.
- [x] `nyquist_compliant: true` set in frontmatter.

**Approval:** approved 2026-06-02 after finalized PLAN.md task mapping
