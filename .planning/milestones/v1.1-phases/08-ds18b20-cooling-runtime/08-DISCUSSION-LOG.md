# Phase 8: DS18B20 Cooling Runtime - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md - this log preserves the alternatives considered.

**Date:** 2026-05-23
**Phase:** 8-DS18B20 Cooling Runtime
**Areas discussed:** Sensor fault behavior, Cooling mode semantics, Compressor protection timing, Runtime visibility/status

---

## Folded Todo

| Option | Description | Selected |
|--------|-------------|----------|
| Fold it | Carry forward DS18B20 wiring, cooling relay, cooling defaults, and compressor-protection contract into Phase 8 planning. | yes |
| Reviewed only | Record that it was considered but do not make it part of Phase 8 decisions. | |
| Skip | Ignore this todo for Phase 8 context. | |

**User's choice:** Fold it into Phase 8 context.
**Notes:** The matched todo mostly covered Phase 6 foundation work, but its DS18B20 and compressor-protection decisions remain directly relevant to Phase 8.

---

## Sensor Fault Behavior

| Question | Option | Description | Selected |
|----------|--------|-------------|----------|
| When should firmware declare fault? | Fail after one read miss | Fastest fail-safe response, but may be too sensitive to transient bus noise. | |
| When should firmware declare fault? | Fail after multiple consecutive misses | Reduces false faults while still failing safe. | yes |
| When should firmware declare fault? | Separate startup unknown from runtime fault | Treat startup separately from runtime faults. | |
| Failure threshold | 2 consecutive failures | Faster fault detection with minor noise tolerance. | |
| Failure threshold | 3 consecutive failures | Balanced default for 1-Wire sensor reads. | yes |
| Failure threshold | 5 consecutive failures | More tolerant, but leaves unknown/failing state longer. | |
| Startup before first valid reading | Relay OFF, not fault yet | Wait for first valid reading while staying safe. | yes |
| Startup before first valid reading | Fault immediately | Safest-looking state, but shows fault on normal boot delay. | |
| Startup before first valid reading | Use last value from NVS | Smoother UI, but unsafe because stale temperature may be wrong. | |
| Fault recovery | Recover after one successful reading | Fast recovery but prone to bouncing on intermittent wiring. | |
| Fault recovery | Recover after multiple successful readings | Requires stable sensor recovery before clearing fault. | yes |
| Fault recovery | Require user restart | Most manual control, but poor self-recovery. | |

**User's choice:** Runtime fault after 3 consecutive failed reads; startup unknown keeps relay OFF without immediate fault; clear fault after 2 consecutive successful reads.
**Notes:** Automatic cooling must stay OFF during unknown/fault. Test ON is discussed separately as a wiring-test exception.

---

## Cooling Mode Semantics

| Question | Option | Description | Selected |
|----------|--------|-------------|----------|
| Runtime control model | Use mode as the main control | Auto, Force OFF, and Test ON directly express runtime behavior; auto_enable remains boot preference. | yes |
| Runtime control model | Separate enabled from mode | More flexible but redundant and easier to confuse. | |
| Runtime control model | Use auto_enable as runtime on/off | Fewer fields, but mixes boot preference with current state. | |
| Boot with auto_enable=true and unknown temperature | mode=Auto, relay OFF until sensor reading succeeds | Preserves Auto intent while staying fail-safe. | yes |
| Boot with auto_enable=true and unknown temperature | Start Force OFF then switch to Auto later | Safe, but mode changes automatically in a confusing way. | |
| Boot with auto_enable=true and unknown temperature | Do not start runtime until sensor is ready | Adds another state layer. | |
| Test ON during fault/unknown | Allow with timeout and compressor protection | Supports wiring verification while limiting risk. | yes |
| Test ON during fault/unknown | Reject during fault/unknown | Safest, but prevents relay testing when sensor is unavailable. | |
| Test ON during fault/unknown | Allow only during unknown, not fault | Mixed rule that is harder to explain and validate. | |
| Test ON timeout behavior | Return to previous mode | Matches requirement and user expectation. | yes |
| Test ON timeout behavior | Always return to Force OFF | Safer but does not match requirement when starting from Auto. | |
| Test ON timeout behavior | Return to Auto if auto_enable=true | Over-couples boot preference to runtime state. | |

**User's choice:** `mode` is the runtime control. `auto_enable` is boot-only. Test ON can run during sensor unknown/fault, is timeout-limited, returns to previous mode, and does not persist across reboot.
**Notes:** Test ON must be visibly reported as manual/test override.

---

## Compressor Protection Timing

| Question | Option | Description | Selected |
|----------|--------|-------------|----------|
| Boot last-off initialization | Treat relay as just OFF at boot and wait min off-time | Safest for compressor after reboot or power loss. | yes |
| Boot last-off initialization | Allow immediate ON if temperature reaches threshold | Faster cooling but risks short cycling. | |
| Boot last-off initialization | Persist last-off timestamp in NVS | More complex and unreliable without a trusted RTC after power loss. | |
| Auto demand during lockout | Relay OFF and show lockout countdown/status | Enforces protection and explains why cooling is blocked. | yes |
| Auto demand during lockout | Turn ON if temperature is far above threshold | Improves cooling response but weakens protection. | |
| Auto demand during lockout | Add emergency override | New capability outside Phase 8. | |
| Test ON and min off-time | Test ON waits like Auto | Safety-first and consistent. | yes |
| Test ON and min off-time | Test ON always bypasses | Convenient but risky for compressor loads. | |
| Test ON and min off-time | Bypass only with explicit override flag | Flexible, but belongs in later API/UI scope. | |
| Restart min off-time | Restart on every relay ON-to-OFF transition | Covers Auto, Force OFF, reinit, Test timeout, and faults. | yes |
| Restart min off-time | Restart only when Auto shuts off by hysteresis | Misses manual/fault short-cycle cases. | |
| Restart min off-time | Restart only for user-driven OFF | Misses automatic control cases. | |

**User's choice:** Boot/reinit starts a 300 second lockout. Auto and Test ON both obey lockout. Restart min off-time on every relay ON-to-OFF transition.
**Notes:** If Test ON is blocked by compressor protection, status must say so.

---

## Runtime Visibility/Status

| Question | Option | Description | Selected |
|----------|--------|-------------|----------|
| Status route | Add separate `/api/cooling/status` | Keeps cooling runtime separate from pump runtime and supports Phase 9 expansion. | yes |
| Status route | Put cooling fields in `/api/pump/status` | Easy dashboard access, but mixes separate domains. | |
| Status route | Put only in `/api/status` | Good summary location, poor runtime detail surface. | |
| Minimum status fields | Complete debug/UI/validation field set | Supports manual validation and Phase 10 dashboard work. | yes |
| Minimum status fields | Operator summary only | Smaller but not enough detail for debugging. | |
| Minimum status fields | Machine state only | Backend-friendly but UI would need config route earlier. | |
| Sensor/fault representation | Stable string enums | Machine-readable and UI-safe. | yes |
| Sensor/fault representation | Booleans only | Simpler but loses unknown/fault/config detail. | |
| Sensor/fault representation | Human text only | Easy to display, brittle for validation. | |
| Blocked cooling representation | Separate actual relay, demand, and blocked_reason | Clearly distinguishes demand from relay output. | yes |
| Blocked cooling representation | Only relay_energized plus lockout_active | Field-light but ambiguous. | |
| Blocked cooling representation | Single status_text | Human-friendly but weak API contract. | |

**User's choice:** Add authenticated `GET /api/cooling/status`; expose stable status enums and separate actual relay state, cooling demand, and block reason.
**Notes:** Full cooling config/mutation API remains Phase 9 scope.

---

## the agent's Discretion

- Exact C component names, struct names, enum constants, DS18B20 driver strategy, polling interval, JSON field ordering, and helper names.
- Whether to add a small summary to `/api/status`, provided detailed runtime status remains under `/api/cooling/status`.

## Deferred Ideas

None - discussion stayed within phase scope.
