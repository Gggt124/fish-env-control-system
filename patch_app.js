const fs = require('fs');
let code = fs.readFileSync('main/static/app.js', 'utf8');

code = code.replace(
    /function markHardwareDirty\(\) \{[\s\S]*?updateHardwareSaveButton\(\);\r?\n\}/,
    `function markHardwareDirty() {
    var dirty = checkHardwareDirty();
    hardwareDirty = dirty;
    var banner = hardwareEl('hardware-unsaved-banner');
    if (banner) {
        if (dirty) {
            banner.classList.remove('hidden');
        } else {
            banner.classList.add('hidden');
        }
    }
    updateHardwareSaveButton();
}`
);

code = code.replace(
    /function setHardwareClean\(\) \{[\s\S]*?updateHardwareSaveButton\(\);\r?\n\}/,
    `function setHardwareClean() {
    hardwareDirty = false;
    hardwarePending = false;
    var banner = hardwareEl('hardware-unsaved-banner');
    if (banner) banner.classList.add('hidden');
    var confirm = hardwareEl('hardware-confirm-reboot');
    if (confirm) confirm.checked = false;
    updateHardwareSaveButton();
}`
);

code = code.replace(
    /function updateHardwareSaveButton\(\) \{[\s\S]*?\}\r?\n/,
    `function updateHardwareSaveButton() {
    var btn = hardwareEl('hardware-save-map');
    if (!btn) return;
    var confirm = hardwareEl('hardware-confirm-reboot');
    btn.disabled = hardwarePending || !hardwareDirty || !confirm || !confirm.checked;
    btn.textContent = hardwarePending ? 'Applying...' : 'Apply';
}\n`
);

fs.writeFileSync('main/static/app.js', code);
