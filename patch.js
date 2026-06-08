const fs=require('fs');
let code=fs.readFileSync('main/static/app.js','utf8');
const oldStr=`function renderPumpCountdown() {\n    pumpDisplayedCountdownSec = getPumpCountdownSec();\n    setText('pump-countdown', formatPumpCountdown(pumpDisplayedCountdownSec));\n}`;
const newStr=`function renderPumpCountdown() {\n    pumpDisplayedCountdownSec = getPumpCountdownSec();\n    setText('pump-countdown', formatPumpCountdown(pumpDisplayedCountdownSec));\n\n    var ring = document.getElementById('pump-progress-ring-circle');\n    if (!ring) return;\n\n    var radius = 62;\n    var circumference = 2 * Math.PI * radius;\n    ring.style.strokeDasharray = circumference;\n\n    var offset = 0;\n    if (pumpLastStatus && pumpLastStatus.running && pumpLastStatus.active_timer && pumpLastStatus.phase) {\n        var totalSec = getPumpDurationSec(pumpLastStatus.active_timer, pumpLastStatus.phase);\n        if (totalSec > 0) {\n            var pct = pumpDisplayedCountdownSec / totalSec;\n            if (pct < 0) pct = 0;\n            if (pct > 1) pct = 1;\n            offset = circumference * (1 - pct);\n        }\n    }\n    ring.style.strokeDashoffset = offset;\n}`;
code=code.replace(oldStr,newStr);
code=code.replace(oldStr.replace(/\n/g,'\r\n'),newStr);
fs.writeFileSync('main/static/app.js',code);
