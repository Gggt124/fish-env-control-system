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

function apiGet(url, cb) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, true);
    xhr.onload = function() {
        if (xhr.status === 200) {
            try { cb(null, JSON.parse(xhr.responseText)); }
            catch(e) { cb(e, null); }
        } else {
            cb(new Error('HTTP ' + xhr.status), null);
        }
    };
    xhr.onerror = function() { cb(new Error('Network error'), null); };
    xhr.send();
}

function apiPost(url, body, cb) {
    var xhr = new XMLHttpRequest();
    xhr.open('POST', url, true);
    xhr.setRequestHeader('Content-Type', 'application/json');
    xhr.onload = function() {
        try {
            var parsed = JSON.parse(xhr.responseText);
            cb(null, parsed);
        } catch(e) {
            if (xhr.status === 200) {
                cb(e, null);
            } else {
                cb(new Error('HTTP ' + xhr.status), null);
            }
        }
    };
    xhr.onerror = function() { cb(new Error('Network error'), null); };
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
var pumpRelayPolarity = 'active_low';
var pumpDirty = false;
var pumpPending = false;
var pumpLastStatus = null;
var pumpLastSyncMs = 0;
var pumpLocalCountdown = null;
var pumpPollTimer = null;
var pumpTickTimer = null;

function initDashboard() {
    initPumpDashboard();
}

function initPumpDashboard() {
    if (window.location.pathname !== '/dashboard') return;
    wirePumpForm();
    wirePumpActions();
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
    pumpDirty = true;
    var warning = pumpEl('pump-unsaved-warning');
    if (warning) warning.classList.remove('hidden');
    setText('pump-config-state', 'มีการแก้ไขที่ยังไม่ได้บันทึก');
}

function setPumpClean(label) {
    pumpDirty = false;
    var warning = pumpEl('pump-unsaved-warning');
    if (warning) warning.classList.add('hidden');
    setText('pump-config-state', label || 'บันทึกแล้ว');
}

function loadPumpConfig() {
    setText('pump-config-state', 'กำลังโหลดค่า...');
    setPumpAlert('pump-config-error', '');
    apiGet('/api/pump/config', function(err, data) {
        if (err || !data || !data.ok) {
            setText('pump-config-state', 'โหลดค่าไม่สำเร็จ');
            setPumpAlert('pump-config-error', 'โหลดค่าตั้งเวลาไม่สำเร็จ แต่ยังสามารถดูสถานะปั๊มได้');
            return;
        }
        pumpConfig = data;
        pumpRelayPolarity = data.relay_polarity || pumpRelayPolarity;
        setDurationFields('timer1-on', data.timer1_on_sec);
        setDurationFields('timer1-off', data.timer1_off_sec);
        setDurationFields('timer2-on', data.timer2_on_sec);
        setDurationFields('timer2-off', data.timer2_off_sec);
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
}

function validatePumpConfig() {
    clearPumpValidation();
    var timer1On = readDuration('timer1-on', 'Timer 1 ช่วงเปิด');
    var timer1Off = readDuration('timer1-off', 'Timer 1 ช่วงปิด');
    var timer2On = readDuration('timer2-on', 'Timer 2 ช่วงเปิด');
    var timer2Off = readDuration('timer2-off', 'Timer 2 ช่วงปิด');
    if (!timer1On.ok || !timer1Off.ok || !timer2On.ok || !timer2Off.ok) {
        return null;
    }
    return {
        timer1_on_sec: timer1On.value,
        timer1_off_sec: timer1Off.value,
        timer2_on_sec: timer2On.value,
        timer2_off_sec: timer2Off.value,
        auto_start: !!(pumpEl('pump-auto-start') && pumpEl('pump-auto-start').checked),
        relay_polarity: pumpRelayPolarity
    };
}

function savePumpConfig() {
    var payload = validatePumpConfig();
    var saveBtn = pumpEl('pump-save-config');
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
            return;
        }
        pumpConfig = data.config || data;
        if (pumpConfig.relay_polarity) pumpRelayPolarity = pumpConfig.relay_polarity;
        setPumpClean('บันทึกแล้ว');
        showToast('บันทึกค่าปั๊มเรียบร้อย', 'success');
        loadPumpConfig();
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
    apiGet('/api/pump/status', function(err, data) {
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
        pumpLocalCountdown = Number(status.countdown_sec || 0);
    }

    setText('pump-running-label', status.running ? 'ระบบปั๊ม: กำลังทำงาน' : 'ระบบปั๊ม: หยุดอยู่');
    setText('pump-countdown', formatPumpCountdown(pumpLocalCountdown));
    setText('pump-active-timer', renderPumpTimer(status.active_timer));
    setText('pump-phase', renderPumpPhase(status.active_timer, status.phase));
    setText('pump-float-state', renderFloatState(status.float_state));
    setText('pump-relay-state', renderRelayState(status.relay_energized));

    var sync = pumpEl('pump-sync-state');
    if (sync) {
        sync.className = status.initial_stabilizing ? 'runtime-eyebrow warn' : 'runtime-eyebrow good';
        sync.textContent = status.initial_stabilizing ? 'กำลังรอลูกลอยนิ่ง' : 'ซิงก์ล่าสุดจากอุปกรณ์';
    }
    if (!pumpDirty && status.settings_status) {
        setText('pump-config-state', renderSettingsStatus(status.settings_status, status.auto_start));
    }
    setPumpAlert('pump-status-error', status.config_valid === false ? 'ค่าตั้งเวลายังไม่พร้อมใช้งาน' : '');
    updatePumpButtons();
}

function handlePumpStatusFailure() {
    var now = Date.now();
    var stale = !pumpLastSyncMs || (now - pumpLastSyncMs > 5000);
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
    var stale = !pumpLastSyncMs || (Date.now() - pumpLastSyncMs > 5000);
    var disabled = pumpPending || stale || !pumpLastStatus || pumpLastStatus.config_valid === false;
    if (startBtn) startBtn.disabled = disabled || !!(pumpLastStatus && pumpLastStatus.running);
    if (stopBtn) stopBtn.disabled = disabled || !(pumpLastStatus && pumpLastStatus.running);
}

function startPumpLiveTimers() {
    stopPumpLiveTimers();
    if (document.hidden) return;
    pumpPollTimer = setInterval(function() { syncPumpStatus(false); }, 2000);
    pumpTickTimer = setInterval(tickPumpCountdown, 1000);
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
    if (pumpLastStatus.running && pumpLocalCountdown !== null && pumpLocalCountdown > 0) {
        pumpLocalCountdown -= 1;
        setText('pump-countdown', formatPumpCountdown(pumpLocalCountdown));
    }
    if (pumpLastSyncMs && Date.now() - pumpLastSyncMs > 5000) {
        handlePumpStatusFailure();
    }
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

function renderPumpPhase(timer, phase) {
    if (!timer || timer === 'none' || !phase || phase === 'idle') return 'Idle';
    var timerLabel = renderPumpTimer(timer);
    if (phase === 'on') return timerLabel + ' - ช่วงเปิด';
    if (phase === 'off') return timerLabel + ' - ช่วงปิด';
    return timerLabel + ' - --';
}

function renderFloatState(value) {
    if (value === 'off') return 'ลูกลอย: OFF (ใช้ Timer 1)';
    if (value === 'on') return 'ลูกลอย: ON (ใช้ Timer 2)';
    return 'ลูกลอย: ไม่ทราบสถานะ';
}

function renderRelayState(value) {
    if (value === true) return 'รีเลย์: ON (จ่ายปั๊ม)';
    if (value === false) return 'รีเลย์: OFF (ตัดปั๊ม)';
    return 'รีเลย์: ไม่ทราบสถานะ';
}

function renderSettingsStatus(value, autoStart) {
    var label = 'สถานะค่า: ' + value;
    if (value === 'loaded') label = 'ค่าโหลดแล้ว';
    else if (value === 'defaults_missing') label = 'ใช้ค่าเริ่มต้น';
    else if (value === 'defaults_invalid') label = 'ค่าเริ่มต้นไม่ถูกต้อง';
    else if (value === 'defaults_error') label = 'โหลดค่าเริ่มต้นผิดพลาด';
    return label + ' • Auto-start: ' + (autoStart ? 'ON' : 'OFF');
}

/* ======== Status Page ======== */

function initStatus() {
    if (window.location.pathname !== '/status') return;
    refreshStatus();
    refreshFullStatus();
    setInterval(refreshFullStatus, 30000);
}

function refreshFullStatus() {
    apiGet('/api/status', function(err, data) {
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
    apiGet('/api/status', function(err, data) {
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

        if (!err && data && data.ok) {
            showToast('\u0e15\u0e31\u0e14\u0e40\u0e0a\u0e37\u0e48\u0e2d\u0e21 Wi-Fi \u0e40\u0e23\u0e35\u0e22\u0e1a\u0e23\u0e49\u0e2d\u0e22', 'success');
            clearSelection();
            updateConnectionStatus();
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
    }
})();
