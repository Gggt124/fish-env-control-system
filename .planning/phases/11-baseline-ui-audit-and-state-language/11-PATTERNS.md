# Phase 11 Pattern Map: Baseline UI Audit And State Language

## Delivery Shape

Phase 11 is a documentation and evidence phase. It audits the existing embedded
web UI, records a shared state language, and produces implementation briefs for
Phases 12 and 13. It must not change shipped files under `main/static/`.

## Existing Analogs

| Deliverable | Existing analog | Pattern to preserve |
| --- | --- | --- |
| Audit evidence | Phase summaries and UAT artifacts in `.planning/phases/` | Record source, method, result, and limitations instead of claiming unverified behavior. |
| Shared UI language | `11-UI-SPEC.md`, `PRODUCT.md`, and `DESIGN.md` | Keep Thai-first user-facing copy, offline operation, compact embedded footprint, and the current plain HTML/CSS/JS stack. |
| Implementation briefs | Phase context files and roadmap success criteria | Translate findings into scoped, acceptance-testable work for the owning downstream phase. |
| Static UI review | `main/static/*.html`, `main/static/style.css`, `main/static/app.js` | Inspect the current pages without introducing a framework, CDN, or external runtime dependency. |

## Artifact Flow

```text
main/static source inventory
        |
        v
detector output + browser/manual evidence
        |
        v
11-BASELINE-UI-AUDIT.md
        |
        v
11-STATE-LANGUAGE.md + 11-ESP32-UI-CHECKLIST.md
        |
        v
11-PHASE-12-BRIEF.md + 11-PHASE-13-BRIEF.md
```

## Source Inspection Anchors

- `main/static/login.html`: login content, failure state, and redirect behavior.
- `main/static/dashboard.html`: overview hierarchy and summary cards.
- `main/static/hardware.html`: active and pending GPIO values, reboot-required feedback, and installer guidance.
- `main/static/status.html`: detailed device status and disconnected states.
- `main/static/wifi.html`: scan, connect, disconnect, and network row interaction.
- `main/static/style.css`: hierarchy, spacing, focus treatment, motion, and touch target sizing.
- `main/static/app.js`: loading, success, error, empty, disabled, unavailable, pending reboot, and disconnected state rendering.

## Validation Commands

- Run the Impeccable detector against `main/static`.
- Search static assets for external URLs and CDN references.
- Search for accessible live regions, focus-visible handling, reduced-motion support, and compact interactive controls.
- Retry the Browser plugin for local screenshot capture. If browser automation is unavailable, use a documented static-server or manual fallback and label unexecuted checks clearly.
- Verify that `git diff --name-only -- main/static` is empty before Phase 11 closes.

## Guardrails

- Do not modify firmware behavior or `main/static/` source in Phase 11.
- Do not expose passwords, session cookies, private SSIDs, MAC addresses, or sensitive local network details in committed screenshots.
- Label evidence as device-backed, simulated, source-inspected, or not run.
- Record Phase 12 and Phase 13 ownership for every actionable finding.

## Pattern Mapping Complete

Phase 11 should execute as three sequential plans:

1. Capture the evidence pack and baseline audit.
2. Define the shared state language and ESP32-adapted checklist.
3. Produce implementation briefs and verify handoff completeness.
