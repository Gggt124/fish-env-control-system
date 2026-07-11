### Task 2: pump_control.h struct fields

**Files:**
- Modify: `components/pump_control/pump_control.h:50-73` (update struct), `:75-94` (status struct)

**Interfaces:**
- Consumes: `uint32_t` types from `<stdint.h>` (already included).
- Produces: `min_dwell_sec` field in `pump_control_config_t` and `pump_control_timer_update_t`; `min_dwell_sec` + `cooldown_remaining_sec` in `pump_control_status_t`. Consumed by Tasks 3, 5, 6.

- [ ] **Step 1: Add min_dwell_sec to pump_control_config_t**

In `components/pump_control/pump_control.h`, inside `typedef struct { ... } pump_control_config_t;` (lines 59-73), add `min_dwell_sec` after the existing `debounce_ms` field:

```c
    uint32_t debounce_ms;
    uint32_t min_dwell_sec;
    pump_control_timer_config_t timer1;
```

- [ ] **Step 2: Add debounce_ms and min_dwell_sec to pump_control_timer_update_t**

In `pump_control_timer_update_t` (lines 50-57), add the two fields after `relay2_polarity`:

```c
    pump_control_relay_polarity_t relay2_polarity;
    uint32_t debounce_ms;
    uint32_t min_dwell_sec;
} pump_control_timer_update_t;
```

- [ ] **Step 3: Add min_dwell_sec and cooldown_remaining_sec to pump_control_status_t**

In `pump_control_status_t` (lines 75-94), add after `total_duration_sec`:

```c
    uint32_t total_duration_sec;
    uint32_t min_dwell_sec;
    uint32_t cooldown_remaining_sec;
} pump_control_status_t;
```

- [ ] **Step 4: Build esp32 to verify header compiles**

Run: `.\scripts\build.ps1 -Target esp32`
Expected: Build fails or succeeds depending on whether consumers reference the new fields yet. The struct additions alone are ABI-affecting but the C source does not yet set them, so initializers using designated field names will leave them as 0. Build should still succeed. If it fails on an initializer that uses positional args, fix by adding `.min_dwell_sec = 0` to that initializer.

- [ ] **Step 5: Commit**

```powershell
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" add components/pump_control/pump_control.h
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" commit -m "feat(pump_control): add min_dwell_sec and cooldown fields to config/update/status structs"
```

---

