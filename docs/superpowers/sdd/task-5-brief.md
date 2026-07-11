### Task 5: web_server.c config GET/POST + runtime config + update_timers call

**Files:**
- Modify: `main/web_server.c:877-903` (api_pump_settings_to_runtime_config), `:905-951` (api_pump_add_config_fields), `:1754-1799` (parse), `:3411-3427` (update_timers call)

**Interfaces:**
- Consumes: `nvs_store_pump_settings_t.min_dwell_sec` from Task 4; `pump_control_config_t.min_dwell_sec` and `pump_control_timer_update_t` from Task 2; bounds from Task 1.
- Produces: `min_dwell_sec` in GET `/api/pump/config` JSON; `min_dwell_sec` parsed from POST body and persisted; `min_dwell_sec` + `debounce_ms` passed to `pump_control_update_timers`.

- [ ] **Step 1: Wire min_dwell_sec into runtime config**

In `api_pump_settings_to_runtime_config()` (lines 877-903), after `config->timer2_start_phase = ...`, add:

```c
    config->timer2_start_phase =
        api_pump_start_phase_from_hardware(settings->timer2_start_phase);
    config->min_dwell_sec = settings->min_dwell_sec;
}
```

- [ ] **Step 2: Expose min_dwell_sec in GET config JSON**

In `api_pump_add_config_fields()` (lines 905-951), the GET response currently emits `defaults.debounce_ms` from `pump_control_default_config()`. After that line (line 930), add `min_dwell_sec` from the loaded settings (not defaults, so the persisted value is returned):

```c
    cJSON_AddNumberToObject(root, "debounce_ms", defaults.debounce_ms);
    cJSON_AddNumberToObject(root, "min_dwell_sec", settings->min_dwell_sec);
```

- [ ] **Step 3: Parse min_dwell_sec in POST config**

In `api_pump_parse_settings_payload()` (lines 1754-1799), `debounce_ms` is in the `readonly_fields[]` rejection list (line 1768). Leave `debounce_ms` in the readonly list (it stays default-only at the web layer per spec scope; the runtime can still be tuned via update_timers if needed in future). Add `min_dwell_sec` as an OPTIONAL parsed field (if omitted, keep the loaded value).

After the existing required-field checks (after the `api_pump_required_bool(... "auto_start" ...)` block around line 1792-1795), add:

```c
    const cJSON *min_dwell = cJSON_GetObjectItem(root, "min_dwell_sec");
    if (min_dwell) {
        if (!cJSON_IsNumber(min_dwell) || min_dwell->valuedouble < 0.0) {
            *error_code = "invalid_type";
            *error_message = "min_dwell_sec";
            return false;
        }
        uint32_t md = (uint32_t)min_dwell->valuedouble;
        if ((double)md != min_dwell->valuedouble ||
            md > APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_MAX_SEC) {
            *error_code = "min_dwell_out_of_range";
            *error_message = "min_dwell_sec";
            return false;
        }
        settings->min_dwell_sec = md;
    }
```

- [ ] **Step 4: Pass min_dwell_sec + debounce_ms into pump_control_update_timers**

In `handle_api_pump_config_post()` (lines 3416-3429), the soft-change branch builds a `pump_control_timer_update_t update` initializer. Add the two new fields:

```c
        pump_control_timer_update_t update = {
            .timer1             = config.timer1,
            .timer2             = config.timer2,
            .timer1_start_phase = config.timer1_start_phase,
            .timer2_start_phase = config.timer2_start_phase,
            .relay1_polarity    = config.relay1_polarity,
            .relay2_polarity    = config.relay2_polarity,
            .debounce_ms        = config.debounce_ms,
            .min_dwell_sec      = config.min_dwell_sec,
        };
```

Note: `config.debounce_ms` comes from `pump_control_default_config()` (via `api_pump_settings_to_runtime_config` which starts from defaults). Since `debounce_ms` is not editable via web, this passes the current default (2000ms) on every soft save — which is the intended behavior (the larger default is now always in effect).

- [ ] **Step 5: Build both targets**

Run: `.\scripts\build.ps1 -Target esp32` then `.\scripts\build.ps1 -Target esp32s3`
Expected: Both builds succeed.

- [ ] **Step 6: Commit**

```powershell
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" add main/web_server.c
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" commit -m "feat(web_server): expose min_dwell_sec in pump config GET/POST and pass to update_timers"
```

---

