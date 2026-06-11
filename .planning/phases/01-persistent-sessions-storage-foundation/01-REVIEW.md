---
phase: 01-persistent-sessions-storage-foundation
reviewed: 2024-05-24T12:00:00Z
depth: standard
files_reviewed: 8
files_reviewed_list:
  - components/nvs_store/nvs_store.c
  - components/nvs_store/nvs_store.h
  - components/session/CMakeLists.txt
  - components/session/session.c
  - components/session/session.h
  - main/static/app.js
  - main/static/index.html
  - main/web_server.c
findings:
  critical: 1
  warning: 0
  info: 0
  total: 1
status: issues_found
---

# Phase 01: Code Review Report

**Reviewed:** 2024-05-24T12:00:00Z
**Depth:** standard
**Files Reviewed:** 8
**Status:** issues_found

## Summary

The review focused on the session, NVS storage layers, and the web UI interaction. The C codebase successfully bounds string copies, handles JSON parsing correctly, and strictly checks session tokens. However, the frontend JavaScript logic in `app.js` contains a Critical Cross-Site Scripting (XSS) vulnerability related to how Wi-Fi SSIDs are embedded into dynamically generated HTML strings.

## Critical Issues

### CR-01: Stored XSS via unescaped double quotes in dynamically generated HTML attributes

**File:** `main/static/app.js:1958, 1963, 2117`
**Issue:** When generating HTML for the Wi-Fi network list and saved profiles, the `escJs()` function is used to escape the SSID string before placing it inside an `onchange` or `onclick` HTML attribute (e.g. `onclick="selectNetwork('` + escJs(nets[i].ssid) + `')"`). While `escJs()` escapes single quotes and backslashes for JS literals, it does *not* HTML-encode double quotes (`"`). An attacker can broadcast a malicious Wi-Fi network with an SSID like `X" onmouseover="alert(1)` to prematurely close the double-quoted `onclick` attribute and inject an arbitrary HTML attribute, executing malicious JavaScript when the user views the Wi-Fi page.
**Fix:**
Wrap the `escJs` calls with `escHtml` to ensure any double quotes or HTML special characters are properly encoded for the HTML attribute context:
```javascript
// Line 1958:
html += '<div class="toggle-switch"><input type="checkbox" onchange="toggleAutoProfile(\'' + escHtml(escJs(p.ssid)) + '\', this.checked, ' + (p.auto ? 'true' : 'false') + ')" ' + (p.auto ? 'checked' : '') + '><span class="toggle-slider"></span></div>';

// Line 1963:
html += '<button class="btn-icon-text btn-forget" onclick="forgetProfile(\'' + escHtml(escJs(p.ssid)) + '\')">';

// Line 2117:
(isConnected ? '' : 'onclick=\"selectNetwork(\'' + escHtml(escJs(nets[i].ssid)) + '\')\" style="cursor:pointer;"') + '>' +
```
