### Task 4: nvs_store min_dwell_sec persistence

**Files:**
- Modify: `components/nvs_store/nvs_store.h:22-33` (struct), `components/nvs_store/nvs_store.c:42` (blob version), `:80-89` (valid), `:690-711` (defaults)

**Interfaces:**
- Consumes: `APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_SEC`, `_MAX_SEC` from Task 1.
- Produces: `nvs_store_pump_settings_t` with a new trailing `min_dwell_sec` field; defaults set it to 30; validation enforces the 0..3600 range. Consumed by Task 5.

- [ ] **Step 1: Append min_dwell_sec to nvs_store_pump_settings_t**

In `components/nvs_store/nvs_store.h`, the struct (lines 22-33) currently ends with `bool auto_start;`. Append `min_dwell_sec` as the LAST field (critical for old-blob forward compatibility):

```c
typedef struct {
    uint32_t timer1_on_sec;
    uint32_t timer1_off_sec;
    uint32_t timer2_on_sec;
    uint32_t timer2_off_sec;
    bool relay_active_low;
    bool relay1_active_low;
    bool relay2_active_low;
    hardware_timer_start_phase_t timer1_start_phase;
    hardware_timer_start_phase_t timer2_start_phase;
    bool auto_start;
    uint32_t min_dwell_sec;
} nvs_store_pump_settings_t;
```

- [ ] **Step 2: Bump NVS_PUMP_BLOB_VERSION to 2**

In `components/nvs_store/nvs_store.c` line 42, change:

```c
#define NVS_PUMP_BLOB_VERSION       1   /* increment if struct layout changes */
```

to:

```c
#define NVS_PUMP_BLOB_VERSION       2   /* v2: added min_dwell_sec (append-only) */
```

Note: The load path (lines 740-754) already handles size mismatch — an old v1 blob has version byte `1`, so `blob[0] == NVS_PUMP_BLOB_VERSION` (now 2) is false for old blobs. That means old v1 blobs will hit the "Incompatible blob version" warning and fall through to defaults. To preserve upgrade-friendliness, instead accept BOTH v1 and v2 on load. Modify the version check in `nvs_store_load_pump_settings` (line 741):

```c
        if (blob_len > 0 && (blob[0] == NVS_PUMP_BLOB_VERSION || blob[0] == 1)) {
```

This lets a v1 blob load with the new field defaulting to 0 (memset), which `pump_settings_valid` will accept (0 is a valid min_dwell). On the next save it is re-serialized as v2.

- [ ] **Step 3: Set min_dwell_sec default in nvs_store_pump_settings_defaults**

In `nvs_store_pump_settings_defaults()` (lines 690-711), add after `out->auto_start = ...`:

```c
    out->auto_start = APP_TEMPLATE_PUMP_AUTO_START_DEFAULT;
    out->min_dwell_sec = APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_SEC;
}
```

- [ ] **Step 4: Add min_dwell_sec range check to pump_settings_valid**

In `pump_settings_valid()` (lines 80-89), add the range check:

```c
static bool pump_settings_valid(const nvs_store_pump_settings_t *settings)
{
    return settings &&
           pump_duration_valid(settings->timer1_on_sec) &&
           pump_duration_valid(settings->timer1_off_sec) &&
           pump_duration_valid(settings->timer2_on_sec) &&
           pump_duration_valid(settings->timer2_off_sec) &&
           hardware_map_timer_start_phase_valid(settings->timer1_start_phase) &&
           hardware_map_timer_start_phase_valid(settings->timer2_start_phase) &&
           settings->min_dwell_sec <= APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_MAX_SEC;
}
```

- [ ] **Step 5: Build both targets**

Run: `.\scripts\build.ps1 -Target esp32` then `.\scripts\build.ps1 -Target esp32s3`
Expected: Both builds succeed. If a v1 blob exists on a flashed device, the first load falls through to defaults (acceptable; user re-saves config to persist min_dwell).

- [ ] **Step 6: Commit**

```powershell
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" add components/nvs_store/nvs_store.h components/nvs_store/nvs_store.c
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" commit -m "feat(nvs_store): persist min_dwell_sec in pump settings blob (v2)"
```

---

