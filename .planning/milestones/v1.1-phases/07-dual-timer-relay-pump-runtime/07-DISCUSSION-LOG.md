# Phase 7: Dual Timer Relay Pump Runtime - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md - this log preserves the alternatives considered.

**Date:** 2026-05-23
**Phase:** 7-Dual Timer Relay Pump Runtime
**Areas discussed:** Float transition timing, Dual relay API/status contract, Timer start phase semantics, Minimum dashboard clarity

---

## Float Transition Timing

| Question | Option | Selected |
|----------|--------|----------|
| During float debounce, how should relays behave when the raw signal first changes? | Wait for debounce before switching | yes |
| During float debounce, how should relays behave when the raw signal first changes? | Turn both relays off during debounce | |
| During float debounce, how should relays behave when the raw signal first changes? | Switch immediately from raw signal | |
| During float debounce, how should relays behave when the raw signal first changes? | Let the agent decide | |
| After debounce confirms a real float change, what sequence should the channel switch use? | Turn old relay off first, then start new timer | yes |
| After debounce confirms a real float change, what sequence should the channel switch use? | Start new timer, then turn old relay off | |
| After debounce confirms a real float change, what sequence should the channel switch use? | Turn both off, wait briefly, then start | |
| After debounce confirms a real float change, what sequence should the channel switch use? | Let the agent decide | |
| At Start or auto-start, what should happen while float state is still unknown during stabilization? | Both relays off until float confirmed | yes |
| At Start or auto-start, what should happen while float state is still unknown during stabilization? | Use first raw reading immediately | |
| At Start or auto-start, what should happen while float state is still unknown during stabilization? | Use Timer 1 as fallback | |
| At Start or auto-start, what should happen while float state is still unknown during stabilization? | Let the agent decide | |
| When stopped but reading float for display, how should active timer be shown? | Show timer that would be used if Start is pressed | yes |
| When stopped but reading float for display, how should active timer be shown? | active_timer = none plus preview field | |
| When stopped but reading float for display, how should active timer be shown? | active_timer = none only | |
| When stopped but reading float for display, how should active timer be shown? | Let the agent decide | |

**User's choice:** Wait for debounce before switching; turn old relay off first; keep both relays off while float is unknown; show the ready timer while stopped.
**Notes:** User added that dashboard/status should show float state before Start and should not show unknown just because the controller is stopped.

---

## Dual Relay API/status Contract

| Question | Option | Selected |
|----------|--------|----------|
| How should Phase 7 change pump status JSON for dual relays without breaking existing UI/API consumers? | Add new fields and keep old fields | yes |
| How should Phase 7 change pump status JSON for dual relays without breaking existing UI/API consumers? | Switch fully to dual-relay fields | |
| How should Phase 7 change pump status JSON for dual relays without breaking existing UI/API consumers? | Keep old fields only | |
| How should Phase 7 change pump status JSON for dual relays without breaking existing UI/API consumers? | Let the agent decide | |
| What string values should active_relay use? | relay1 / relay2 / none | yes |
| What string values should active_relay use? | pump_relay_1 / pump_relay_2 / none | |
| What string values should active_relay use? | gpio26 / gpio27 / none | |
| What string values should active_relay use? | Let the agent decide | |
| In stopped state, how should ready channel and relay state be represented? | active_timer/active_relay show ready channel, relay energized false | yes |
| In stopped state, how should ready channel and relay state be represented? | active_timer/active_relay none plus ready fields | |
| In stopped state, how should ready channel and relay state be represented? | active_timer/active_relay none only | |
| In stopped state, how should ready channel and relay state be represented? | Let the agent decide | |
| How should firmware handle impossible or unsafe relay states such as two pump relays energized together or GPIO write/state mismatch? | Fail-safe stop plus status error | yes |
| How should firmware handle impossible or unsafe relay states such as two pump relays energized together or GPIO write/state mismatch? | Try to recover and continue | |
| How should firmware handle impossible or unsafe relay states such as two pump relays energized together or GPIO write/state mismatch? | Log only | |
| How should firmware handle impossible or unsafe relay states such as two pump relays energized together or GPIO write/state mismatch? | Let the agent decide | |

**User's choice:** Add dual-relay fields and preserve old compatibility fields; use `active_relay: relay1|relay2|none`; show ready channel while stopped with all relays false; fail safe on impossible relay state.
**Notes:** `relay_energized` remains a compatibility alias for the active/ready channel.

---

## Timer Start Phase Semantics

| Question | Option | Selected |
|----------|--------|----------|
| If Timer 1 or Timer 2 has start_phase = OFF, how should Start/auto-start or a float switch into that channel begin? | Start the real OFF phase | yes |
| If Timer 1 or Timer 2 has start_phase = OFF, how should Start/auto-start or a float switch into that channel begin? | Use start_phase only for boot/Start and make float switches start ON | |
| If Timer 1 or Timer 2 has start_phase = OFF, how should Start/auto-start or a float switch into that channel begin? | Ignore start_phase in Phase 7 | |
| If Timer 1 or Timer 2 has start_phase = OFF, how should Start/auto-start or a float switch into that channel begin? | Let the agent decide | |
| If float changes while the current channel is in OFF phase, what should happen? | Restart new channel from its start_phase | yes |
| If float changes while the current channel is in OFF phase, what should happen? | Wait for current OFF phase to finish | |
| If float changes while the current channel is in OFF phase, what should happen? | Carry the current phase into the new channel | |
| If float changes while the current channel is in OFF phase, what should happen? | Let the agent decide | |
| When stopped and dashboard/status previews the ready channel, what phase/countdown should be shown? | Show start_phase and duration that would start | yes |
| When stopped and dashboard/status previews the ready channel, what phase/countdown should be shown? | phase idle/countdown 0 until Start | |
| When stopped and dashboard/status previews the ready channel, what phase/countdown should be shown? | Show only ready timer, no countdown | |
| When stopped and dashboard/status previews the ready channel, what phase/countdown should be shown? | Let the agent decide | |
| If config changes timer durations/start phases while controller is running, how should Phase 7 apply it? | Stop safe, reinit, restart from current float/start_phase | yes |
| If config changes timer durations/start phases while controller is running, how should Phase 7 apply it? | Apply new duration to current phase immediately | |
| If config changes timer durations/start phases while controller is running, how should Phase 7 apply it? | Save for next Stop/Start | |
| If config changes timer durations/start phases while controller is running, how should Phase 7 apply it? | Let the agent decide | |

**User's choice:** Respect configured start phase everywhere; restart selected channel from its configured start phase on every confirmed float transition; preview start phase/countdown while stopped; preserve safe stop/reinit/restart save semantics.
**Notes:** This locks the Phase 6 `t1_start`/`t2_start` schema as runtime-relevant in Phase 7.

---

## Minimum Dashboard Clarity

| Question | Option | Selected |
|----------|--------|----------|
| How much should Phase 7 touch the static dashboard/UI? | Minimum changes to make mapping/status correct | yes |
| How much should Phase 7 touch the static dashboard/UI? | Do not touch UI | |
| How much should Phase 7 touch the static dashboard/UI? | Full dual-relay UI refresh | |
| How much should Phase 7 touch the static dashboard/UI? | Let the agent decide | |
| What float mapping wording should UI use? | Float ON -> Timer 1 / Relay 1, Float OFF -> Timer 2 / Relay 2 | yes |
| What float mapping wording should UI use? | Float ON/OFF plus timer only | |
| What float mapping wording should UI use? | Show active timer and relay separately only | |
| What float mapping wording should UI use? | Let the agent decide | |
| How should dashboard show relay state in Phase 7? | Show Relay 1 and Relay 2 separately | yes |
| How should dashboard show relay state in Phase 7? | Show active relay only | |
| How should dashboard show relay state in Phase 7? | Keep existing relay wording | |
| How should dashboard show relay state in Phase 7? | Let the agent decide | |
| How should stopped-state preview text be presented? | Ready to start, not running | yes |
| How should stopped-state preview text be presented? | Display like normal runtime | |
| How should stopped-state preview text be presented? | Hide countdown while stopped | |
| How should stopped-state preview text be presented? | Let the agent decide | |

**User's choice:** Make minimum dashboard/UI updates for correctness; state the new float mapping directly; show Relay 1 and Relay 2 separately; make stopped-state preview clearly ready-to-start, not running.
**Notes:** Full dashboard redesign remains outside Phase 7.

---

## the agent's Discretion

- Exact internal C helper names, struct field placement, JSON grouping, UI element IDs, and Thai microcopy details.
- Exact fault field representation, provided unsafe relay behavior fails safe and is visible.

## Deferred Ideas

None.
