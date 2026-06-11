---
phase: 01-persistent-sessions-storage-foundation
reviewed: 2026-06-11T09:00:00Z
depth: standard
files_reviewed: 8
files_reviewed_list:
  - components/nvs_store/nvs_store.c
  - components/nvs_store/nvs_store.h
  - components/session/session.c
  - components/session/session.h
  - main/web_server.c
  - components/session/CMakeLists.txt
  - main/static/index.html
  - main/static/app.js
findings:
  critical: 2
  warning: 1
  info: 1
  total: 4
status: issues_found
---

# Phase 01: Code Review Report

**Reviewed:** 2026-06-11T09:00:00Z
**Depth:** standard
**Files Reviewed:** 8
**Status:** issues_found

## Summary

The standard depth review identified 4 issues in the session validation, HTTP request handling, and frontend JS. The most critical issues are a stack buffer over-read during JWT signature verification in `session.c` and a DOM XSS vulnerability in `app.js` caused by insufficient HTML encoding of SSID data when injected into HTML attributes.

## Critical Issues

### CR-01: Stack Buffer Over-Read in JWT Signature Verification

**File:** `components/session/session.c:150`
**Issue:** The constant-time string comparison in `session_validate` iterates up to 44 characters (the maximum base64url length for an HMAC-SHA256 signature). However, if the client-provided `signature_b64_in` is shorter than 44 characters and happens to be located near the end of the `token_copy` stack buffer, the loop will read past the null terminator and past the end of `token_copy[256]`. This is an undefined behavior stack buffer over-read that could cause crashes or unintended hardware watchpoint triggers.
**Fix:** Validate that the lengths match before the loop. Since the expected signature length is fixed, returning early on a length mismatch does not leak timing information useful for forging the signature.

```c
    size_t expected_len = strlen(expected_sig_b64);
    size_t in_len = strlen(signature_b64_in);
    
    if (in_len != expected_len) {
        return false;
    }

    // Constant-time string comparison to mitigate HMAC timing attacks
    volatile int diff = 0;
    for (size_t i = 0; i < expected_len; i++) {
        diff |= (expected_sig_b64[i] ^ signature_b64_in[i]);
    }
    if (diff != 0) {
        return false; // Signature mismatch
    }
```

### CR-02: DOM XSS via Unescaped Double Quotes in `escJs`

**File:** `main/static/app.js:2117`
**Issue:** The `escJs()` function escapes backslashes and single quotes but does not encode double quotes into HTML entities (it encodes them as `\"`). When `escJs()` is used inside double-quoted HTML attributes like `onclick=\"...\"`, an attacker broadcasting an SSID containing a double quote (e.g., `X" onmouseover="alert(1)`) can break out of the HTML attribute and inject arbitrary event handlers. This executes malicious JS in the admin's browser during Wi-Fi setup.
**Fix:** Apply `escHtml()` to the output of `escJs()` when interpolating JS strings into HTML attributes.

```javascript
// Change lines 2115-2117
html += '<div class=\"network-item' + (isConnected ? ' connected' : '') + (!isConnected && selectedSsid === nets[i].ssid ? ' selected' : '') + '\" ' +
        'data-ssid=\"' + escHtml(nets[i].ssid) + '\" ' +
        (isConnected ? '' : 'onclick=\"selectNetwork(\'' + escHtml(escJs(nets[i].ssid)) + '\')\" style="cursor:pointer;"') + '>' +
```

## Warnings

### WR-01: Global Rate Limiting Allows Unauthenticated DoS

**File:** `main/web_server.c:1960`
**Issue:** The `/api/login` endpoint uses global static variables (`s_login_attempts` and `s_login_block_until`) to rate limit login failures. Any unauthenticated attacker can intentionally submit invalid credentials 5 times and trigger the rate limit, blocking all legitimate users (including the owner) from logging in for the duration of `RATE_LIMIT_WINDOW`.
**Fix:** Track rate limits per-IP or introduce progressive delays instead of a global lockout. For a lightweight embedded fix, consider an LRU cache tracking the last 4 IP addresses and their failure counts.

## Info

### IN-01: HTML Entities Displayed Literally in Confirm Modal

**File:** `main/static/app.js:1975`
**Issue:** When prompting to forget a Wi-Fi network, `escHtml(ssid)` is passed to `showConfirmModal()`. However, `showConfirmModal` uses `textContent` to inject the message into the DOM (line 161). This causes `escHtml` to double-escape characters, making entities like `&amp;` visible to the user.
**Fix:** Pass the raw `ssid` to `showConfirmModal` since `textContent` safely handles special characters without executing them.

```javascript
    showConfirmModal('ลืมเครือข่าย', '\u0e25\u0e37\u0e21 "' + ssid + '" \u0e43\u0e0a\u0e48\u0e2b\u0e23\u0e37\u0e2d\u0e44\u0e21\u0e48?', function() {
```
