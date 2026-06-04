# Phase 11: Baseline UI Audit And State Language - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md - this log preserves the alternatives considered.

**Date:** 2026-06-02
**Phase:** 11-Baseline UI Audit And State Language
**Areas discussed:** Audit Evidence Pack, Shared State Language, Accessibility Contract, Phase 12-13 Handoff Briefs

---

## Audit Evidence Pack

| Decision | Selected choice | Alternatives considered |
|----------|-----------------|-------------------------|
| Screenshot coverage | Every page: Login, Dashboard, Hardware/Install, Wi-Fi, and Status | Primary pages only; planner discretion |
| Mobile viewport | `375px` | `360px`; both widths; planner discretion |
| Desktop viewport | `1440px` | `1280px`; both widths; planner discretion |
| Inspection workflow | Browser plugin with `impeccable` audit automation where practical | Manual-only inspection |
| State evidence | Include representative loading, error, empty, disabled, and pending-reboot states where safe to simulate | Happy-path screenshots only |
| Finding priority | `Must fix / Should fix / Enhancement` | `Critical / High / Medium / Low`; planner discretion |
| Storage | Markdown report plus screenshot folder organized by page and viewport | Markdown only; planner discretion |

**User's choice:** Use full automated evidence capture with a compact three-level finding priority.
**Notes:** Screenshot evidence must serve the Phase 12-13 implementation pass, not exist as a visual-only archive.

---

## Shared State Language

| Decision | Selected choice | Alternatives considered |
|----------|-----------------|-------------------------|
| In-progress operations | Action-specific text, disabled control, and small spinner | Skeleton loading; toast only; planner discretion |
| Successful actions | Contextual status plus short toast where useful | Toast only; persistent page text only; planner discretion |
| Errors | Contextual error, recovery guidance, and assistive announcement | Page-top banner; toast only; planner discretion |
| Unavailable states | Separate processing, unavailable with reason, and pending reboot | Generic disabled state; color-only distinction; planner discretion |
| Asset delivery | Local embedded assets only | CDN or remote dependency use |
| Future TFT | Keep vocabulary short and layout-independent; defer TFT implementation | Implement TFT in the current milestone |

**User's choice:** Keep state feedback explicit, concise, offline-safe, and reusable by a future TFT interface.
**Notes:** The device may run through SoftAP or a local network without internet. Do not add CDN dependencies, remote fonts, icon packages, frameworks, large images, or heavy animations. Track embedded asset footprint.

---

## Accessibility Contract

| Decision | Selected choice | Alternatives considered |
|----------|-----------------|-------------------------|
| Keyboard focus | Clear `2-4px` blue focus ring on every interactive control | Main controls only; browser defaults; planner discretion |
| Dynamic announcements | `aria-live="polite"` for info/success; `role="alert"` for actionable errors | Alert for everything; polite for everything; planner discretion |
| Mobile touch targets | At least `44x44px` | `48x48px`; fix only difficult controls; planner discretion |
| Labels | Visible labels plus concise helper text where needed | Labels only; placeholders where possible; planner discretion |

**User's choice:** Apply the complete accessibility contract across all audited pages.
**Notes:** Preserve concise Thai user-facing wording and precise English technical nouns such as `GPIO`, `STA`, `AP`, and `IP`.

---

## Phase 12-13 Handoff Briefs

| Decision | Selected choice | Alternatives considered |
|----------|-----------------|-------------------------|
| Finding organization | By page and flow, tagged for Phase 12 or Phase 13 | By issue category; combined cross-page summary; planner discretion |
| Finding detail | Problem, screenshot evidence, user impact, and acceptance criteria | Problem plus evidence; short task list; planner discretion |
| Checklist adaptation | Select applicable `ui-ux-pro-max` checks and record rationale | Apply full checklist; undocumented guidance; planner discretion |
| Work split | Separate Phase 12 and Phase 13 briefs | Combined backlog; audit report only; planner discretion |

**User's choice:** Produce separate actionable briefs for Phase 12 and Phase 13.
**Notes:** Phase 12 receives Login, App Shell, and Dashboard. Phase 13 receives Hardware/Install, Wi-Fi, Status, and applicable state gaps.

---

## the agent's Discretion

- Audit report filename and screenshot subdirectory naming.
- Browser automation script structure.
- Exact Thai microcopy inventory format.
- Phase 12 and Phase 13 brief filenames.

## Deferred Ideas

- Add a `2.4-inch` TFT local display in a future milestone. Reuse the Phase 11 state vocabulary, but do not implement TFT support in Phase 11-14.
