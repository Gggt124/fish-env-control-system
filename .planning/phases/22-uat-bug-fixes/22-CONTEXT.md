# Phase 22: UAT Bug Fixes

## Objective
Address the User Acceptance Testing (UAT) feedback for the Modern Web UI milestone.

## Bugs to Fix
1. **Garbled Thai Encoding ("ภาษาเพี้ยน"):**
   - **Diagnosis:** During previous execution, PowerShell commands rewrote `main/static/index.html` and `main/static/app.js` using Windows default encoding (UTF-16LE or Windows-1252) instead of UTF-8, breaking Thai characters.
   - **Fix:** Using the agent's native file modification tools (`replace_file_content` or `multi_replace_file_content`) to edit these files will automatically re-save them in pure UTF-8. Trigger minor edits in all 3 static files to correct their encoding.

2. **Sidebar Visible on Desktop Login Page ("version คอม หน้า login มี sidebar"):**
   - **Diagnosis:** `#app-sidebar` is rendered globally outside the views, so it remains visible even when `#view-login` is active.
   - **Fix:** In `app.js` `handleRoute()`, add/remove a class (e.g., `login-active`) to `document.body` based on whether `viewId === 'view-login'`. In `style.css`, add a rule: `body.login-active #app-sidebar, body.login-active .mobile-topbar { display: none; }` and also adjust `#app-content` margin for login page.

3. **Automatic Dark Mode Issue ("theme มันเปลี่ยนเป็นดำ เองตาม default ของ browser"):**
   - **Diagnosis:** `style.css` contains `@media (prefers-color-scheme: dark)` which forces dark mode based on the user's OS/browser, leading to inconsistency or undesired behavior.
   - **Fix:** Remove the `@media (prefers-color-scheme: dark)` block from `style.css` so the UI only uses the standard light theme (or keep the colors under a specific class like `.dark-mode` that is off by default).

4. **Missing % Memory ("%memory หาย"):**
   - **Diagnosis:** The System Status page only shows free heap in KB (e.g., "120 KB"). The percentage text is missing.
   - **Fix:** In `app.js` `refreshFullStatus()`, append the percentage to `st-free-heap` display (e.g., `setText('st-free-heap', freeKb + ' KB (' + pct.toFixed(1) + '% used)');`).

5. **Missing Wi-Fi Modal ("modal wifi ไม่มี"):**
   - **Diagnosis:** The "Connect" action on the Wi-Fi Setup Stepper calls `doConnect()` directly without a modal confirmation, contrary to the design requirements.
   - **Fix:** Wrap the logic inside `doConnect()` in `app.js` with `showConfirmModal('ยืนยันการเชื่อมต่อ', 'คุณต้องการเชื่อมต่อกับ ' + selectedSsid + ' ใช่หรือไม่?', function() { ... });`.
