### Task 8: app.js load/save + cooldown badge

**Files:**
- Modify: `main/static/app.js:715-743` (loadPumpConfig populate), `:833-851` (validatePumpConfig payload), `:945-991` (applyPumpStatus render)

**Interfaces:**
- Consumes: `min_dwell_sec` from GET `/api/pump/config` (Task 5); `cooldown_remaining_sec` from GET `/api/pump/status` (Task 6); DOM elements `pump-min-dwell`, `pump-min-dwell-error` from Task 7.
- Produces: form loads/displays min_dwell_sec, saves it in the POST payload, and shows a cooldown badge when `cooldown_remaining_sec > 0`.

- [ ] **Step 1: Populate min_dwell_sec on config load**

In `loadPumpConfig` (around line 733-740), after `setStartPhaseField('timer2-start-phase', data.timer2_start_phase);`, add:

```js
        setStartPhaseField('timer2-start-phase', data.timer2_start_phase);
        var minDwell = pumpEl('pump-min-dwell');
        if (minDwell) minDwell.value = (data.min_dwell_sec != null) ? data.min_dwell_sec : 30;
```

- [ ] **Step 2: Read + validate min_dwell_sec into the save payload**

In `validatePumpConfig` (around lines 833-851), before the `return { ... }` that builds the payload, add validation and include the field:

```js
    var minDwellEl = pumpEl('pump-min-dwell');
    var minDwellErr = pumpEl('pump-min-dwell-error');
    var minDwellRaw = minDwellEl ? minDwellEl.value.trim() : '';
    var minDwellVal = parseInt(minDwellRaw, 10);
    if (isNaN(minDwellVal) || minDwellVal < 0 || minDwellVal > 3600 ||
        String(minDwellVal) !== String(minDwellRaw)) {
        if (minDwellErr) minDwellErr.textContent = 'ค่าระยะหน่วงต้องเป็นตัวเลข 0-3600';
        if (minDwellEl) minDwellEl.classList.add('invalid');
        return null;
    }
    if (minDwellErr) minDwellErr.textContent = '';
    if (minDwellEl) minDwellEl.classList.remove('invalid');

    return {
        timer1_on_sec: timer1On.value,
        timer1_off_sec: timer1Off.value,
        timer2_on_sec: timer2On.value,
        timer2_off_sec: timer2Off.value,
        timer1_start_phase: timer1Start.value,
        timer2_start_phase: timer2Start.value,
        auto_start: !!(pumpEl('pump-auto-start') && pumpEl('pump-auto-start').checked),
        relay_polarity: pumpRelayPolarity,
        min_dwell_sec: minDwellVal
    };
```

- [ ] **Step 3: Render cooldown badge in applyPumpStatus**

In `applyPumpStatus` (around lines 945-983), after the `setHtml('pump-relay-2-state', ...)` line, add a cooldown badge update targeting `pump-sync-state`'s sibling or a dedicated element. Use the existing `pump-active-timer` badge area by appending text. Add after line 961:

```js
    setHtml('pump-relay-2-state', renderRelayChannelState('relay2', status));
    var cooldown = Number(status.cooldown_remaining_sec || 0);
    var cooldownEl = pumpEl('pump-cooldown-badge');
    if (cooldownEl) {
        if (cooldown > 0) {
            cooldownEl.textContent = 'หน่วงสลับอีก ' + cooldown + ' วิ';
            cooldownEl.classList.remove('hidden');
        } else {
            cooldownEl.classList.add('hidden');
            cooldownEl.textContent = '';
        }
    }
```

- [ ] **Step 4: Add the cooldown badge element to index.html**

In `main/static/index.html`, inside the `pump-runtime-panel` hero card (around lines 395-410), add a small badge next to the active timer badge. After the `<span class="hero-state-badge" id="pump-active-timer">...</span>` block (line 399), add:

```html
                                <span class="hero-state-badge warn hidden" id="pump-cooldown-badge"></span>
```

- [ ] **Step 5: Build both targets (embeds updated static assets)**

Run: `.\scripts\build.ps1 -Target esp32` then `.\scripts\build.ps1 -Target esp32s3`
Expected: Both builds succeed; the new static files are embedded.

- [ ] **Step 6: Commit**

```powershell
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" add main/static/app.js main/static/index.html
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" commit -m "feat(ui): load/save min_dwell_sec and show cooldown badge on dashboard"
```

---

