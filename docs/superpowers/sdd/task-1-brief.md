### Task 1: app_config.h defaults and bounds

**Files:**
- Modify: `components/app_config/app_config.h:103`

**Interfaces:**
- Produces: `APP_TEMPLATE_PUMP_FLOAT_DEBOUNCE_MS` (new value 2000), `APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_SEC` (30), `APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_MIN_SEC` (0), `APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_MAX_SEC` (3600). Consumed by Tasks 2, 3, 4, 5.

- [ ] **Step 1: Read current debounce defines**

Run: `git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" checkout -- components/app_config/app_config.h` (no-op if clean), then read lines around 100-105.

- [ ] **Step 2: Change debounce default and add min_dwell defines**

In `components/app_config/app_config.h`, replace the line:

```c
#define APP_TEMPLATE_PUMP_FLOAT_DEBOUNCE_MS 100
```

with:

```c
#define APP_TEMPLATE_PUMP_FLOAT_DEBOUNCE_MS 2000

#define APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_SEC      30
#define APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_MIN_SEC   0
#define APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_MAX_SEC  3600
```

- [ ] **Step 3: Build esp32 to verify header compiles**

Run: `.\scripts\build.ps1 -Target esp32` (run in `C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl`)
Expected: Build succeeds (no consumer changed yet — the new macros are unused but harmless).

- [ ] **Step 4: Commit**

```powershell
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" add components/app_config/app_config.h
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" commit -m "feat(app_config): raise debounce default to 2000ms, add min_dwell_sec bounds"
```

---

