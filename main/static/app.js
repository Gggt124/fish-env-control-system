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

function initDashboard() {
    if (window.location.pathname !== '/dashboard') return;
    refreshStatus();
    setInterval(refreshStatus, 10000);
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
