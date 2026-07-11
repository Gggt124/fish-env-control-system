### Task 9: hardware_simulator.py mock responses

**Files:**
- Modify: `hardware_simulator.py:60` (state), and the GET/POST `/api/pump/config` and `/api/pump/status` handlers.

**Interfaces:**
- Consumes: the mock server's own state dict.
- Produces: `min_dwell_sec` in config GET/POST responses and `cooldown_remaining_sec` in status responses, so the UI can be exercised against the simulator without real hardware.

- [ ] **Step 1: Add min_dwell_sec to simulator state**

In `hardware_simulator.py`, in the `state` dict (after `"debounce_ms": 100,` around line 60), add:

```python
    "debounce_ms": 100,
    "min_dwell_sec": 30,
    "cooldown_remaining_sec": 0,
```

(Also update `"debounce_ms": 100,` to `"debounce_ms": 2000,` to mirror the new firmware default.)

- [ ] **Step 2: Include the fields in the config GET response**

Find the handler that returns the pump config JSON (search for `"debounce_ms"` in `hardware_simulator.py`) and add `"min_dwell_sec": state["min_dwell_sec"],` next to the existing `debounce_ms` field.

- [ ] **Step 3: Parse min_dwell_sec in the config POST handler**

Find the POST `/api/pump/config` handler, add `min_dwell_sec` to the parsed body fields with the same 0..3600 range check the firmware uses, and store it in `state["min_dwell_sec"]`.

- [ ] **Step 4: Include cooldown_remaining_sec in the status response**

Find the `/api/pump/status` handler and add `"cooldown_remaining_sec": state["cooldown_remaining_sec"],` to the JSON. (The simulator does not run the real cooldown timer; leave it at 0 so the badge stays hidden by default, which is correct for the no-splash simulation.)

- [ ] **Step 5: Smoke-test the simulator**

Run: `python hardware_simulator.py` (with a display or headless as appropriate), then from another shell curl the endpoints:

```powershell
curl -s http://127.0.0.1:8000/api/pump/config -H "Cookie: session=<token>" | python -m json.tool
curl -s http://127.0.0.1:8000/api/pump/status -H "Cookie: session=<token>" | python -m json.tool
```

Expected: `min_dwell_sec` appears in the config response (30); `cooldown_remaining_sec` appears in status (0).

- [ ] **Step 6: Commit**

```powershell
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" add hardware_simulator.py
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" commit -m "feat(simulator): mock min_dwell_sec and cooldown_remaining_sec in pump APIs"
```

---

