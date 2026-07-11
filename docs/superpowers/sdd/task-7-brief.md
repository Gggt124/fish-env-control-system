### Task 7: index.html min_dwell_sec input field

**Files:**
- Modify: `main/static/index.html:660-679` (insert new field row before the settings-actions block)

**Interfaces:**
- Consumes: nothing.
- Produces: an `<input id="pump-min-dwell">` element inside the pump config form, with a sibling error element `pump-min-dwell-error`. Consumed by Task 8.

- [ ] **Step 1: Insert the min_dwell_sec field row**

In `main/static/index.html`, immediately before the `<div class="settings-actions">` block (line 661), insert a new field row that matches the existing form-row pattern used by other settings. Use Thai labels:

```html
                                <div class="form-row">
                                    <label for="pump-min-dwell">ระยะหน่วงสลับปั๊ม (Cooldown)
                                        <span lang="en">Min Dwell</span></label>
                                    <div class="field-with-unit">
                                        <input id="pump-min-dwell" type="number" min="0"
                                            max="3600" step="1" inputmode="numeric" value="30">
                                        <span class="field-unit">วินาที</span>
                                    </div>
                                    <small>ห้ามสลับ Timer กลับภายในระยะเวลานี้หลังสลับครั้งล่าสุด
                                        (0 = ปิด, ค่าเริ่มต้น 30 วินาที) — กันปั๊มสลับถี่จากน้ำกระเพื่อม</small>
                                    <div class="field-error" id="pump-min-dwell-error" role="alert"
                                        aria-live="polite"></div>
                                </div>
```

- [ ] **Step 2: Verify the file still parses (no build step for static)**

Run: open `main/static/index.html` and confirm the new block sits inside `<form id="pump-config-form">` and closes before `<div class="settings-actions">`. No compile step — static assets are embedded at build time.

- [ ] **Step 3: Commit**

```powershell
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" add main/static/index.html
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" commit -m "feat(ui): add min_dwell_sec input to pump config form"
```

---

