# Phase 1: Hardware-Safe Pump Control Core - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-05-19
**Phase:** 1-Hardware-Safe Pump Control Core
**Areas discussed:** Relay polarity and inactive safety, Float switch change and debounce, Stop and restart semantics, Timer validation and anti-chatter

---

## Relay Polarity And Inactive Safety

### Default relay polarity in firmware

| Option | Description | Selected |
|--------|-------------|----------|
| Active-high | GPIO HIGH energizes relay; GPIO LOW inactive | |
| Active-low | GPIO LOW energizes relay; GPIO HIGH inactive | yes |
| Do not lock default polarity yet | Leave default for later relay-module confirmation | |

**User's choice:** Active-low because the target relay module is active-low.
**Notes:** Inactive state is GPIO HIGH.

### Initial pump core state during Phase 1 boot/init

| Option | Description | Selected |
|--------|-------------|----------|
| Stopped + relay inactive | Init GPIO and keep relay inactive until start | yes |
| Ready state + relay inactive | Prepare timer/float state but do not energize relay | |
| Start timer immediately after init | Begin default behavior during Phase 1 init | |

**User's choice:** Stopped + relay inactive.
**Notes:** Auto-start belongs to Phase 2.

### Behavior for invalid timer/pin/polarity configuration

| Option | Description | Selected |
|--------|-------------|----------|
| Fail-safe stopped | Do not start; relay inactive; report invalid config | yes |
| Use defaults instead | Replace invalid config with defaults and continue | |
| Keep timer running but relay disabled | Let state machine run for debug while relay stays inactive | |

**User's choice:** Fail-safe stopped.
**Notes:** No silent fallback for unsafe config.

### GPIO initialization order for relay safety

| Option | Description | Selected |
|--------|-------------|----------|
| Set inactive level before enable output | Prepare inactive level before/while configuring GPIO, then write inactive again | yes |
| Config output first, then set inactive | Simpler but may leave a short undefined output window | |
| Let planner choose API sequence | Lock only the no-pulse outcome | |

**User's choice:** Set inactive level before enable output.
**Notes:** Important because active-low relays can energize if driven low.

---

## Float Switch Change And Debounce

### Float switch debounce duration

| Option | Description | Selected |
|--------|-------------|----------|
| Stable 100 ms | Accept new state after 100 ms stable input | yes |
| Stable 250 ms | More conservative debounce, slightly slower response | |
| No debounce in Phase 1 | Raw GPIO changes immediately affect timer selection | |

**User's choice:** Stable 100 ms.
**Notes:** Balances mechanical switch bounce protection with quick response.

### Behavior when debounced float state changes

| Option | Description | Selected |
|--------|-------------|----------|
| Reset immediately into ON phase of the new timer | Switch timer and load ON duration immediately | yes |
| Switch timer but keep current phase | Preserve ON/OFF phase across timer switch | |
| Wait for current phase to finish before switching | Defer timer switch until phase boundary | |

**User's choice:** Reset immediately into ON phase of the new timer.
**Notes:** Matches locked requirement and flow behavior.

### Behavior while raw float input is bouncing before debounce completes

| Option | Description | Selected |
|--------|-------------|----------|
| Do not change active timer until stable | Keep last confirmed float state and continue countdown | yes |
| Freeze countdown during debounce | Pause timer while signal stabilizes | |
| Force relay inactive during debounce | Disable relay while raw input is unstable | |

**User's choice:** Do not change active timer until stable.
**Notes:** Relay should not be forced inactive merely because debounce is pending.

### Initial float read when pump core starts

| Option | Description | Selected |
|--------|-------------|----------|
| Debounce before selecting first timer | Wait 100 ms stable input, then select Timer 1/2 | yes |
| Read raw immediately on start | Start faster but may choose wrong timer briefly | |
| Start Timer 1 until float is stable | Simple but wrong if float is already ON | |

**User's choice:** Debounce before selecting first timer.
**Notes:** During the initial 100 ms stabilization window, relay remains inactive and no timer phase starts.

---

## Stop And Restart Semantics

### Behavior when stop is called while pump core is running

| Option | Description | Selected |
|--------|-------------|----------|
| Clear runtime state | Relay inactive, running=false, clear timer/phase/countdown | yes |
| Pause state | Relay inactive but retain countdown for resume | |
| Stop relay only | Relay inactive while state machine continues internally | |

**User's choice:** Clear runtime state.
**Notes:** User asked whether Pause can be added as another button; captured as deferred idea.

### Behavior when start is called from stopped state

| Option | Description | Selected |
|--------|-------------|----------|
| Debounce float then start ON phase | Confirm float state, select timer, start ON phase | yes |
| Always start Timer 1 ON | Ignore current float state at start | |
| Resume last timer before stop | Restore previous runtime state | |

**User's choice:** Debounce float then start ON phase.
**Notes:** Consistent with clear runtime state on stop.

### Behavior when start is called while already running

| Option | Description | Selected |
|--------|-------------|----------|
| No-op success | Do not reset timer; report already running/success | yes |
| Restart from current float | Reset countdown every time start is called | |
| Return error | Treat repeated start as invalid operation | |

**User's choice:** No-op success.
**Notes:** UI/API should later disable repeated Start clicks, but the core must still tolerate repeated calls safely.

### Behavior when stop is called while already stopped

| Option | Description | Selected |
|--------|-------------|----------|
| No-op success | Keep relay inactive and report success/already stopped | yes |
| Return error | Treat repeated stop as invalid operation | |
| Re-apply inactive only | Write relay inactive again and report success | |

**User's choice:** No-op success.
**Notes:** Keeps start/stop operations idempotent.

---

## Timer Validation And Anti-Chatter

### Minimum accepted ON/OFF duration

| Option | Description | Selected |
|--------|-------------|----------|
| Minimum 1 second | Flexible for testing but can switch relay quickly | |
| Minimum 5 seconds | Reduces chatter risk while still allowing quick tests | yes |
| Minimum 10 seconds | More conservative, slower for testing | |

**User's choice:** Minimum 5 seconds.
**Notes:** Applies to all Timer 1/2 ON/OFF durations.

### Maximum accepted ON/OFF duration

| Option | Description | Selected |
|--------|-------------|----------|
| Maximum 24 hours per phase | Broad practical limit and overflow guard | yes |
| Maximum 99 minutes 59 seconds | Matches likely UI min/sec bounds, more restrictive | |
| No high max beyond overflow guard | Most flexible, may hide bad config | |

**User's choice:** Maximum 24 hours per phase.
**Notes:** Applies per phase duration.

### Behavior when timer config is outside accepted range

| Option | Description | Selected |
|--------|-------------|----------|
| Reject entire config | Do not start; relay inactive; report invalid config | yes |
| Clamp values into range | Modify invalid values automatically | |
| Fallback to default timers | Replace invalid timer config with defaults | |

**User's choice:** Reject entire config.
**Notes:** No clamp and no silent fallback.

### Where Phase 1 default timer constants should live

| Option | Description | Selected |
|--------|-------------|----------|
| app_config.h constants + pump component default struct | Product defaults are central and component exposes usable defaults | yes |
| Only inside pump component | Keeps component self-contained but hides product defaults | |
| Do not add defaults in Phase 1 | Caller must always supply config | |

**User's choice:** `app_config.h` constants plus pump component default struct.
**Notes:** Required because TIME-05/TIME-06 are covered by Phase 1.

---

## the agent's Discretion

- Exact internal API names, enum names, status struct names, and timer implementation mechanism.
- Exact ESP-IDF GPIO calls used to satisfy the no-pulse inactive initialization intent.

## Deferred Ideas

- Add Pause/Resume as a future capability with its own UI/API behavior; not part of Phase 1 start/stop core scope.
