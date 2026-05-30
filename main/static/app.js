/*
 * ESP32 Wi-Fi Setup/Web Server Template - Application JavaScript
 * Vanilla JS, no frameworks. Compact for ESP32 flash.
 */

/* ======== Session / Auth ======== */

function getSessionToken() {
    var c = document.cookie.match(/session=([^;]+)/);
    return c ? c[1] : null;
}

function isAuthenticated() {
    return !!getSessionToken();
}

function checkAuth() {
    /* Skip auth check on login page itself */
    if (window.location.pathname === '/login') return;
    if (!isAuthenticated()) {
        window.location.href = '/login';
    }
}

/* ======== API Helpers ======== */

var API_TIMEOUT_MS = 8000;

function apiGet(url, cb) {
    var xhr = new XMLHttpRequest();
    var done = false;
    function finish(err, data) {
        if (done) return;
        done = true;
        cb(err, data);
    }
    xhr.open('GET', url, true);
    xhr.timeout = API_TIMEOUT_MS;
    xhr.onload = function() {
        if (xhr.status === 200) {
            try { finish(null, JSON.parse(xhr.responseText)); }
            catch(e) { finish(e, null); }
        } else {
            finish(new Error('HTTP ' + xhr.status), null);
        }
    };
    xhr.onerror = function() { finish(new Error('Network error'), null); };
    xhr.ontimeout = function() { finish(new Error('Request timeout'), null); };
    xhr.onabort = function() { finish(new Error('Request aborted'), null); };
    xhr.send();
}

function apiPost(url, body, cb) {
    var xhr = new XMLHttpRequest();
    var done = false;
    function finish(err, data) {
        if (done) return;
        done = true;
        cb(err, data);
    }
    xhr.open('POST', url, true);
    xhr.timeout = API_TIMEOUT_MS;
    xhr.setRequestHeader('Content-Type', 'application/json');
    xhr.onload = function() {
        try {
            var parsed = JSON.parse(xhr.responseText);
            finish(null, parsed);
        } catch(e) {
            if (xhr.status === 200) {
                finish(e, null);
            } else {
                finish(new Error('HTTP ' + xhr.status), null);
            }
        }
    };
    xhr.onerror = function() { finish(new Error('Network error'), null); };
    xhr.ontimeout = function() { finish(new Error('Request timeout'), null); };
    xhr.onabort = function() { finish(new Error('Request aborted'), null); };
    xhr.send(JSON.stringify(body));
}

/* ======== Toast Notifications ======== */

function showToast(msg, type) {
    type = type || 'info';
    var el = document.createElement('div');
    el.className = 'toast ' + type;
    el.textContent = msg;
    document.body.appendChild(el);
    setTimeout(function() {
        el.style.opacity = '0';
        el.style.transition = 'opacity 0.3s';
        setTimeout(function() { if (el.parentNode) el.parentNode.removeChild(el); }, 300);
    }, 3000);
}

/* ======== Login Page ======== */

function initLogin() {
    if (window.location.pathname !== '/login') return;

    var form = document.getElementById('login-form');
    var btn = document.getElementById('login-btn');
    var errEl = document.getElementById('login-error');

    form.onsubmit = function(e) {
        e.preventDefault();
        var username = document.getElementById('username').value.trim();
        var password = document.getElementById('password').value;

        if (!username || !password) {
            errEl.style.display = 'block';
            errEl.textContent = '\u0e01\u0e23\u0e38\u0e13\u0e32\u0e01\u0e23\u0e2d\u0e01\u0e0a\u0e37\u0e48\u0e2d\u0e1c\u0e39\u0e49\u0e43\u0e0a\u0e49\u0e41\u0e25\u0e30\u0e23\u0e2b\u0e31\u0e2a\u0e1c\u0e48\u0e32\u0e19';
            return;
        }

        btn.disabled = true;
        btn.textContent = '\u0e01\u0e33\u0e25\u0e31\u0e07\u0e40\u0e02\u0e49\u0e32\u0e2a\u0e39\u0e48\u0e23\u0e30\u0e1a\u0e1a...';
        errEl.style.display = 'none';

        apiPost('/api/login', { username: username, password: password }, function(err, data) {
            btn.disabled = false;
            btn.textContent = '\u0e40\u0e02\u0e49\u0e32\u0e2a\u0e39\u0e48\u0e23\u0e30\u0e1a\u0e1a';

            if (err || !data || !data.ok) {
                errEl.style.display = 'block';
                if (data && data.error === 'rate_limited' && data.retry_after) {
                    errEl.textContent = '\u0e1e\u0e22\u0e32\u0e22\u0e32\u0e21\u0e25\u0e47\u0e2d\u0e01\u0e2d\u0e34\u0e19\u0e21\u0e32\u0e01\u0e40\u0e01\u0e34\u0e19\u0e44\u0e1b \u0e01\u0e23\u0e38\u0e13\u0e32\u0e23\u0e2d ' + data.retry_after + ' \u0e27\u0e34\u0e19\u0e32\u0e17\u0e35\u0e41\u0e25\u0e49\u0e27\u0e25\u0e2d\u0e07\u0e43\u0e2b\u0e21\u0e48';
                } else if (data && data.error === 'invalid_credentials') {
                    errEl.textContent = '\u0e0a\u0e37\u0e48\u0e2d\u0e1c\u0e39\u0e49\u0e43\u0e0a\u0e49\u0e2b\u0e23\u0e37\u0e2d\u0e23\u0e2b\u0e31\u0e2a\u0e1c\u0e48\u0e32\u0e19\u0e44\u0e21\u0e48\u0e16\u0e39\u0e01\u0e15\u0e49\u0e2d\u0e07';
                } else {
                    errEl.textContent = '\u0e40\u0e01\u0e34\u0e14\u0e02\u0e49\u0e2d\u0e1c\u0e34\u0e14\u0e1e\u0e25\u0e32\u0e14 \u0e01\u0e23\u0e38\u0e13\u0e32\u0e25\u0e2d\u0e07\u0e43\u0e2b\u0e21\u0e48';
                }
                return;
            }

            window.location.href = '/dashboard';
        });
    };
}

/* ======== Logout ======== */

function doLogout() {
    apiPost('/api/logout', {}, function() {
        document.cookie = 'session=; Path=/; Max-Age=0';
        window.location.href = '/login';
    });
}

/* ======== Dashboard Page ======== */

var pumpConfig = null;
var pumpRelayPolarity = null;
var pumpConfigLoaded = false;
var pumpEditVersion = 0;
var pumpDirty = false;
var pumpPending = false;
var pumpLastStatus = null;
var pumpLastSyncMs = 0;
var pumpCountdownDeadlineMs = null;
var pumpCountdownStatusKey = '';
var pumpDisplayedCountdownSec = null;
var pumpStatusRequestInFlight = false;
var pumpAwaitingRolloverSync = false;
var pumpPollTimer = null;
var pumpTickTimer = null;
var PUMP_STATUS_POLL_MS = 750;
var PUMP_COUNTDOWN_TICK_MS = 250;
var PUMP_STALE_MS = 5000;
var PUMP_DEADLINE_DRIFT_MS = 1500;
var coolingConfig = null;
var coolingConfigLoaded = false;
var coolingDirty = false;
var coolingPending = false;
var coolingLastStatus = null;
var coolingLastSyncMs = 0;
var coolingStatusRequestInFlight = false;
var coolingPollTimer = null;
var COOLING_STATUS_POLL_MS = 2000;
var COOLING_STALE_MS = 8000;
var statusSummaryRequestInFlight = false;
var statusFullRequestInFlight = false;
var hardwareMapData = null;
var hardwareDirty = false;
var hardwarePending = false;
var HARDWARE_FIELDS = [
    { key: 'float_input_gpio', select: 'hardware-float-input-gpio', wire: 'wire-float-input', label: 'Float Input' },
    { key: 'pump_relay1_gpio', select: 'hardware-pump-relay1-gpio', wire: 'wire-pump-relay1', label: 'Pump Relay 1' },
    { key: 'pump_relay2_gpio', select: 'hardware-pump-relay2-gpio', wire: 'wire-pump-relay2', label: 'Pump Relay 2' },
    { key: 'ds18b20_gpio', select: 'hardware-ds18b20-gpio', wire: 'wire-ds18b20', label: 'DS18B20 Sensor' },
    { key: 'cooling_relay_gpio', select: 'hardware-cooling-relay-gpio', wire: 'wire-cooling-relay', label: 'Cooling Relay' }
];

function initDashboard() {
    initPumpDashboard();
    initCoolingDashboard();
}

function initPumpDashboard() {
    if (window.location.pathname !== '/dashboard') return;
    wirePumpForm();
    wirePumpActions();
    updatePumpConfigSaveButton(false);
    updatePumpButtons();
    loadPumpConfig();
    syncPumpStatus(true);
    refreshStatus();
    setInterval(refreshStatus, 10000);
    startPumpLiveTimers();
    document.addEventListener('visibilitychange', handlePumpVisibilityChange);
}

function pumpEl(id) {
    return document.getElementById(id);
}

function initCoolingDashboard() {
    if (window.location.pathname !== '/dashboard') return;
    wireCoolingForm();
    wireCoolingActions();
    updateCoolingButtons();
    loadCoolingConfig();
    syncCoolingStatus(true);
    startCoolingStatusTimer();
    document.addEventListener('visibilitychange', handleCoolingVisibilityChange);
}

function setPumpAlert(id, message) {
    var el = pumpEl(id);
    if (!el) return;
    if (message) {
        el.textContent = message;
        el.classList.remove('hidden');
    } else {
        el.textContent = '';
        el.classList.add('hidden');
    }
}

function setCoolingAlert(message) {
    setPumpAlert('cooling-config-error', message);
}

function wirePumpForm() {
    var form = pumpEl('pump-config-form');
    if (form) {
        form.onsubmit = function(e) {
            e.preventDefault();
            savePumpConfig();
        };
    }

    var ids = [
        'timer1-on-min', 'timer1-on-sec', 'timer1-off-min', 'timer1-off-sec',
        'timer2-on-min', 'timer2-on-sec', 'timer2-off-min', 'timer2-off-sec',
        'timer1-start-phase', 'timer2-start-phase',
        'pump-auto-start'
    ];
    for (var i = 0; i < ids.length; i++) {
        var input = pumpEl(ids[i]);
        if (!input) continue;
        input.oninput = markPumpDirty;
        input.onchange = markPumpDirty;
    }
}

function wirePumpActions() {
    var startBtn = pumpEl('pump-start-btn');
    var stopBtn = pumpEl('pump-stop-btn');
    if (startBtn) startBtn.onclick = startPump;
    if (stopBtn) stopBtn.onclick = stopPump;
}

function markPumpDirty() {
    pumpEditVersion += 1;
    pumpDirty = true;
    var warning = pumpEl('pump-unsaved-warning');
    if (warning) warning.classList.remove('hidden');
    setText('pump-config-state', 'มีการแก้ไขที่ยังไม่ได้บันทึก');
    updatePumpConfigSaveButton(true);
}

function setPumpClean(label) {
    pumpDirty = false;
    var warning = pumpEl('pump-unsaved-warning');
    if (warning) warning.classList.add('hidden');
    setText('pump-config-state', label || 'บันทึกแล้ว');
    updatePumpConfigSaveButton(true);
}

function updatePumpConfigSaveButton(enabled) {
    var saveBtn = pumpEl('pump-save-config');
    if (saveBtn) saveBtn.disabled = !enabled || !pumpConfigLoaded || !pumpRelayPolarity;
}

function loadPumpConfig() {
    var requestEditVersion = pumpEditVersion;
    setText('pump-config-state', 'กำลังโหลดค่า...');
    setPumpAlert('pump-config-error', '');
    updatePumpConfigSaveButton(false);
    apiGet('/api/pump/config', function(err, data) {
        if (err || !data || !data.ok) {
            pumpConfigLoaded = false;
            setText('pump-config-state', 'โหลดค่าไม่สำเร็จ');
            setPumpAlert('pump-config-error', 'โหลดค่าตั้งเวลาไม่สำเร็จ กรุณาโหลดค่าให้สำเร็จก่อนบันทึก');
            updatePumpConfigSaveButton(false);
            return;
        }
        pumpConfig = data;
        pumpRelayPolarity = data.relay_polarity || null;
        pumpConfigLoaded = !!pumpRelayPolarity;
        if (pumpEditVersion !== requestEditVersion) {
            setText('pump-config-state', 'มีการแก้ไขที่ยังไม่ได้บันทึก');
            updatePumpConfigSaveButton(pumpConfigLoaded);
            return;
        }
        setDurationFields('timer1-on', data.timer1_on_sec);
        setDurationFields('timer1-off', data.timer1_off_sec);
        setDurationFields('timer2-on', data.timer2_on_sec);
        setDurationFields('timer2-off', data.timer2_off_sec);
        setStartPhaseField('timer1-start-phase', data.timer1_start_phase);
        setStartPhaseField('timer2-start-phase', data.timer2_start_phase);
        var autoStart = pumpEl('pump-auto-start');
        if (autoStart) autoStart.checked = !!data.auto_start;
        clearPumpValidation();
        setPumpClean('โหลดค่าแล้ว');
    });
}

function setDurationFields(prefix, totalSec) {
    totalSec = Number(totalSec || 0);
    var minEl = pumpEl(prefix + '-min');
    var secEl = pumpEl(prefix + '-sec');
    if (minEl) minEl.value = Math.floor(totalSec / 60);
    if (secEl) secEl.value = totalSec % 60;
}

function setStartPhaseField(id, value) {
    var el = pumpEl(id);
    if (!el) return;
    el.value = value === 'off' ? 'off' : 'on';
}

function readDuration(prefix, label) {
    var minEl = pumpEl(prefix + '-min');
    var secEl = pumpEl(prefix + '-sec');
    var errEl = pumpEl(prefix + '-error');
    var rawMin = minEl ? minEl.value.trim() : '';
    var rawSec = secEl ? secEl.value.trim() : '';
    var fail = function(message) {
        if (errEl) errEl.textContent = message;
        if (minEl) minEl.classList.add('invalid');
        if (secEl) secEl.classList.add('invalid');
        return { ok: false, value: 0 };
    };

    if (rawMin === '' || rawSec === '') {
        return fail(label + ': กรุณากรอกนาทีและวินาที');
    }
    if (!/^\d+$/.test(rawMin) || !/^\d+$/.test(rawSec)) {
        return fail(label + ': ต้องเป็นจำนวนเต็ม 0 หรือมากกว่า');
    }

    var minutes = parseInt(rawMin, 10);
    var seconds = parseInt(rawSec, 10);
    if (seconds > 59) {
        return fail(label + ': วินาทีต้องอยู่ระหว่าง 0..59');
    }

    var total = (minutes * 60) + seconds;
    if (total < 5 || total > 86400) {
        return fail(label + ': ระยะเวลาต้องอยู่ระหว่าง 5..86400 วินาที');
    }

    if (errEl) errEl.textContent = '';
    if (minEl) minEl.classList.remove('invalid');
    if (secEl) secEl.classList.remove('invalid');
    return { ok: true, value: total };
}

function readStartPhase(id, label) {
    var el = pumpEl(id);
    var errEl = pumpEl(id + '-error');
    var value = el ? el.value : '';
    if (value !== 'on' && value !== 'off') {
        if (errEl) errEl.textContent = label + ': เลือก ON หรือ OFF';
        if (el) el.classList.add('invalid');
        return { ok: false, value: 'on' };
    }
    if (errEl) errEl.textContent = '';
    if (el) el.classList.remove('invalid');
    return { ok: true, value: value };
}

function clearPumpValidation() {
    var ids = ['timer1-on', 'timer1-off', 'timer2-on', 'timer2-off'];
    for (var i = 0; i < ids.length; i++) {
        var errEl = pumpEl(ids[i] + '-error');
        var minEl = pumpEl(ids[i] + '-min');
        var secEl = pumpEl(ids[i] + '-sec');
        if (errEl) errEl.textContent = '';
        if (minEl) minEl.classList.remove('invalid');
        if (secEl) secEl.classList.remove('invalid');
    }
    var startIds = ['timer1-start-phase', 'timer2-start-phase'];
    for (var j = 0; j < startIds.length; j++) {
        var startEl = pumpEl(startIds[j]);
        var startErr = pumpEl(startIds[j] + '-error');
        if (startErr) startErr.textContent = '';
        if (startEl) startEl.classList.remove('invalid');
    }
}

function validatePumpConfig() {
    clearPumpValidation();
    var timer1On = readDuration('timer1-on', 'Timer 1 ช่วงเปิด');
    var timer1Off = readDuration('timer1-off', 'Timer 1 ช่วงปิด');
    var timer2On = readDuration('timer2-on', 'Timer 2 ช่วงเปิด');
    var timer2Off = readDuration('timer2-off', 'Timer 2 ช่วงปิด');
    var timer1Start = readStartPhase('timer1-start-phase', 'Timer 1 เริ่มรอบ');
    var timer2Start = readStartPhase('timer2-start-phase', 'Timer 2 เริ่มรอบ');
    if (!timer1On.ok || !timer1Off.ok || !timer2On.ok || !timer2Off.ok ||
        !timer1Start.ok || !timer2Start.ok) {
        return null;
    }
    return {
        timer1_on_sec: timer1On.value,
        timer1_off_sec: timer1Off.value,
        timer2_on_sec: timer2On.value,
        timer2_off_sec: timer2Off.value,
        timer1_start_phase: timer1Start.value,
        timer2_start_phase: timer2Start.value,
        auto_start: !!(pumpEl('pump-auto-start') && pumpEl('pump-auto-start').checked),
        relay_polarity: pumpRelayPolarity
    };
}

function savePumpConfig() {
    var saveBtn = pumpEl('pump-save-config');
    if (!pumpConfigLoaded || !pumpRelayPolarity) {
        setText('pump-config-state', 'ยังบันทึกไม่ได้');
        setPumpAlert('pump-config-error', 'กรุณาโหลดค่าจากอุปกรณ์ให้สำเร็จก่อนบันทึก');
        updatePumpConfigSaveButton(false);
        return;
    }

    var requestEditVersion = pumpEditVersion;
    var payload = validatePumpConfig();
    if (!payload) {
        setPumpAlert('pump-config-error', 'ตรวจสอบค่าตั้งเวลาแล้วลองบันทึกอีกครั้ง');
        return;
    }

    setPumpAlert('pump-config-error', '');
    if (saveBtn) {
        saveBtn.disabled = true;
        saveBtn.textContent = 'กำลังบันทึก...';
    }
    setText('pump-config-state', 'กำลังบันทึก...');

    apiPost('/api/pump/config', payload, function(err, data) {
        if (saveBtn) {
            saveBtn.disabled = false;
            saveBtn.textContent = 'บันทึกค่า';
        }
        if (err || !data || !data.ok) {
            setText('pump-config-state', 'บันทึกไม่สำเร็จ');
            setPumpAlert('pump-config-error', (data && data.message) ? data.message : 'บันทึกค่าตั้งเวลาไม่สำเร็จ');
            updatePumpConfigSaveButton(true);
            return;
        }
        pumpConfig = data.config || data;
        if (pumpConfig.relay_polarity) pumpRelayPolarity = pumpConfig.relay_polarity;
        pumpConfigLoaded = !!pumpRelayPolarity;
        if (pumpEditVersion === requestEditVersion) {
            setPumpClean('บันทึกแล้ว');
        } else {
            setText('pump-config-state', 'มีการแก้ไขที่ยังไม่ได้บันทึก');
            updatePumpConfigSaveButton(pumpConfigLoaded);
        }
        showToast('บันทึกค่าปั๊มเรียบร้อย', 'success');
        if (data.status) applyPumpStatus(data.status, true);
        syncPumpStatus(true);
    });
}

function startPump() {
    requestPumpAction('/api/pump/start', 'กำลังสั่ง Start...', 'เริ่มระบบปั๊มแล้ว');
}

function stopPump() {
    requestPumpAction('/api/pump/stop', 'กำลังสั่ง Stop...', 'หยุดระบบปั๊มแล้ว');
}

function requestPumpAction(url, pendingText, successText) {
    if (pumpPending) return;
    pumpPending = true;
    setText('pump-action-state', pendingText);
    updatePumpButtons();
    apiPost(url, {}, function(err, data) {
        pumpPending = false;
        if (err || !data || !data.ok) {
            setText('pump-action-state', '');
            setPumpAlert('pump-status-error', (data && data.message) ? data.message : 'สั่งงานปั๊มไม่สำเร็จ');
            updatePumpButtons();
            showToast('สั่งงานปั๊มไม่สำเร็จ', 'error');
            return;
        }
        if (data.status) applyPumpStatus(data.status, true);
        setText('pump-action-state', data.already_running ? 'ระบบทำงานอยู่แล้ว' : (data.already_stopped ? 'ระบบหยุดอยู่แล้ว' : ''));
        setPumpAlert('pump-status-error', '');
        updatePumpButtons();
        showToast(successText, 'success');
        syncPumpStatus(true);
    });
}

function syncPumpStatus(force) {
    if (window.location.pathname !== '/dashboard') return;
    if (document.hidden && !force) return;
    if (pumpStatusRequestInFlight) return;
    pumpStatusRequestInFlight = true;
    apiGet('/api/pump/status', function(err, data) {
        pumpStatusRequestInFlight = false;
        if (err || !data || !data.ok) {
            handlePumpStatusFailure();
            return;
        }
        applyPumpStatus(data, true);
    });
}

function applyPumpStatus(status, authoritative) {
    pumpLastStatus = status;
    if (authoritative) {
        pumpLastSyncMs = Date.now();
        pumpAwaitingRolloverSync = false;
        updatePumpCountdownDeadline(status);
    }

    setText('pump-running-label', renderPumpRunLabel(status));
    renderPumpCountdown();
    setText('pump-active-timer', renderPumpTimer(status.active_timer));
    setText('pump-phase', renderPumpPhase(status.active_timer, status.phase, status.running));
    setText('pump-float-state', renderFloatState(status.float_state));
    setText('pump-relay-state', renderActiveRelayState(status));
    setText('pump-relay-1-state', renderRelayChannelState('relay1', status));
    setText('pump-relay-2-state', renderRelayChannelState('relay2', status));

    var sync = pumpEl('pump-sync-state');
    if (sync) {
        sync.className = status.fault ? 'runtime-eyebrow error' :
            (status.initial_stabilizing ? 'runtime-eyebrow warn' : 'runtime-eyebrow good');
        sync.textContent = status.fault ? 'ระบบ relay fault: ปิด relay ทั้งหมดแล้ว' :
            (status.initial_stabilizing ? 'กำลังรอลูกลอยนิ่ง' : 'ซิงก์ล่าสุดจากอุปกรณ์');
    }
    if (!pumpDirty && status.settings_status) {
        setText('pump-config-state', renderSettingsStatus(status.settings_status, status.auto_start));
    }
    setPumpAlert('pump-status-error', status.fault ? 'พบ relay fault ระบบบังคับปิด Relay 1 และ Relay 2 แล้ว' :
        (status.config_valid === false ? 'ค่าตั้งเวลายังไม่พร้อมใช้งาน' : ''));
    updatePumpButtons();
}

function handlePumpStatusFailure() {
    var now = Date.now();
    var stale = !pumpLastSyncMs || (now - pumpLastSyncMs > PUMP_STALE_MS);
    var sync = pumpEl('pump-sync-state');
    if (sync) {
        sync.className = stale ? 'runtime-eyebrow error' : 'runtime-eyebrow warn';
        sync.textContent = stale ? 'สถานะปั๊มขาดการซิงก์' : 'รอซิงก์จากอุปกรณ์';
    }
    if (stale) {
        setPumpAlert('pump-status-error', 'ไม่สามารถอ่านสถานะปั๊มได้ชั่วคราว ปิดปุ่ม Start/Stop จนกว่าจะซิงก์สำเร็จ');
    }
    updatePumpButtons();
}

function updatePumpButtons() {
    var startBtn = pumpEl('pump-start-btn');
    var stopBtn = pumpEl('pump-stop-btn');
    var stale = !pumpLastSyncMs || (Date.now() - pumpLastSyncMs > PUMP_STALE_MS);
    var disabled = pumpPending || stale || !pumpLastStatus || pumpLastStatus.config_valid === false;
    if (startBtn) startBtn.disabled = disabled || !!(pumpLastStatus && pumpLastStatus.running);
    if (stopBtn) stopBtn.disabled = disabled || !(pumpLastStatus && pumpLastStatus.running);
}

function startPumpLiveTimers() {
    stopPumpLiveTimers();
    if (document.hidden) return;
    pumpPollTimer = setInterval(function() { syncPumpStatus(false); }, PUMP_STATUS_POLL_MS);
    pumpTickTimer = setInterval(tickPumpCountdown, PUMP_COUNTDOWN_TICK_MS);
}

function stopPumpLiveTimers() {
    if (pumpPollTimer) clearInterval(pumpPollTimer);
    if (pumpTickTimer) clearInterval(pumpTickTimer);
    pumpPollTimer = null;
    pumpTickTimer = null;
}

function handlePumpVisibilityChange() {
    if (document.hidden) {
        stopPumpLiveTimers();
    } else {
        startPumpLiveTimers();
        syncPumpStatus(true);
    }
}

function tickPumpCountdown() {
    if (!pumpLastStatus) return;
    if (pumpLastStatus.running) {
        if (getPumpCountdownSec() <= 0 && !predictPumpPhaseAdvance()) {
            requestPumpRolloverSync();
        }
        renderPumpCountdown();
    }
    if (pumpLastSyncMs && Date.now() - pumpLastSyncMs > PUMP_STALE_MS) {
        handlePumpStatusFailure();
    }
}

function pumpStatusKey(status) {
    if (!status) return '';
    return [
        status.running ? '1' : '0',
        status.initial_stabilizing ? '1' : '0',
        status.active_timer || '',
        status.active_relay || '',
        status.phase || ''
    ].join('|');
}

function updatePumpCountdownDeadline(status) {
    var countdown = Number(status && status.countdown_sec || 0);
    var key = pumpStatusKey(status);
    if (!status || !status.running || status.phase === 'idle' || countdown <= 0) {
        pumpCountdownDeadlineMs = null;
        pumpCountdownStatusKey = key;
        pumpDisplayedCountdownSec = countdown;
        return;
    }

    var nextDeadline = Date.now() + (countdown * 1000);
    var shouldReset = !pumpCountdownDeadlineMs ||
        pumpCountdownStatusKey !== key ||
        Math.abs(nextDeadline - pumpCountdownDeadlineMs) > PUMP_DEADLINE_DRIFT_MS;

    if (shouldReset) {
        pumpCountdownDeadlineMs = nextDeadline;
    }
    pumpCountdownStatusKey = key;
    pumpDisplayedCountdownSec = getPumpCountdownSec();
}

function getPumpCountdownSec() {
    if (!pumpCountdownDeadlineMs) {
        return pumpDisplayedCountdownSec;
    }
    return Math.max(0, Math.ceil((pumpCountdownDeadlineMs - Date.now()) / 1000));
}

function renderPumpCountdown() {
    pumpDisplayedCountdownSec = getPumpCountdownSec();
    setText('pump-countdown', formatPumpCountdown(pumpDisplayedCountdownSec));
}

function getPumpDurationSec(timer, phase) {
    if (!pumpConfig || (timer !== 'timer1' && timer !== 'timer2')) return null;
    if (phase !== 'on' && phase !== 'off') return null;
    var value = Number(pumpConfig[timer + '_' + phase + '_sec']);
    if (!isFinite(value) || value <= 0) return null;
    return value;
}

function predictPumpPhaseAdvance() {
    if (!pumpLastStatus || !pumpLastStatus.running || pumpLastStatus.initial_stabilizing) return false;
    if (pumpLastStatus.phase !== 'on' && pumpLastStatus.phase !== 'off') return false;

    var nextPhase = pumpLastStatus.phase === 'on' ? 'off' : 'on';
    var nextDuration = getPumpDurationSec(pumpLastStatus.active_timer, nextPhase);
    if (!nextDuration) return false;

    pumpLastStatus.phase = nextPhase;
    pumpLastStatus.countdown_sec = nextDuration;
    pumpLastStatus.relay_energized = nextPhase === 'on';
    if (pumpLastStatus.active_relay === 'relay1') {
        pumpLastStatus.relay1_energized = nextPhase === 'on';
        pumpLastStatus.relay2_energized = false;
    } else if (pumpLastStatus.active_relay === 'relay2') {
        pumpLastStatus.relay1_energized = false;
        pumpLastStatus.relay2_energized = nextPhase === 'on';
    }
    pumpCountdownStatusKey = pumpStatusKey(pumpLastStatus);
    pumpCountdownDeadlineMs = Date.now() + (nextDuration * 1000);
    pumpDisplayedCountdownSec = nextDuration;

    setText('pump-phase', renderPumpPhase(pumpLastStatus.active_timer, pumpLastStatus.phase, pumpLastStatus.running));
    setText('pump-relay-state', renderActiveRelayState(pumpLastStatus));
    setText('pump-relay-1-state', renderRelayChannelState('relay1', pumpLastStatus));
    setText('pump-relay-2-state', renderRelayChannelState('relay2', pumpLastStatus));
    return true;
}

function requestPumpRolloverSync() {
    if (pumpAwaitingRolloverSync) return;
    pumpAwaitingRolloverSync = true;
    syncPumpStatus(true);
}

function formatPumpCountdown(value) {
    if (value === null || value === undefined || isNaN(value)) return '--:--';
    var seconds = Math.max(0, Math.floor(Number(value)));
    var minutes = Math.floor(seconds / 60);
    var rem = seconds % 60;
    return minutes + ':' + (rem < 10 ? '0' : '') + rem;
}

function renderPumpTimer(value) {
    if (value === 'timer1') return 'Timer 1';
    if (value === 'timer2') return 'Timer 2';
    return '--';
}

function renderPumpRunLabel(status) {
    if (!status) return 'ระบบปั๊ม: --';
    if (status.running) return 'ระบบปั๊ม: กำลังทำงาน';
    if (status.active_timer && status.active_timer !== 'none') {
        return 'ระบบปั๊ม: หยุดอยู่ - พร้อมเริ่ม ' + renderPumpTimer(status.active_timer);
    }
    return 'ระบบปั๊ม: หยุดอยู่';
}

function renderPumpPhase(timer, phase, running) {
    if (!timer || timer === 'none' || !phase || phase === 'idle') return 'Idle';
    var timerLabel = renderPumpTimer(timer);
    var prefix = running ? '' : 'พร้อมเริ่ม ';
    if (phase === 'on') return prefix + timerLabel + ' - ช่วงเปิด';
    if (phase === 'off') return prefix + timerLabel + ' - ช่วงปิด';
    return timerLabel + ' - --';
}

function renderFloatState(value) {
    if (value === 'on') return 'ลูกลอย: ON -> Timer 1 / Relay 1';
    if (value === 'off') return 'ลูกลอย: OFF -> Timer 2 / Relay 2';
    return 'ลูกลอย: ไม่ทราบสถานะ';
}

function renderRelayName(value) {
    if (value === 'relay1') return 'Relay 1';
    if (value === 'relay2') return 'Relay 2';
    return 'Relay: none';
}

function relayEnergizedFor(channel, status) {
    if (!status) return false;
    var field = channel === 'relay1' ? status.relay1_energized : status.relay2_energized;
    if (typeof field === 'boolean') return field;
    if (status.active_relay === channel) return !!status.relay_energized;
    return false;
}

function renderActiveRelayState(status) {
    if (!status || !status.active_relay || status.active_relay === 'none') return 'รีเลย์: --';
    var label = renderRelayName(status.active_relay);
    if (!status.running) return label + ' พร้อมเริ่ม (OFF)';
    return label + (status.relay_energized ? ' ON' : ' OFF');
}

function renderRelayChannelState(channel, status) {
    var label = renderRelayName(channel);
    var energized = relayEnergizedFor(channel, status);
    var suffix = (!status || status.running || status.active_relay !== channel) ? '' : ' พร้อมเริ่ม';
    return label + ': ' + (energized ? 'ON' : 'OFF') + suffix;
}

function renderSettingsStatus(value, autoStart) {
    var label = 'สถานะค่า: ' + value;
    if (value === 'loaded') label = 'ค่าโหลดแล้ว';
    else if (value === 'defaults_missing') label = 'ใช้ค่าเริ่มต้น';
    else if (value === 'defaults_invalid') label = 'ค่าเริ่มต้นไม่ถูกต้อง';
    else if (value === 'defaults_error') label = 'โหลดค่าเริ่มต้นผิดพลาด';
    return label + ' • Auto-start: ' + (autoStart ? 'ON' : 'OFF');
}

/* ======== Cooling Dashboard ======== */

function wireCoolingForm() {
    var form = pumpEl('cooling-config-form');
    if (form) {
        form.onsubmit = function(e) {
            e.preventDefault();
            saveCoolingConfig();
        };
    }

    var ids = [
        'cooling-threshold-input',
        'cooling-hysteresis-input',
        'cooling-test-timeout',
        'cooling-min-off',
        'cooling-auto-enable'
    ];
    for (var i = 0; i < ids.length; i++) {
        var input = pumpEl(ids[i]);
        if (!input) continue;
        input.oninput = markCoolingDirty;
        input.onchange = markCoolingDirty;
    }
}

function wireCoolingActions() {
    var autoBtn = pumpEl('cooling-mode-auto');
    var forceBtn = pumpEl('cooling-mode-force-off');
    var testBtn = pumpEl('cooling-test-on');
    if (autoBtn) autoBtn.onclick = function() { setCoolingMode('auto'); };
    if (forceBtn) forceBtn.onclick = function() { setCoolingMode('force_off'); };
    if (testBtn) testBtn.onclick = function() { setCoolingMode('test_on'); };
}

function markCoolingDirty() {
    coolingDirty = true;
    var warning = pumpEl('cooling-unsaved-warning');
    if (warning) warning.classList.remove('hidden');
    setText('cooling-config-state', 'มีการแก้ไขที่ยังไม่ได้บันทึก');
    updateCoolingConfigSaveButton(true);
}

function setCoolingClean(label) {
    coolingDirty = false;
    var warning = pumpEl('cooling-unsaved-warning');
    if (warning) warning.classList.add('hidden');
    setText('cooling-config-state', label || 'บันทึกแล้ว');
    updateCoolingConfigSaveButton(true);
}

function updateCoolingConfigSaveButton(enabled) {
    var saveBtn = pumpEl('cooling-save-config');
    if (saveBtn) saveBtn.disabled = !enabled || !coolingConfigLoaded || coolingPending;
}

function loadCoolingConfig() {
    setText('cooling-config-state', 'กำลังโหลดค่า...');
    setCoolingAlert('');
    updateCoolingConfigSaveButton(false);
    apiGet('/api/cooling/config', function(err, data) {
        if (err || !data || !data.ok) {
            coolingConfigLoaded = false;
            setText('cooling-config-state', 'โหลดค่า cooling ไม่สำเร็จ');
            setCoolingAlert('โหลดค่า cooling ไม่สำเร็จ');
            updateCoolingConfigSaveButton(false);
            updateCoolingButtons();
            return;
        }
        coolingConfig = data;
        coolingConfigLoaded = true;
        setCoolingConfigFields(data);
        if (data.status) applyCoolingStatus(data.status, true);
        setCoolingClean('โหลดค่า cooling แล้ว');
    });
}

function setCoolingConfigFields(data) {
    setNumericInput('cooling-threshold-input', x10ToInput(data.threshold_c_x10));
    setNumericInput('cooling-hysteresis-input', x10ToInput(data.hysteresis_c_x10));
    setNumericInput('cooling-test-timeout', data.test_timeout_sec);
    setNumericInput('cooling-min-off', data.compressor_min_off_sec);
    var auto = pumpEl('cooling-auto-enable');
    if (auto) auto.checked = !!data.auto_enable;
}

function setNumericInput(id, value) {
    var el = pumpEl(id);
    if (el && value !== null && value !== undefined) el.value = value;
}

function x10ToInput(value) {
    if (value === null || value === undefined || isNaN(value)) return '';
    return (Number(value) / 10).toFixed(1);
}

function readCoolingX10(id, label, minValue, maxValue) {
    var el = pumpEl(id);
    var raw = el ? el.value.trim() : '';
    var value = Number(raw);
    if (raw === '' || !isFinite(value)) {
        return { ok: false, message: label + ': กรุณากรอกตัวเลข', value: 0 };
    }
    var x10 = Math.round(value * 10);
    if (Math.abs((value * 10) - x10) > 0.0001 || x10 < minValue || x10 > maxValue) {
        return { ok: false, message: label + ': ค่าอยู่นอกช่วงที่รองรับ', value: 0 };
    }
    return { ok: true, value: x10 };
}

function readCoolingU32(id, label, minValue, maxValue) {
    var el = pumpEl(id);
    var raw = el ? el.value.trim() : '';
    if (!/^\d+$/.test(raw)) {
        return { ok: false, message: label + ': ต้องเป็นจำนวนเต็ม', value: 0 };
    }
    var value = parseInt(raw, 10);
    if (value < minValue || value > maxValue) {
        return { ok: false, message: label + ': ค่าอยู่นอกช่วงที่รองรับ', value: 0 };
    }
    return { ok: true, value: value };
}

function buildCoolingConfigPayload() {
    if (!coolingConfig) return null;
    var threshold = readCoolingX10('cooling-threshold-input', 'Threshold', -550, 1250);
    var hysteresis = readCoolingX10('cooling-hysteresis-input', 'Hysteresis', 1, 500);
    var testTimeout = readCoolingU32('cooling-test-timeout', 'Test ON seconds', 1, 3600);
    var minOff = readCoolingU32('cooling-min-off', 'Lockout seconds', 0, 86400);
    if (!threshold.ok) return threshold;
    if (!hysteresis.ok) return hysteresis;
    if (!testTimeout.ok) return testTimeout;
    if (!minOff.ok) return minOff;

    return {
        ok: true,
        payload: {
            threshold_c_x10: threshold.value,
            hysteresis_c_x10: hysteresis.value,
            auto_enable: !!(pumpEl('cooling-auto-enable') && pumpEl('cooling-auto-enable').checked),
            mode: coolingConfig.mode === 'auto' ? 'auto' : 'force_off',
            test_timeout_sec: testTimeout.value,
            compressor_min_off_sec: minOff.value,
            cooling_relay_polarity: coolingConfig.cooling_relay_polarity || 'active_low'
        }
    };
}

function saveCoolingConfig() {
    if (!coolingConfigLoaded || coolingPending) return;
    var built = buildCoolingConfigPayload();
    if (!built || !built.ok) {
        setCoolingAlert((built && built.message) ? built.message : 'ตรวจสอบค่า cooling แล้วลองบันทึกอีกครั้ง');
        return;
    }

    coolingPending = true;
    setCoolingAlert('');
    setText('cooling-config-state', 'กำลังบันทึก cooling...');
    updateCoolingConfigSaveButton(false);
    updateCoolingButtons();
    apiPost('/api/cooling/config', built.payload, function(err, data) {
        coolingPending = false;
        if (err || !data || !data.ok) {
            setText('cooling-config-state', 'บันทึก cooling ไม่สำเร็จ');
            setCoolingAlert((data && data.message) ? data.message : 'บันทึก cooling ไม่สำเร็จ');
            updateCoolingConfigSaveButton(true);
            updateCoolingButtons();
            return;
        }
        coolingConfig = data;
        coolingConfigLoaded = true;
        setCoolingConfigFields(data);
        if (data.status) applyCoolingStatus(data.status, true);
        setCoolingClean('บันทึก cooling แล้ว');
        showToast('บันทึกค่า cooling เรียบร้อย', 'success');
    });
}

function setCoolingMode(mode) {
    if (coolingPending) return;
    coolingPending = true;
    var label = mode === 'auto' ? 'Auto' : (mode === 'test_on' ? 'Test ON' : 'Force OFF');
    setText('cooling-mode-action-state', 'กำลังสั่ง ' + label + '...');
    updateCoolingButtons();
    apiPost('/api/cooling/mode', { mode: mode }, function(err, data) {
        coolingPending = false;
        if (err || !data || !data.ok) {
            setText('cooling-mode-action-state', '');
            setCoolingAlert((data && data.message) ? data.message : 'สั่งงาน cooling ไม่สำเร็จ');
            updateCoolingButtons();
            showToast('สั่งงาน cooling ไม่สำเร็จ', 'error');
            return;
        }
        coolingConfig = data;
        coolingConfigLoaded = true;
        if (data.status) applyCoolingStatus(data.status, true);
        setText('cooling-mode-action-state', '');
        setCoolingAlert('');
        updateCoolingButtons();
        showToast('สั่งงาน cooling เรียบร้อย', 'success');
        syncCoolingStatus(true);
    });
}

function syncCoolingStatus(force) {
    if (window.location.pathname !== '/dashboard') return;
    if (document.hidden && !force) return;
    if (coolingStatusRequestInFlight) return;
    coolingStatusRequestInFlight = true;
    apiGet('/api/cooling/status', function(err, data) {
        coolingStatusRequestInFlight = false;
        if (err || !data || !data.ok) {
            handleCoolingStatusFailure();
            return;
        }
        applyCoolingStatus(data, true);
    });
}

function applyCoolingStatus(status, authoritative) {
    coolingLastStatus = status;
    if (authoritative) coolingLastSyncMs = Date.now();

    setText('cooling-temperature', renderCoolingTemperature(status));
    setText('cooling-relay-state', 'Relay: ' + (status.relay_energized ? 'ON' : 'OFF'));
    setText('cooling-mode-state', renderCoolingMode(status.mode));
    setText('cooling-sensor-state', renderCoolingSensor(status.sensor_state));
    setText('cooling-fault-state', renderCoolingFault(status));
    setText('cooling-threshold', renderCoolingCelsius(status.threshold_c));
    setText('cooling-hysteresis', renderCoolingCelsius(status.hysteresis_c));
    setText('cooling-auto-enable-state', 'Auto-enable: ' + (status.auto_enable ? 'ON' : 'OFF'));
    setText('cooling-blocked-reason', renderCoolingBlocked(status.blocked_reason));
    setText('cooling-lockout', status.lockout_active ? formatPumpCountdown(status.lockout_remaining_sec) : '--');
    setText('cooling-test-remaining', status.test_remaining_sec ? formatPumpCountdown(status.test_remaining_sec) : '--');

    var sync = pumpEl('cooling-sync-state');
    if (sync) {
        sync.className = status.fault || status.sensor_state === 'fault'
            ? 'runtime-eyebrow error'
            : (status.lockout_active || status.mode === 'test_on' ? 'runtime-eyebrow warn' : 'runtime-eyebrow good');
        sync.textContent = status.fault || status.sensor_state === 'fault'
            ? 'Sensor fault: cooling relay forced OFF'
            : (status.lockout_active ? 'Compressor lockout active' : 'ซิงก์ล่าสุดจาก cooling runtime');
    }
    setCoolingAlert(status.fault ? 'พบ sensor fault ระบบบังคับปิด cooling relay แล้ว' : '');
    updateCoolingButtons();
}

function handleCoolingStatusFailure() {
    var stale = !coolingLastSyncMs || (Date.now() - coolingLastSyncMs > COOLING_STALE_MS);
    var sync = pumpEl('cooling-sync-state');
    if (sync) {
        sync.className = stale ? 'runtime-eyebrow error' : 'runtime-eyebrow warn';
        sync.textContent = stale ? 'สถานะ cooling ขาดการซิงก์' : 'รอซิงก์ cooling';
    }
    if (stale) {
        setCoolingAlert('ไม่สามารถอ่านสถานะ cooling ได้ชั่วคราว');
    }
    updateCoolingButtons();
}

function updateCoolingButtons() {
    var stale = !coolingLastSyncMs || (Date.now() - coolingLastSyncMs > COOLING_STALE_MS);
    var disabled = coolingPending || stale || !coolingLastStatus || coolingLastStatus.config_valid === false;
    var autoBtn = pumpEl('cooling-mode-auto');
    var forceBtn = pumpEl('cooling-mode-force-off');
    var testBtn = pumpEl('cooling-test-on');
    if (autoBtn) autoBtn.disabled = disabled;
    if (forceBtn) forceBtn.disabled = disabled;
    if (testBtn) testBtn.disabled = disabled;
    updateCoolingConfigSaveButton(coolingConfigLoaded);
}

function startCoolingStatusTimer() {
    stopCoolingStatusTimer();
    if (document.hidden) return;
    coolingPollTimer = setInterval(function() { syncCoolingStatus(false); }, COOLING_STATUS_POLL_MS);
}

function stopCoolingStatusTimer() {
    if (coolingPollTimer) clearInterval(coolingPollTimer);
    coolingPollTimer = null;
}

function handleCoolingVisibilityChange() {
    if (document.hidden) {
        stopCoolingStatusTimer();
    } else {
        startCoolingStatusTimer();
        syncCoolingStatus(true);
    }
}

function renderCoolingTemperature(status) {
    if (!status || !status.temperature_valid) return '--';
    var value = Number(status.temperature_c);
    return isFinite(value) ? value.toFixed(1) + ' C' : '--';
}

function renderCoolingCelsius(value) {
    var num = Number(value);
    return isFinite(num) ? num.toFixed(1) + ' C' : '--';
}

function renderCoolingMode(value) {
    if (value === 'auto') return 'Auto';
    if (value === 'test_on') return 'Test ON';
    if (value === 'force_off') return 'Force OFF';
    return '--';
}

function renderCoolingSensor(value) {
    if (value === 'ok') return 'Sensor: OK';
    if (value === 'fault') return 'Sensor: FAULT';
    return 'Sensor: unknown';
}

function renderCoolingFault(status) {
    if (!status) return 'Fault: --';
    if (!status.fault) return 'Fault: none';
    return 'Fault: ' + (status.fault_code || 'unknown');
}

function renderCoolingBlocked(value) {
    if (!value || value === 'none') return '--';
    if (value === 'compressor_lockout') return 'Compressor lockout';
    if (value === 'sensor_fault') return 'Sensor fault';
    if (value === 'force_off') return 'Force OFF';
    if (value === 'config_invalid') return 'Config invalid';
    return value;
}

/* ======== Hardware/Install Page ======== */

function initHardwareInstall() {
    if (window.location.pathname !== '/hardware') return;
    wireHardwareForm();
    updateHardwareSaveButton();
    loadHardwareMap();
}

function hardwareEl(id) {
    return document.getElementById(id);
}

function wireHardwareForm() {
    var form = hardwareEl('hardware-map-form');
    if (form) {
        form.addEventListener('submit', function(e) {
            e.preventDefault();
            saveHardwareMap();
        });
    }

    for (var i = 0; i < HARDWARE_FIELDS.length; i++) {
        var select = hardwareEl(HARDWARE_FIELDS[i].select);
        if (select) {
            select.addEventListener('change', markHardwareDirty);
        }
    }

    var confirm = hardwareEl('hardware-confirm-reboot');
    if (confirm) {
        confirm.addEventListener('change', updateHardwareSaveButton);
    }
}

function setHardwareState(text) {
    setText('hardware-map-state', text);
}

function setHardwareError(message) {
    var el = hardwareEl('hardware-map-error');
    if (!el) return;
    el.textContent = message || '';
}

function markHardwareDirty() {
    hardwareDirty = true;
    var warn = hardwareEl('hardware-unsaved-warning');
    if (warn) warn.classList.remove('hidden');
    updateHardwareSaveButton();
}

function setHardwareClean() {
    hardwareDirty = false;
    hardwarePending = false;
    var warn = hardwareEl('hardware-unsaved-warning');
    if (warn) warn.classList.add('hidden');
    var confirm = hardwareEl('hardware-confirm-reboot');
    if (confirm) confirm.checked = false;
    updateHardwareSaveButton();
}

function updateHardwareSaveButton() {
    var btn = hardwareEl('hardware-save-map');
    if (!btn) return;
    var confirm = hardwareEl('hardware-confirm-reboot');
    btn.disabled = hardwarePending || !hardwareDirty || !confirm || !confirm.checked;
    btn.textContent = hardwarePending ? 'Saving...' : 'Save Pending Map';
}

function loadHardwareMap() {
    setHardwareState('Loading hardware map');
    setHardwareError('');
    apiGet('/api/hardware/map', function(err, data) {
        if (err) {
            if (err.message === 'HTTP 401') {
                window.location.href = '/login';
                return;
            }
            setHardwareState('Load failed');
            setHardwareError('Could not load hardware map: ' + err.message);
            return;
        }
        if (!data || data.ok !== true) {
            setHardwareState('Load failed');
            setHardwareError('Hardware map API returned an invalid response.');
            return;
        }
        applyHardwareMap(data);
    });
}

function applyHardwareMap(data) {
    hardwareMapData = data;
    var effective = data.pending_valid && data.pending ? data.pending : data.active;

    renderHardwareBanner(data);
    renderHardwareSummary('hardware-active-summary', data.active, 'active');
    renderHardwareSummary('hardware-pending-summary', data.pending_valid ? data.pending : null, 'pending');
    populateHardwareSelects(data.options || {}, effective || data.active || {});
    renderHardwareWiring(data.active, data.pending_valid ? data.pending : null);
    renderHardwareTechnical(data.options || {});

    setHardwareClean();
    setHardwareState(data.reboot_required ? 'Pending map saved - reboot required' : 'Active map loaded');
}

function renderHardwareBanner(data) {
    var el = hardwareEl('hardware-reboot-banner');
    if (!el) return;
    if (!data || !data.reboot_required) {
        el.classList.add('hidden');
        el.textContent = '';
        return;
    }
    el.classList.remove('hidden');
    el.innerHTML = '<strong>Reboot required</strong><span> A pending GPIO map is saved. Reboot the ESP32 before wiring behavior changes.</span>';
}

function renderHardwareSummary(id, map, emptyLabel) {
    var el = hardwareEl(id);
    if (!el) return;
    if (!map) {
        el.innerHTML = '<div class="hardware-summary-empty">No ' + escHtml(emptyLabel) + ' changes</div>';
        return;
    }

    var html = '';
    for (var i = 0; i < HARDWARE_FIELDS.length; i++) {
        var field = HARDWARE_FIELDS[i];
        html += '<div class="hardware-summary-row">'
            + '<span>' + escHtml(field.label) + '</span>'
            + '<strong>GPIO ' + escHtml(String(map[field.key])) + '</strong>'
            + '</div>';
    }
    el.innerHTML = html;
}

function populateHardwareSelects(options, map) {
    for (var i = 0; i < HARDWARE_FIELDS.length; i++) {
        var field = HARDWARE_FIELDS[i];
        var select = hardwareEl(field.select);
        if (!select) continue;

        var current = map && map[field.key] !== undefined ? String(map[field.key]) : '';
        var roleOptions = options[field.key] || [];
        var html = '';
        for (var j = 0; j < roleOptions.length; j++) {
            var opt = roleOptions[j];
            var value = String(opt.gpio);
            var label = opt.label || ('GPIO ' + value);
            if (opt.is_default) label += ' (default)';
            html += '<option value="' + escHtml(value) + '"'
                + (value === current ? ' selected' : '')
                + '>' + escHtml(label) + '</option>';
        }
        select.innerHTML = html;
        if (current) select.value = current;
    }
}

function renderHardwareWiring(active, pending) {
    for (var i = 0; i < HARDWARE_FIELDS.length; i++) {
        var field = HARDWARE_FIELDS[i];
        var activeValue = active && active[field.key] !== undefined ? active[field.key] : '--';
        var text = 'Active GPIO ' + activeValue;
        if (pending && pending[field.key] !== undefined && String(pending[field.key]) !== String(activeValue)) {
            text += ' / pending GPIO ' + pending[field.key];
        }
        setText(field.wire, text);
    }
}

function renderHardwareTechnical(options) {
    var el = hardwareEl('hardware-technical-list');
    if (!el) return;

    var html = '';
    for (var i = 0; i < HARDWARE_FIELDS.length; i++) {
        var field = HARDWARE_FIELDS[i];
        var roleOptions = options[field.key] || [];
        html += '<div class="hardware-technical-group">'
            + '<h4>' + escHtml(field.label) + '</h4>';
        if (!roleOptions.length) {
            html += '<p>No options reported by firmware.</p>';
        }
        for (var j = 0; j < roleOptions.length; j++) {
            html += renderHardwareOption(roleOptions[j]);
        }
        html += '</div>';
    }
    el.innerHTML = html;
}

function renderHardwareOption(opt) {
    var tags = [];
    if (opt.input_capable) tags.push('Input');
    if (opt.output_capable) tags.push('Output');
    if (opt.internal_pull_capable) tags.push('Internal pull');
    if (opt.is_default) tags.push('Default');

    var tagHtml = '';
    for (var i = 0; i < tags.length; i++) {
        tagHtml += '<span class="capability-tag">' + escHtml(tags[i]) + '</span>';
    }

    return '<div class="hardware-option-row">'
        + '<div><strong>GPIO ' + escHtml(String(opt.gpio)) + '</strong><span>' + escHtml(opt.label || '') + '</span></div>'
        + '<div class="capability-tags">' + tagHtml + '</div>'
        + '</div>';
}

function readHardwareMapForm() {
    var map = {};
    for (var i = 0; i < HARDWARE_FIELDS.length; i++) {
        var field = HARDWARE_FIELDS[i];
        var select = hardwareEl(field.select);
        if (!select || select.value === '') return null;
        map[field.key] = parseInt(select.value, 10);
    }
    return map;
}

function saveHardwareMap() {
    var confirm = hardwareEl('hardware-confirm-reboot');
    if (!confirm || !confirm.checked) {
        setHardwareError('Confirm that a reboot is required before saving.');
        updateHardwareSaveButton();
        return;
    }

    var map = readHardwareMapForm();
    if (!map) {
        setHardwareError('Choose a GPIO for every hardware role.');
        return;
    }

    hardwarePending = true;
    updateHardwareSaveButton();
    setHardwareError('');
    setHardwareState('Saving pending map');

    apiPost('/api/hardware/map', {
        map: map,
        confirm_reboot_required: true
    }, function(err, data) {
        hardwarePending = false;
        updateHardwareSaveButton();
        if (err) {
            setHardwareState('Save failed');
            setHardwareError('Could not save hardware map: ' + err.message);
            return;
        }
        if (!data || data.ok !== true) {
            setHardwareState('Save failed');
            setHardwareError('Save rejected: ' + ((data && (data.message || data.error)) || 'invalid hardware map'));
            return;
        }
        applyHardwareMap(data);
        showToast('Pending hardware map saved', 'success');
    });
}

/* ======== Status Page ======== */

function initStatus() {
    if (window.location.pathname !== '/status') return;
    refreshFullStatus();
    setInterval(refreshFullStatus, 30000);
}

function refreshFullStatus() {
    if (document.hidden) return;
    if (statusFullRequestInFlight) return;
    statusFullRequestInFlight = true;
    apiGet('/api/status', function(err, data) {
        statusFullRequestInFlight = false;
        if (err || !data || !data.ok) {
            if (err && err.message === 'HTTP 401') { window.location.href = '/login'; }
            return;
        }

        /* System */
        setText('st-chip-model', data.chip_model);
        setText('st-chip-revision', '' + data.chip_revision);
        setText('st-chip-cores', '' + data.chip_cores);
        setText('st-cpu-freq', data.cpu_freq_mhz + ' MHz');
        setText('st-idf-version', data.idf_version);
        setText('st-project-version', data.project_version);
        setText('st-reset-reason', data.reset_reason || '--');

        /* Memory */
        var freeKb = (data.free_heap / 1024).toFixed(0);
        var minKb = (data.min_free_heap / 1024).toFixed(0);
        var totalKb = (data.total_heap / 1024).toFixed(0);
        setText('st-free-heap', freeKb + ' KB');
        setText('st-min-free-heap', minKb + ' KB');
        setText('st-total-heap', totalKb + ' KB');

        var pct = data.total_heap > 0 ? ((data.total_heap - data.free_heap) / data.total_heap * 100) : 0;
        var bar = document.getElementById('st-heap-bar');
        if (bar) bar.style.width = Math.min(100, Math.max(5, pct)).toFixed(0) + '%';
        var pctEl = document.getElementById('st-heap-pct');
        if (pctEl) pctEl.textContent = 'Usage: ' + pct.toFixed(1) + '%';

        /* Uptime */
        var secs = Math.floor(data.uptime_ms / 1000);
        var mins = Math.floor(secs / 60);
        var hrs = Math.floor(mins / 60);
        mins = mins % 60;
        var uptimeStr;
        if (hrs > 24) {
            var days = Math.floor(hrs / 24);
            hrs = hrs % 24;
            uptimeStr = days + 'd ' + hrs + 'h ' + mins + 'm';
        } else if (hrs > 0) {
            uptimeStr = hrs + 'h ' + mins + 'm';
        } else {
            uptimeStr = mins + 'm ' + (secs % 60) + 's';
        }
        setText('st-uptime', uptimeStr);

        /* STA */
        setText('st-sta-status', data.sta_connected ? 'Connected' : 'Disconnected');
        setText('st-sta-ssid', data.sta_connected ? data.sta_ssid : '--');
        setText('st-sta-ip', data.sta_connected ? data.sta_ip : '--');
        setText('st-sta-rssi', data.sta_connected && data.sta_rssi !== undefined ? data.sta_rssi + ' dBm' : '--');
        setText('st-sta-channel', data.sta_connected && data.sta_channel !== undefined ? '' + data.sta_channel : '--');
        setText('st-sta-auth', data.sta_connected && data.sta_auth ? data.sta_auth : '--');
        setText('st-mac-sta', data.mac_sta || '--');

        /* AP */
        setText('st-ap-ssid', data.ap_enabled ? data.ap_ssid : '--');
        setText('st-ap-ip', data.ap_enabled ? data.ap_ip : '--');
        setText('st-ap-clients', data.ap_enabled ? '' + data.ap_clients : '--');
        setText('st-mac-ap', data.mac_ap || '--');

        /* Services */
        setText('st-wifi-mode', data.wifi_mode || '--');
        setText('st-dns-status', data.dns_server ? 'Running' : 'Stopped');
    });
}

function setText(id, val) {
    var el = document.getElementById(id);
    if (el) el.textContent = val;
}

function refreshStatus() {
    if (document.hidden) return;
    if (statusSummaryRequestInFlight) return;
    statusSummaryRequestInFlight = true;
    apiGet('/api/status', function(err, data) {
        statusSummaryRequestInFlight = false;
        if (err || !data || !data.ok) {
            if (err && err.message === 'HTTP 401') { window.location.href = '/login'; }
            return;
        }

        /* AP Status */
        var apStatus = document.getElementById('card-ap-status');
        var apIp = document.getElementById('card-ap-ip');
        if (apStatus) apStatus.textContent = data.ap_enabled ? 'Active' : 'Off';
        if (apIp) apIp.textContent = data.ap_enabled ? 'IP: ' + data.ap_ip : 'IP: --';

        /* STA Status */
        var staStatus = document.getElementById('card-sta-status');
        var staSsid = document.getElementById('card-sta-ssid');
        if (staStatus) staStatus.textContent = data.sta_connected ? '\u0e40\u0e0a\u0e37\u0e48\u0e2d\u0e21\u0e15\u0e48\u0e2d\u0e41\u0e25\u0e49\u0e27' : '\u0e44\u0e21\u0e48\u0e44\u0e14\u0e49\u0e40\u0e0a\u0e37\u0e48\u0e2d\u0e21\u0e15\u0e48\u0e2d';
        if (staSsid) {
            if (data.sta_connected) {
                staSsid.textContent = 'Station: ' + (data.sta_ssid || '--');
                staSsid.className = 'card-sub good';
            } else {
                staSsid.textContent = 'Station: \u0e44\u0e21\u0e48\u0e44\u0e14\u0e49\u0e40\u0e0a\u0e37\u0e48\u0e2d\u0e21\u0e15\u0e48\u0e2d';
                staSsid.className = 'card-sub neutral';
            }
        }

        /* STA IP */
        var staIp = document.getElementById('card-sta-ip');
        if (staIp) {
            if (data.sta_connected && data.sta_ip) {
                staIp.textContent = 'IP: ' + data.sta_ip;
                staIp.className = 'card-sub good';
            } else {
                staIp.textContent = 'Station IP: --';
                staIp.className = 'card-sub neutral';
            }
        }

        /* Memory */
        var heapEl = document.getElementById('card-heap');
        var heapBar = document.getElementById('card-heap-bar');
        if (heapEl) heapEl.textContent = (data.free_heap / 1024).toFixed(0) + ' KB free';
        if (heapBar) {
            var pct = Math.min(100, Math.max(5, ((300000 - data.free_heap) / 300000) * 100));
            heapBar.style.width = pct.toFixed(0) + '%';
        }

        /* Uptime */
        var uptimeEl = document.getElementById('card-uptime');
        if (uptimeEl) {
            var secs = Math.floor(data.uptime_ms / 1000);
            var mins = Math.floor(secs / 60);
            var hrs = Math.floor(mins / 60);
            mins = mins % 60;
            if (hrs > 24) {
                var days = Math.floor(hrs / 24);
                hrs = hrs % 24;
                uptimeEl.textContent = days + 'd ' + hrs + 'h';
            } else if (hrs > 0) {
                uptimeEl.textContent = hrs + 'h ' + mins + 'm';
            } else {
                uptimeEl.textContent = mins + 'm ' + (secs % 60) + 's';
            }
        }

        /* Sidebar status dot */
        var dot = document.getElementById('sidebar-status-dot');
        var txt = document.getElementById('sidebar-status-text');
        if (dot && txt) {
            if (data.sta_connected) {
                dot.className = 'status-dot';
                txt.textContent = 'Online';
            } else if (data.ap_enabled) {
                dot.className = 'status-dot';
                txt.textContent = 'AP Mode';
            } else {
                dot.className = 'status-dot off';
                txt.textContent = 'Offline';
            }
        }

        /* Dashboard summary */
        setText('card-version', data.project_version || '--');
        setText('dash-chip', data.chip_model || '--');
        setText('dash-project-version', data.project_version || '--');
        setText('dash-wifi-mode', data.wifi_mode || '--');
        setText('dash-ap-clients', data.ap_enabled ? '' + data.ap_clients : '--');
        if (data.sta_connected && data.sta_rssi !== undefined) {
            setText('dash-rssi', data.sta_rssi + ' dBm');
        } else {
            setText('dash-rssi', '--');
        }
        var dashPct = data.total_heap > 0 ? ((data.total_heap - data.free_heap) / data.total_heap * 100).toFixed(1) : '--';
        setText('dash-heap-pct', dashPct !== '--' ? dashPct + '%' : '--');
    });
}

/* ======== Wi-Fi Page ======== */

var selectedSsid = null;

function initWifi() {
    if (window.location.pathname !== '/wifi') return;
    updateApPill();
    updateConnectionStatus();
}

function updateConnectionStatus() {
    apiGet('/api/status', function(err, data) {
        var el = document.getElementById('connection-status');
        var icon = document.getElementById('conn-icon');
        var title = document.getElementById('conn-title');
        var sub = document.getElementById('conn-sub');
        var btn = document.getElementById('disconnect-btn');
        if (!el || !title) return;

        if (!err && data && data.ok && data.sta_connected) {
            el.className = 'connection-status connected';
            if (icon) icon.innerHTML = '&#10003;';
            title.textContent = '\u0e40\u0e0a\u0e37\u0e48\u0e2d\u0e21\u0e15\u0e48\u0e2d\u0e41\u0e25\u0e49\u0e27: ' + (data.sta_ssid || 'Wi-Fi');
            if (sub) sub.textContent = 'IP: ' + (data.sta_ip || '--');
            if (btn) btn.style.display = '';
        } else {
            el.className = 'connection-status';
            if (icon) icon.innerHTML = '&#128268;';
            title.textContent = '\u0e44\u0e21\u0e48\u0e44\u0e14\u0e49\u0e40\u0e0a\u0e37\u0e48\u0e2d\u0e21\u0e15\u0e48\u0e2d';
            if (sub) sub.textContent = 'STA \u0e44\u0e21\u0e48\u0e44\u0e14\u0e49\u0e40\u0e0a\u0e37\u0e48\u0e2d\u0e21\u0e15\u0e48\u0e2d\u0e01\u0e31\u0e1a\u0e40\u0e04\u0e23\u0e37\u0e48\u0e2d\u0e02\u0e48\u0e32\u0e22\u0e43\u0e14\u0e46';
            if (btn) btn.style.display = 'none';
        }
    });
}

function doDisconnect() {
    var btn = document.getElementById('disconnect-btn');
    if (btn) { btn.disabled = true; btn.textContent = '\u0e01\u0e33\u0e25\u0e31\u0e07\u0e15\u0e31\u0e14\u0e40\u0e0a\u0e37\u0e48\u0e2d\u0e21...'; }

    apiPost('/api/wifi/disconnect', {}, function(err, data) {
        if (btn) { btn.disabled = false; btn.textContent = '\u0e15\u0e31\u0e14\u0e40\u0e0a\u0e37\u0e48\u0e2d\u0e21'; }

        if ((data && data.ok) || (err && !data)) {
            showToast('\u0e15\u0e31\u0e14\u0e40\u0e0a\u0e37\u0e48\u0e2d\u0e21 Wi-Fi \u0e40\u0e23\u0e35\u0e22\u0e1a\u0e23\u0e49\u0e2d\u0e22', 'success');
            clearSelection();
            setTimeout(updateConnectionStatus, 1200);
        } else {
            showToast('\u0e15\u0e31\u0e14\u0e40\u0e0a\u0e37\u0e48\u0e2d\u0e21\u0e44\u0e21\u0e48\u0e2a\u0e33\u0e40\u0e23\u0e47\u0e08', 'error');
        }
    });
}

function updateApPill() {
    apiGet('/api/status', function(err, data) {
        if (!err && data && data.ok) {
            var ipEl = document.getElementById('ap-pill-ip');
            var pill = document.getElementById('ap-pill');
            if (ipEl && data.ap_ip) ipEl.textContent = data.ap_ip;
            if (pill && !data.ap_enabled) pill.style.display = 'none';
        }
    });
}

function doScan() {
    var btn = document.getElementById('scan-btn');
    var list = document.getElementById('network-list');
    var placeholder = document.getElementById('scan-placeholder');

    if (btn) { btn.disabled = true; btn.innerHTML = '<span class=\"spinner\"></span> \u0e01\u0e33\u0e25\u0e31\u0e07\u0e2a\u0e41\u0e01\u0e19...'; }

    apiGet('/api/wifi/scan', function(err, data) {
        if (btn) { btn.disabled = false; btn.innerHTML = '&#8635; \u0e2a\u0e41\u0e01\u0e19\u0e43\u0e2b\u0e21\u0e48'; }
        if (placeholder) placeholder.style.display = 'none';

        if (err || !data || !data.ok) {
            if (list) list.innerHTML = '<li style=\"padding:32px;text-align:center;color:var(--error);\">\u0e2a\u0e41\u0e01\u0e19\u0e44\u0e21\u0e48\u0e2a\u0e33\u0e40\u0e23\u0e47\u0e08</li>';
            return;
        }

        var nets = data.networks || [];
        if (nets.length === 0) {
            if (list) list.innerHTML = '<li style=\"padding:32px;text-align:center;color:var(--outline);\">\u0e44\u0e21\u0e48\u0e1e\u0e1a\u0e40\u0e04\u0e23\u0e37\u0e48\u0e2d\u0e02\u0e48\u0e32\u0e22</li>';
            return;
        }

        /* Sort by RSSI (strongest first) */
        nets.sort(function(a, b) { return b.rssi - a.rssi; });

        var html = '';
        for (var i = 0; i < nets.length; i++) {
            var sigClass = 'weak';
            var sigLabel = '\u0e2d\u0e48\u0e2d\u0e19';
            var sigIcon = '&#9641;';
            if (nets[i].rssi >= -50) { sigClass = 'strong'; sigLabel = '\u0e14\u0e35\u0e21\u0e32\u0e01'; sigIcon = '&#9643;'; }
            else if (nets[i].rssi >= -70) { sigClass = 'good'; sigLabel = '\u0e14\u0e35'; sigIcon = '&#9642;'; }

            html += '<li class=\"network-item' + (selectedSsid === nets[i].ssid ? ' selected' : '') + '\" ' +
                    'data-ssid=\"' + escHtml(nets[i].ssid) + '\" ' +
                    'onclick=\"selectNetwork(\'' + escJs(nets[i].ssid) + '\')\">' +
                '<div class=\"network-item-left\">' +
                    '<div class=\"network-icon\">' + sigIcon + '</div>' +
                    '<div class=\"network-info\">' +
                        '<div class=\"net-ssid\">' + escHtml(nets[i].ssid) + '</div>' +
                        '<div class=\"net-detail\">' +
                            (nets[i].auth !== 'Open' ? '&#128274; ' : '') + escHtml(nets[i].auth) +
                            ' &bull; Ch ' + nets[i].channel +
                        '</div>' +
                    '</div>' +
                '</div>' +
                '<div class=\"network-item-right\">' +
                    '<div class=\"net-signal\">' +
                        '<div class=\"sig-label ' + sigClass + '\">' + sigLabel + '</div>' +
                        '<div class=\"sig-dbm\">' + nets[i].rssi + ' dBm</div>' +
                    '</div>' +
                    '<button class=\"btn btn-outline btn-sm\">\u0e40\u0e25\u0e37\u0e2d\u0e01</button>' +
                '</div>' +
            '</li>';
        }
        if (list) list.innerHTML = html;
    });
}

function toggleStaticIp() {
    var checked = document.getElementById('static-ip-toggle').checked;
    var fields = document.getElementById('static-ip-fields');
    var ipInput = document.getElementById('static-ip');
    var gwInput = document.getElementById('static-gateway');
    var nmInput = document.getElementById('static-netmask');
    if (fields) fields.style.display = checked ? 'block' : 'none';
    if (ipInput) ipInput.disabled = !checked;
    if (gwInput) gwInput.disabled = !checked;
    if (nmInput) nmInput.disabled = !checked;
}

function selectNetwork(ssid) {
    selectedSsid = ssid;

    /* Update display */
    var display = document.getElementById('selected-ssid-display');
    if (display) display.textContent = ssid;

    /* Enable input panel */
    var panel = document.getElementById('input-panel');
    var overlay = document.getElementById('overlay-hint');
    var pwInput = document.getElementById('wifi-password');
    var connectBtn = document.getElementById('connect-btn');
    var cancelBtn = document.getElementById('cancel-btn');

    var ipToggle = document.getElementById('static-ip-toggle');
    if (ipToggle) ipToggle.disabled = false;
    if (panel) panel.classList.remove('disabled');
    if (overlay) overlay.style.display = 'none';
    if (pwInput) pwInput.disabled = false;
    if (connectBtn) connectBtn.disabled = false;
    if (cancelBtn) cancelBtn.disabled = false;

    /* Update step to 2 */
    updateStepper(2);

    /* Highlight in list */
    var items = document.querySelectorAll('.network-item');
    for (var i = 0; i < items.length; i++) {
        if (items[i].getAttribute('data-ssid') === ssid) {
            items[i].classList.add('selected');
        } else {
            items[i].classList.remove('selected');
        }
    }
}

function clearSelection() {
    selectedSsid = null;
    var display = document.getElementById('selected-ssid-display');
    var panel = document.getElementById('input-panel');
    var overlay = document.getElementById('overlay-hint');
    var pwInput = document.getElementById('wifi-password');
    var connectBtn = document.getElementById('connect-btn');
    var cancelBtn = document.getElementById('cancel-btn');
    var statusEl = document.getElementById('connect-status');

    var ipToggle = document.getElementById('static-ip-toggle');
    if (ipToggle) { ipToggle.disabled = true; ipToggle.checked = false; }
    toggleStaticIp();
    if (display) display.textContent = '---';
    if (panel) panel.classList.add('disabled');
    if (overlay) overlay.style.display = '';
    if (pwInput) { pwInput.disabled = true; pwInput.value = ''; }
    if (connectBtn) connectBtn.disabled = true;
    if (cancelBtn) cancelBtn.disabled = true;
    if (statusEl) statusEl.textContent = '';

    updateStepper(1);

    /* Clear highlights */
    var items = document.querySelectorAll('.network-item');
    for (var i = 0; i < items.length; i++) {
        items[i].classList.remove('selected');
    }
}

function doConnect() {
    if (!selectedSsid) return;

    var pwInput = document.getElementById('wifi-password');
    var password = pwInput ? pwInput.value : '';
    var connectBtn = document.getElementById('connect-btn');
    var statusEl = document.getElementById('connect-status');

    if (connectBtn) { connectBtn.disabled = true; connectBtn.textContent = '\u0e01\u0e33\u0e25\u0e31\u0e07\u0e40\u0e0a\u0e37\u0e48\u0e2d\u0e21\u0e15\u0e48\u0e2d...'; }
    if (statusEl) { statusEl.textContent = ''; statusEl.style.color = 'var(--on-surface-variant)'; }

    updateStepper(3);

    var body = { ssid: selectedSsid, password: password };
    if (document.getElementById('static-ip-toggle').checked) {
        body.ip = document.getElementById('static-ip').value.trim();
        body.gateway = document.getElementById('static-gateway').value.trim();
        body.netmask = document.getElementById('static-netmask').value.trim();
    }

    apiPost('/api/wifi/connect', body, function(err, data) {
        if (connectBtn) { connectBtn.disabled = false; connectBtn.textContent = '\u0e40\u0e0a\u0e37\u0e48\u0e2d\u0e21\u0e15\u0e48\u0e2d'; }

        if (err) {
            if (statusEl) { statusEl.textContent = '\u0e40\u0e01\u0e34\u0e14\u0e02\u0e49\u0e2d\u0e1c\u0e34\u0e14\u0e1e\u0e25\u0e32\u0e14'; statusEl.style.color = 'var(--error)'; }
            updateStepper(2);
            return;
        }

        if (data && data.ok) {
            if (statusEl) { statusEl.textContent = '\u0e40\u0e0a\u0e37\u0e48\u0e2d\u0e21\u0e15\u0e48\u0e2d\u0e2a\u0e33\u0e40\u0e23\u0e47\u0e08! IP: ' + (data.ip || 'N/A'); statusEl.style.color = 'var(--secondary)'; }
            /* Mark step 3 as done */
            updateStepper(3, true);
            /* Update AP pill */
            updateApPill();
        } else {
            if (statusEl) {
                statusEl.textContent = '\u0e40\u0e0a\u0e37\u0e48\u0e2d\u0e21\u0e15\u0e48\u0e2d\u0e44\u0e21\u0e48\u0e2a\u0e33\u0e40\u0e23\u0e47\u0e08: ' + (data && data.error ? data.error : '\u0e25\u0e2d\u0e07\u0e2d\u0e35\u0e01\u0e04\u0e23\u0e31\u0e49\u0e07');
                statusEl.style.color = 'var(--error)';
            }
            updateStepper(2);
        }
    });
}

function updateStepper(activeStep, done) {
    for (var i = 1; i <= 3; i++) {
        var step = document.getElementById('step-' + i);
        if (!step) continue;
        step.classList.remove('active', 'done');
        if (i === activeStep && done) step.classList.add('done');
        else if (i === activeStep) step.classList.add('active');
        else if (i < activeStep) step.classList.add('done');
    }
}

/* ======== Utilities ======== */

function escHtml(str) {
    if (!str) return '';
    return str.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}

function escJs(str) {
    if (!str) return '';
    return str.replace(/\\/g, '\\\\').replace(/'/g, "\\'").replace(/"/g, '\\"');
}

/* ======== Init ======== */

(function() {
    var path = window.location.pathname;

    if (path === '/login') {
        initLogin();
    } else if (path === '/dashboard') {
        initDashboard();
    } else if (path === '/status') {
        initStatus();
    } else if (path === '/wifi') {
        initWifi();
    } else if (path === '/hardware') {
        initHardwareInstall();
    }
})();
