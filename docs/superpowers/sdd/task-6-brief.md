### Task 6: web_server.c status JSON (cooldown_remaining_sec + min_dwell_sec)

**Files:**
- Modify: `main/web_server.c:1135-1189` (api_pump_add_status_fields_gen)

**Interfaces:**
- Consumes: `pump_control_status_t.min_dwell_sec` and `.cooldown_remaining_sec` from Task 3.
- Produces: `min_dwell_sec` and `cooldown_remaining_sec` numbers in GET `/api/pump/status` JSON. Consumed by Tasks 8, 9.

- [ ] **Step 1: Add the two fields to the status JSON generator**

In `api_pump_add_status_fields_gen()` (lines 1135-1189), after `json_gen_add_number(gen, "countdown_sec", ...)` (line 1166), add:

```c
    json_gen_add_number(gen, "countdown_sec", (double)(status.countdown_sec));
    json_gen_add_number(gen, "min_dwell_sec", (double)(status.min_dwell_sec));
    json_gen_add_number(gen, "cooldown_remaining_sec", (double)(status.cooldown_remaining_sec));
```

- [ ] **Step 2: Build both targets**

Run: `.\scripts\build.ps1 -Target esp32` then `.\scripts\build.ps1 -Target esp32s3`
Expected: Both builds succeed.

- [ ] **Step 3: Commit**

```powershell
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" add main/web_server.c
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" commit -m "feat(web_server): report min_dwell_sec and cooldown_remaining_sec in pump status"
```

---

