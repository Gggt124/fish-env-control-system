import re

with open('main/static/app.js', 'r', encoding='utf-8') as f:
    content = f.read()

# 1. Add helpers
helpers = """
function setLoading(btn, isLoading) {
    if (typeof btn === 'string') btn = document.getElementById(btn);
    if (!btn) return;
    btn.disabled = !!isLoading;
    if (isLoading) {
        btn.classList.add('loading');
    } else {
        btn.classList.remove('loading');
    }
}

function showConfirmModal(title, message, onConfirm) {
    var container = document.getElementById('modal-container');
    var titleEl = document.getElementById('modal-title');
    var msgEl = document.getElementById('modal-message');
    var cancelBtn = document.getElementById('modal-cancel');
    var confirmBtn = document.getElementById('modal-confirm');
    
    if (!container || !titleEl || !msgEl || !cancelBtn || !confirmBtn) return;
    
    titleEl.textContent = title;
    msgEl.textContent = message;
    
    function cleanup() {
        container.classList.add('hidden');
        cancelBtn.onclick = null;
        confirmBtn.onclick = null;
    }
    
    cancelBtn.onclick = function() { cleanup(); };
    confirmBtn.onclick = function() { cleanup(); if (onConfirm) onConfirm(); };
    
    container.classList.remove('hidden');
}

"""

content = content.replace("/* ======== Login Page ======== */", helpers + "/* ======== Login Page ======== */")

# 2. initLogin
content = re.sub(
    r"btn\.disabled = true;\s*btn\.textContent = '[^']+';",
    r"setLoading(btn, true);",
    content
)
content = re.sub(
    r"btn\.disabled = false;\s*btn\.textContent = '[^']+';",
    r"setLoading(btn, false);",
    content
)

# 3. doLogout
logout_orig = r"if \(\!confirm\('[^']+'\)\) \{\s*return;\s*\}"
logout_repl = r"showConfirmModal('ออกจากระบบ', 'คุณต้องการออกจากระบบใช่หรือไม่?', function() {"
content = re.sub(logout_orig, logout_repl, content)

content = re.sub(
    r"(apiPost\('/api/logout', \{\}, function\(\) \{[\s\S]*?navigateTo\('/login'\);\s*\}\);)",
    r"\1\n    });",
    content
)

# 4. savePumpConfig
content = re.sub(
    r"if \(saveBtn\) \{\s*saveBtn\.disabled = true;\s*saveBtn\.textContent = '[^']+';\s*\}",
    r"if (saveBtn) setLoading(saveBtn, true);",
    content
)
content = re.sub(
    r"if \(saveBtn\) \{\s*saveBtn\.disabled = false;\s*saveBtn\.textContent = '[^']+';\s*\}",
    r"if (saveBtn) setLoading(saveBtn, false);",
    content
)

# 5. updateCoolingConfigSaveButton
content = re.sub(
    r"if \(saveBtn\) saveBtn\.disabled = !coolingConfigLoaded \|\| coolingPending \|\| !coolingDirty;",
    """if (saveBtn) {
        if (coolingPending) {
            setLoading(saveBtn, true);
        } else {
            setLoading(saveBtn, false);
            saveBtn.disabled = !coolingConfigLoaded || !coolingDirty;
        }
    }""",
    content
)

# 6. updateHardwareSaveButton
content = re.sub(
    r"btn\.disabled = hardwarePending \|\| !hardwareDirty \|\| !confirm \|\| !confirm\.checked;\s*btn\.textContent = hardwarePending \? 'Applying\.\.\.' : 'Apply';",
    """if (hardwarePending) {
        setLoading(btn, true);
    } else {
        setLoading(btn, false);
        btn.disabled = !hardwareDirty || !confirm || !confirm.checked;
    }""",
    content
)

# 7. doDisconnect
disconnect_orig = r"if \(\!confirm\('[^']+'\)\) \{\s*return;\s*\}"
disconnect_repl = r"showConfirmModal('ตัดการเชื่อมต่อ', 'คุณต้องการตัดการเชื่อมต่อ Wi-Fi ใช่หรือไม่?', function() {"
content = re.sub(disconnect_orig, disconnect_repl, content)

content = re.sub(
    r"if \(btn\) \{ btn\.disabled = true; btn\.textContent = 'Disconnecting\.\.\.'; \}",
    r"if (btn) setLoading(btn, true);",
    content
)

content = re.sub(
    r"if \(btn\) \{\s*btn\.disabled = false;\s*btn\.textContent = 'Disconnect';\s*\}",
    r"if (btn) setLoading(btn, false);",
    content
)

# Close the modal callback block for doDisconnect
content = re.sub(
    r"(showToast\('[^']+', 'error'\);\s*\n\s*\}\s*\n\s*\}\);)",
    r"\1\n    });",
    content
)

# 8. doScan
content = re.sub(
    r"if \(btn\) \{ btn\.disabled = true; btn\.innerHTML = '<span class=\"spinner\"></span> [^']+'; \}",
    r"if (btn) setLoading(btn, true);",
    content
)

content = re.sub(
    r"if \(btn\) \{ btn\.disabled = false; btn\.innerHTML = '[^']+'; \}",
    r"if (btn) setLoading(btn, false);",
    content
)

# 9. doConnect
content = re.sub(
    r"if \(connectBtn\) \{ connectBtn\.disabled = true; connectBtn\.textContent = '[^']+'; \}",
    r"if (connectBtn) setLoading(connectBtn, true);",
    content
)
content = re.sub(
    r"if \(connectBtn\) \{ connectBtn\.disabled = false; connectBtn\.textContent = '[^']+'; \}",
    r"if (connectBtn) setLoading(connectBtn, false);",
    content
)

with open('main/static/app.js', 'w', encoding='utf-8') as f:
    f.write(content)
