# Float Switch Min-Dwell + Debounce Hardening — Design

- **Date:** 2026-07-09
- **Scope:** `components/pump_control`, `components/app_config`, `main/web_server`, `main/static/`
- **Status:** Ready for user review (self-review passed)
- **Target boards:** ESP32 DevKit V1 30-pin (4MB), ESP32-S3-DevKitC-1 WROOM-1-N16R8 (16MB)

## 1. Problem

The fish pump controller uses a single binary float switch to select between
Timer 1 (relay 1, slow fill) and Timer 2 (relay 2, fast fill). The float maps
to a wet/dry GPIO level. The current debounce logic in
`pump_control.c:413-431` (`update_debounce_locked`) only filters short,
high-frequency noise: a state change is confirmed once the raw GPIO reading
stays constant for `debounce_ms`.

That works for electrical/contact bounce, but does **not** handle water
splashes or ripples that physically wet the sensor for hundreds of
milliseconds up to several seconds:

- The float physically moves and the GPIO genuinely transitions, so the
  signal is "valid" at the sensor level.
- A single splash that keeps the float tilted past `debounce_ms` is
  confirmed as a real level change → the pump switches timers.
- Continuous ripple around the threshold makes the float oscillate and the
  pump relay chatters, which stresses the relay and the pump.

Current default `APP_TEMPLATE_PUMP_FLOAT_DEBOUNCE_MS = 100` is too short to
absorb any realistic splash.

## 2. Goal

Prevent the pump from rapidly switching between Timer 1 and Timer 2 due to
splashes, ripples, or operator-induced water motion, while still responding
promptly to genuine, sustained water level changes. Hardening must be
runtime-configurable from the web dashboard, consistent with the existing
`debounce_ms` setting.

## 3. Non-Goals

- No new sensors (no continuous level sensor, no second float).
- No hysteresis in the water level itself (only one threshold point).
- No automatic tuning or splash frequency analysis.
- No changes to the relay/timer phase state machine beyond the new cooldown
  gate in front of timer switching.
- No OTA, no NVS migration of `min_dwell_sec` beyond existing NVS usage by
  `web_server.c`.

## 4. Approach

Two complementary defenses, both runtime-tunable from the web:

1. **Larger debounce default (Approach A, small).** Raise the default
   `debounce_ms` from 100 ms to 2000 ms. This filters short, transient
   splashes before they ever reach the confirmed state, while still
   responding to genuine changes within ~2 seconds.
2. **Min-dwell cooldown (Approach B, primary).** After the controller
   switches between Timer 1 and Timer 2, ignore any further confirmed float
   state changes for `min_dwell_sec` (default 30 s). This is independent of
   the float reading: even if a long splash survives debounce and produces a
   confirmed state change, the relay does not switch again until the cooldown
   expires.

`min_dwell_sec = 0` disables cooldown and restores legacy behavior (only
debounce protects against splash), which keeps the change backward-compatible
and gives operators an escape hatch.

## 5. Data Flow

```
raw GPIO  -> update_debounce_locked (must stay constant for debounce_ms)
              -> confirmed_float_state
              -> if confirmed != active timer's expected float state:
                   if (now_ms - s_last_switch_ms) < min_dwell_sec*1000:
                       ignore (cooldown active, log debug)
                   else:
                       switch timer (start_channel_for_float_locked)
                       s_last_switch_ms = now_ms
```

The debounce layer still resets `s_pending_*` whenever the raw reading
changes, so continuous ripple keeps `pending` from ever confirming — that is
the first layer of defense. The cooldown layer catches the surviving
splashes that do manage to produce a confirmed change.

## 6. Detailed Changes

### 6.1 `components/app_config/app_config.h`

- Change `APP_TEMPLATE_PUMP_FLOAT_DEBOUNCE_MS` from `100` to `2000`.
- Add `#define APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_SEC 30`.
- Add validation bounds:
  - `#define APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_MIN_SEC 0`   (0 disables)
  - `#define APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_MAX_SEC 3600` (1 hour cap)

### 6.2 `components/pump_control/pump_control.h`

- Add to `pump_control_config_t`:
  ```c
  uint32_t min_dwell_sec;   /* cooldown after switching timers */
  ```
- Add to `pump_control_timer_update_t`:
  ```c
  uint32_t debounce_ms;     /* allow live debounce tuning */
  uint32_t min_dwell_sec;    /* allow live cooldown tuning */
  ```
- Add to `pump_control_status_t`:
  ```c
  uint32_t min_dwell_sec;            /* current configured cooldown */
  uint32_t cooldown_remaining_sec;   /* 0 when not in cooldown */
  ```

### 6.3 `components/pump_control/pump_control.c`

- New file-scope state:
  ```c
  static int64_t s_last_switch_ms;  /* timestamp of last timer switch */
  ```
- `config_valid()`: add `config->min_dwell_sec <= APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_MAX_SEC`
  and `config->debounce_ms > 0` (already checked).
- `pump_control_default_config()`: set
  `.min_dwell_sec = APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_SEC`.
- `reset_runtime_state_locked()`: clear `s_last_switch_ms = 0`.
- `start_channel_for_float_locked()`: when a real switch happens (not a
  no-op where `s_active_timer` is unchanged), set
  `s_last_switch_ms = now_ms`. The function already receives `now_ms`.
- `pump_tick_cb()`: after `update_debounce_locked` reports a confirmed
  change and before calling `start_channel_for_float_locked`, compute:
  ```c
  bool cooldown_active =
      s_config.min_dwell_sec > 0 &&
      (now_ms - s_last_switch_ms) < (int64_t)s_config.min_dwell_sec * 1000;
  if (cooldown_active) {
      /* do not switch; keep current timer; log debug */
  } else {
      start_channel_for_float_locked(s_confirmed_float_state, now_ms);
  }
  ```
- `pump_control_update_timers()`: copy `update->debounce_ms` and
  `update->min_dwell_sec` into `s_config` after validation (reject if out of
  bounds).
- `pump_control_get_status()`: fill `out->min_dwell_sec` and compute
  `out->cooldown_remaining_sec` from `s_last_switch_ms` and current time.
- `pump_control_start()`: reset `s_last_switch_ms` to current `now_ms` so a
  freshly started run does not inherit a stale cooldown from before stop.
- `mark_fault_locked()`: clear `s_last_switch_ms = 0`.

### 6.4 `main/web_server.c`

- In the defaults JSON response (currently around line 930 next to
  `debounce_ms`), add `"min_dwell_sec": defaults.min_dwell_sec`.
- In the POST config parser (around line 1768, the existing
  `"debounce_ms"` string match), add parsing for `"min_dwell_sec"` with
  range validation against
  `APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_MIN_SEC..MAX_SEC`.
- In `/api/status` JSON, add:
  - `"min_dwell_sec"` (current setting)
  - `"cooldown_remaining_sec"` (live countdown)

### 6.5 `main/static/dashboard.html` + `app.js`

- Add a numeric input for "Min Dwell (s)" next to the existing debounce
  field, using the same form-row pattern.
- On status refresh, if `cooldown_remaining_sec > 0`, show a small badge
  "Cooldown: Xs" on the active timer card; hide when 0.
- Submit `min_dwell_sec` together with `debounce_ms` in the existing
  config POST call.

## 7. Edge Cases

| Case | Behavior |
|------|----------|
| `min_dwell_sec = 0` | Cooldown disabled; only debounce filters. Legacy behavior. |
| Stop then Start during cooldown | `start()` resets `s_last_switch_ms` to now; cooldown restarts from 0. |
| `update_timers` during cooldown | Cooldown timestamp is preserved; new `min_dwell_sec` applies from the next switch only. |
| Fault during cooldown | `mark_fault_locked()` clears `s_last_switch_ms = 0`. |
| Float UNKNOWN during stabilizing | Cooldown gate does not apply; `start_channel_for_float_locked` runs as before on first confirmed state. |
| Power-on / re-init | `reset_runtime_state_locked()` clears `s_last_switch_ms`; first switch sets it. |
| `min_dwell_sec > MAX_SEC` from API | Rejected by `pump_control_update_timers` and by web_server parser; previous value retained, 400 returned. |

## 8. Validation Bounds

| Field | Min | Max | Default |
|-------|-----|-----|---------|
| `debounce_ms` | 1 | 60000 (existing) | 2000 (new) |
| `min_dwell_sec` | 0 | 3600 | 30 |

## 9. Testing

- Unit-style checks on `update_debounce_locked` + cooldown gate (in
  `tests/` following existing test layout):
  1. Confirmed change within cooldown → ignored, relay unchanged.
  2. Confirmed change after cooldown → switch happens,
     `s_last_switch_ms` updated.
  3. `min_dwell_sec = 0` → cooldown never blocks.
  4. `update_timers` mid-cooldown → cooldown timestamp preserved.
  5. Stop/Start → `s_last_switch_ms` reset to now.
- Build both targets:
  ```powershell
  .\scripts\build.ps1 -Target esp32
  .\scripts\build.ps1 -Target esp32s3
  ```
- Manual test on hardware: flick the float manually within 30 s and verify
  the relay does not switch; wait > 30 s and flick again to verify it does.

## 10. Risks / Open Items

- **Slower response to genuine rapid level changes.** Acceptable for a koi
  pond where level changes slowly. If a future use case needs faster
  response, `min_dwell_sec` can be lowered to 0 from the dashboard without
  reflashing.
- **No splash-specific filtering.** This is a time-based gate, not a
  frequency/majority filter. If ripple proves persistent and slower than
  `min_dwell_sec`, Approach C (sliding window majority) can be layered on
  later.
- **NVS persistence of `min_dwell_sec`** follows whatever path
  `web_server.c` already uses for `debounce_ms`; no new NVS key is
  introduced by this design.
