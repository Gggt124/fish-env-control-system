### Task 11: Final dual-target build and verification

**Files:**
- None modified.

**Interfaces:**
- Consumes: all prior tasks.
- Produces: a green dual-target build and a clean working tree, ready for manual hardware test.

- [ ] **Step 1: Clean rebuild both targets**

Run:
```powershell
.\scripts\build.ps1 -Target esp32
.\scripts\build.ps1 -Target esp32s3
```
Expected: Both succeed; `build-esp32\fish_pump_relay_timer_control.bin` and `build-esp32s3\fish_pump_relay_timer_control.bin` exist.

- [ ] **Step 2: Run the full Python UI test suite**

Run: `python -m pytest tests/ -v`
Expected: All tests pass (existing phase13/phase16 plus the new test_min_dwell).

- [ ] **Step 3: Confirm git status is clean**

Run: `git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" status --short`
Expected: empty (all changes committed).

- [ ] **Step 4: Manual hardware test plan (documented, executed by user)**

Flash the new firmware to an ESP32 board, connect to `FishPump-Setup`, log in, and verify on the dashboard:
1. The "ระยะหน่วงสลับปั๊ม (Min Dwell)" field shows 30 and is editable.
2. Save a config with min_dwell = 30; reload the page — value persists (NVS).
3. With the pump running, flick the float switch to force a timer switch, then flick it back within 30 s: the relay must NOT switch back. The dashboard should show the cooldown badge counting down.
4. Wait > 30 s, flick the float back: the relay switches.
5. Set min_dwell_sec = 0, save, flick the float rapidly: cooldown no longer blocks (legacy behavior, only debounce filters).

- [ ] **Step 5: Final commit (if any cleanup)**

Only if Step 3 found leftover changes. Otherwise this step is a no-op.

```powershell
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" status --short
```

---

## Self-Review

**1. Spec coverage:**
- Spec §6.1 app_config.h defaults + bounds → Task 1 ✓
- Spec §6.2 pump_control.h struct fields → Task 2 ✓
- Spec §6.3 pump_control.c cooldown logic, validation, default, update, status, reset, fault → Task 3 ✓
- Spec §6.4 web_server.c GET + POST + update_timers call → Task 5 ✓
- Spec §6.4 status JSON (cooldown_remaining_sec) → Task 6 ✓
- Spec §6.5 dashboard UI input + cooldown badge → Tasks 7, 8 ✓
- Spec §7 Edge cases (min_dwell=0, stop/start reset, update during cooldown, fault, bounds) → Task 3 Steps 3,4,6,8,9 ✓
- Spec §9 Testing (build + hardware simulator + manual) → Tasks 9, 10, 11 ✓

**2. Placeholder scan:** No TBD/TODO/"implement later"/"add validation" without code. Every code step shows the exact code. ✓

**3. Type consistency:** `min_dwell_sec` (uint32_t) used consistently across config/update/status structs (Task 2), pump_control.c (Task 3), nvs_store struct (Task 4), web_server (Tasks 5, 6), JS payload (Task 8), simulator (Task 9). `cooldown_remaining_sec` (uint32_t) consistent in status struct (Task 2), get_status (Task 3), status JSON (Task 6), JS render (Task 8), simulator (Task 9). `s_last_switch_ms` (int64_t) consistent in pump_control.c (Task 3). `pump_control_timer_update_t.debounce_ms` and `.min_dwell_sec` (uint32_t) match the config struct and the update_timers call initializer (Tasks 2, 3, 5). ✓

**Note on spec deviation:** The spec §6.4 assumed `debounce_ms` was already parsed at web_server.c line 1768; investigation found it is in a readonly rejection list. The plan keeps `debounce_ms` readonly at the web layer (default raised to 2000 via Task 1) and only makes `min_dwell_sec` web-editable + NVS-persisted, which satisfies the spec's intent ("Hardening must be runtime-configurable from the web dashboard") for the new primary defense (min_dwell). `debounce_ms` is still passed through `pump_control_timer_update_t` so the runtime API supports tuning it if a future task exposes it. This deviation is documented in Task 5 Step 4.
