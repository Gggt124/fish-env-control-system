### Task 3: pump_control.c cooldown logic

**Files:**
- Modify: `components/pump_control/pump_control.c:18-38` (state), `:87-109` (config_valid), `:111-128` (reset), `:187-200` (mark_fault), `:394-411` (start_channel), `:413-431` (debounce — no change), `:447-488` (tick cb), `:504-537` (default config), `:608-643` (start), `:669-720` (update_timers), `:722-764` (get_status)

**Interfaces:**
- Consumes: `APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_SEC`, `_MIN_SEC`, `_MAX_SEC` from Task 1; struct fields from Task 2.
- Produces: `pump_control` runtime that gates timer switches with a cooldown; `pump_control_get_status` reports `min_dwell_sec` and `cooldown_remaining_sec`. Consumed by Tasks 5, 6.

- [ ] **Step 1: Add cooldown timestamp to file-scope state**

In `components/pump_control/pump_control.c`, after the existing static state block (around line 38, after `s_current_phase_duration_sec;`), add:

```c
static int64_t s_last_switch_ms;   /* timestamp of the most recent Timer1<->Timer2 switch */
```

- [ ] **Step 2: Add min_dwell_sec validation to config_valid**

In `config_valid()` (lines 87-109), the current check rejects `config->debounce_ms == 0`. Add a min_dwell range check right after the debounce check:

```c
        config->debounce_ms == 0 ||
        config->min_dwell_sec > APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_MAX_SEC) {
        return false;
    }
```

- [ ] **Step 3: Reset s_last_switch_ms in reset_runtime_state_locked**

In `reset_runtime_state_locked()` (lines 111-128), add at the end before the closing brace:

```c
    s_current_phase_duration_sec = 0;
    s_last_switch_ms = 0;
}
```

- [ ] **Step 4: Reset s_last_switch_ms in mark_fault_locked**

In `mark_fault_locked()` (lines 187-200), add before the final `force_both_relays_inactive_for_config(&s_config);`:

```c
    s_phase_deadline_ms = 0;
    s_last_switch_ms = 0;
    force_both_relays_inactive_for_config(&s_config);
```

- [ ] **Step 5: Record s_last_switch_ms on a real timer switch**

In `start_channel_for_float_locked()` (lines 394-411), the function currently always calls `set_selected_channel_for_float_locked` then `set_phase_locked`. We must only stamp the switch time when the active timer actually changes. Replace the body of `start_channel_for_float_locked` with a version that detects the change. Current function:

```c
static void start_channel_for_float_locked(pump_control_float_state_t float_state, int64_t now_ms)
{
    force_both_relays_inactive_locked();
    if (s_fault) {
        return;
    }

    set_selected_channel_for_float_locked(float_state);
    if (s_active_timer == PUMP_CONTROL_TIMER_NONE) {
        s_phase = PUMP_CONTROL_PHASE_IDLE;
        s_countdown_sec = 0;
        s_phase_deadline_ms = 0;
        force_both_relays_inactive_locked();
        return;
    }

    set_phase_locked(start_phase_to_timer_phase(active_timer_start_phase_locked()), now_ms);
}
```

Replace with:

```c
static void start_channel_for_float_locked(pump_control_float_state_t float_state, int64_t now_ms)
{
    force_both_relays_inactive_locked();
    if (s_fault) {
        return;
    }

    pump_control_active_timer_t prev_timer = s_active_timer;
    set_selected_channel_for_float_locked(float_state);
    if (s_active_timer == PUMP_CONTROL_TIMER_NONE) {
        s_phase = PUMP_CONTROL_PHASE_IDLE;
        s_countdown_sec = 0;
        s_phase_deadline_ms = 0;
        force_both_relays_inactive_locked();
        return;
    }

    if (s_active_timer != prev_timer) {
        s_last_switch_ms = now_ms;
    }

    set_phase_locked(start_phase_to_timer_phase(active_timer_start_phase_locked()), now_ms);
}
```

- [ ] **Step 6: Gate float_changed in pump_tick_cb with cooldown**

In `pump_tick_cb()` (lines 447-488), find the block:

```c
    if (float_changed) {
        start_channel_for_float_locked(s_confirmed_float_state, now_ms);
    } else {
        advance_phase_if_needed_locked(now_ms);
    }
```

Replace with:

```c
    if (float_changed) {
        bool cooldown_active =
            s_config.min_dwell_sec > 0 &&
            (now_ms - s_last_switch_ms) < ((int64_t)s_config.min_dwell_sec * 1000);
        if (cooldown_active) {
            ESP_LOGD(TAG, "float changed but cooldown active (%us), ignoring",
                     (unsigned)s_config.min_dwell_sec);
            s_countdown_sec = seconds_remaining(now_ms);
        } else {
            start_channel_for_float_locked(s_confirmed_float_state, now_ms);
        }
    } else {
        advance_phase_if_needed_locked(now_ms);
    }
```

- [ ] **Step 7: Set min_dwell_sec in pump_control_default_config**

In `pump_control_default_config()` (lines 504-537), the initializer sets `.debounce_ms = APP_TEMPLATE_PUMP_FLOAT_DEBOUNCE_MS,`. Add immediately after:

```c
        .debounce_ms = APP_TEMPLATE_PUMP_FLOAT_DEBOUNCE_MS,
        .min_dwell_sec = APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_SEC,
```

- [ ] **Step 8: Reset s_last_switch_ms in pump_control_start**

In `pump_control_start()` (lines 608-643), after `s_running = true;` and before the `s_initial_stabilizing` assignment, add:

```c
    s_running = true;
    s_last_switch_ms = esp_timer_get_time() / 1000;
    s_initial_stabilizing = s_confirmed_float_state == PUMP_CONTROL_FLOAT_UNKNOWN;
```

- [ ] **Step 9: Apply debounce_ms and min_dwell_sec in pump_control_update_timers**

In `pump_control_update_timers()` (lines 669-720), the validation block (lines 680-688) checks timer config, polarity, and start phase. Add bounds checks for the two new fields. After the existing validation `if (...)` block that returns false, add:

```c
    if (!timer_config_valid(&update->timer1) ||
        !timer_config_valid(&update->timer2) ||
        !polarity_valid(update->relay1_polarity) ||
        !polarity_valid(update->relay2_polarity) ||
        !start_phase_valid(update->timer1_start_phase) ||
        !start_phase_valid(update->timer2_start_phase) ||
        update->debounce_ms == 0 ||
        update->min_dwell_sec > APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_MAX_SEC) {
        ESP_LOGE(TAG, "pump_control_update_timers: invalid update fields");
        return false;
    }
```

Then in the apply block (after `s_config.relay_polarity = update->relay1_polarity; /* legacy alias */`), add:

```c
    s_config.relay_polarity     = update->relay1_polarity; /* legacy alias */
    s_config.debounce_ms        = update->debounce_ms;
    s_config.min_dwell_sec     = update->min_dwell_sec;
```

- [ ] **Step 10: Report min_dwell_sec and cooldown_remaining_sec in pump_control_get_status**

In `pump_control_get_status()` (lines 722-764), the status fields are filled near the end. After `out->total_duration_sec = ...` (lines 759-760), add:

```c
    out->total_duration_sec = (s_phase == PUMP_CONTROL_PHASE_ON || s_phase == PUMP_CONTROL_PHASE_OFF) 
                              ? s_current_phase_duration_sec : 0;
    out->min_dwell_sec = s_config.min_dwell_sec;
    if (s_running && s_config.min_dwell_sec > 0 && s_last_switch_ms > 0) {
        int64_t now_ms_local = esp_timer_get_time() / 1000;
        int64_t elapsed_ms = now_ms_local - s_last_switch_ms;
        int64_t cooldown_ms = (int64_t)s_config.min_dwell_sec * 1000;
        int64_t remaining_ms = cooldown_ms - elapsed_ms;
        out->cooldown_remaining_sec = remaining_ms > 0 ? (uint32_t)(remaining_ms / 1000) + 1 : 0;
    } else {
        out->cooldown_remaining_sec = 0;
    }
```

- [ ] **Step 11: Build both targets**

Run: `.\scripts\build.ps1 -Target esp32` then `.\scripts\build.ps1 -Target esp32s3`
Expected: Both builds succeed; `fish_pump_relay_timer_control.bin` generated in each `build-<target>/`.

- [ ] **Step 12: Commit**

```powershell
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" add components/pump_control/pump_control.c
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" commit -m "feat(pump_control): add min_dwell cooldown gate after timer switch"
```

---

