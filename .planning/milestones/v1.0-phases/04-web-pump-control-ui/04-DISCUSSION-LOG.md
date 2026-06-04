# Phase 4: Web Pump Control UI - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md - this log preserves the alternatives considered.

**Date:** 2026-05-20
**Phase:** 4-Web Pump Control UI
**Areas discussed:** Pump UI Structure, Timer Settings And Save Flow, Start Stop And Live Status, Thai Status Wording, Refresh And Countdown Behavior

---

## Pump UI Structure

| Question | Options Considered | User's Choice |
|----------|--------------------|---------------|
| Dashboard role | Replace dashboard with Pump Control; Add Pump Control section to current dashboard; Create separate `/pump` page | Replace dashboard with Pump Control |
| First panel | Runtime state + countdown + Start/Stop; Timer settings + Save; Two-column runtime/settings | Runtime state + countdown + Start/Stop |
| Timer settings placement | Same page below main status; Collapsible section; Separate settings route/tab | Same page below main status |
| Existing Wi-Fi/system info | Compact quick status row; Move nearly all to Status/Wi-Fi pages; Keep all old cards at page bottom | Compact quick status row |

**Notes:** The user wants `/dashboard` to become the primary pump-control page. Wi-Fi/system status should remain visible but secondary.

---

## Timer Settings And Save Flow

| Question | Options Considered | User's Choice |
|----------|--------------------|---------------|
| Timer inputs | Separate minute/second fields; Single `MM:SS` field; Preset/stepper controls | Separate minute/second fields |
| UI validation | Full validation before API request; Basic numeric validation only; Backend-only validation | Full validation before API request |
| After save | Keep form editable with saved feedback; Lock inputs like prototype; Reload device config after save | Keep form editable with saved feedback |
| Unsaved changes | In-page warning only; Disable Start until save/cancel; Auto-save before Start | In-page warning only |

**Notes:** The UI should not copy the reference simulation's save-lock-start flow. The firmware UI should let users keep editing and saving while clearly showing unsaved changes.

---

## Start Stop And Live Status

| Question | Options Considered | User's Choice |
|----------|--------------------|---------------|
| Start/Stop controls | Two clear top-panel buttons; Single toggle button; Extra-prominent emergency Stop | Two clear top-panel buttons |
| Pending command state | Show pending and disable both buttons; Optimistic update; Toast only | Show pending and disable both buttons |
| Runtime hierarchy | Running -> Countdown -> Timer/Phase -> Relay/Float; Timer/Phase first; Relay first | Running -> Countdown -> Timer/Phase -> Relay/Float |
| Status API failure | Degraded usable state; Hide pump panel; Keep last value with stale label | Degraded usable state |

**Notes:** Start/Stop should be fast to reach but should not show optimistic runtime state before the device responds.

---

## Thai Status Wording

| Question | Options Considered | User's Choice |
|----------|--------------------|---------------|
| Float wording | `ลูกลอย: ON/OFF (ใช้ Timer ...)`; Water low/high; `Float Switch: OFF/ON` | `ลูกลอย: ON/OFF (ใช้ Timer ...)` |
| Relay wording | Relay working/stopped; Relay power/no power; Relay ON/OFF with Thai explanation | Relay ON/OFF with Thai explanation |
| Timer phase wording | Phase ON/OFF; `ช่วงเปิด/ช่วงปิด`; Pumping/resting | `ช่วงเปิด/ช่วงปิด` |
| Controller running wording | `ระบบปั๊ม: กำลังทำงาน/หยุดอยู่`; Pump open/closed; Controller Running/Stopped | `ระบบปั๊ม: กำลังทำงาน/หยุดอยู่` |

**Notes:** Wording must keep controller running state, timer phase, relay state, and float state distinct.

---

## Refresh And Countdown Behavior

| Question | Options Considered | User's Choice |
|----------|--------------------|---------------|
| Status refresh model | Poll every 1s; Poll every 2s; Poll every 5s; Local countdown plus periodic sync | Local countdown plus periodic sync |
| Sync cadence | Local tick every 1s + sync every 2s; Local tick every 1s + sync every 5s; Local tick every 1s + sync every 10s | Local tick every 1s + sync every 2s |
| Temporary sync failure | Continue countdown with waiting-for-sync label; Stop countdown immediately; Keep last value without warning | Continue countdown with waiting-for-sync label |
| Long sync failure threshold | 5 seconds; 10 seconds; 30 seconds | 5 seconds |
| Hidden tab behavior | Stop local ticking while hidden, sync on visible; Keep ticking where possible; Reload on visible | Stop local ticking while hidden, sync on visible |

**Notes:** The user proposed separating browser-side counting from device polling, then accepted the final rule: browser countdown ticks every 1 second, `/api/pump/status` syncs every 2 seconds, and device status always wins.

---

## the agent's Discretion

- Exact layout names, CSS class names, helper names, and visual styling details within the existing design system.
- Exact Thai microcopy as long as it preserves the locked semantic distinctions.
- Exact compact presentation of Wi-Fi/system quick status.

## Deferred Ideas

None.
