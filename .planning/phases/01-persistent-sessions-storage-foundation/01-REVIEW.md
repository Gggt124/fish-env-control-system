---
phase: 01-persistent-sessions-storage-foundation
reviewed: 2026-06-11T08:35:00Z
depth: standard
files_reviewed: 8
files_reviewed_list:
  - components/nvs_store/nvs_store.c
  - components/nvs_store/nvs_store.h
  - components/session/session.c
  - components/session/session.h
  - components/session/CMakeLists.txt
  - main/web_server.c
  - main/static/index.html
  - main/static/app.js
findings:
  critical: 3
  warning: 2
  info: 1
  total: 6
status: issues_found
---

# Phase 01: Code Review Report

**Reviewed:** 2026-06-11T08:35:00Z
**Depth:** standard
**Files Reviewed:** 8
**Status:** issues_found

## Summary

The review covered the core NVS storage, stateless JWT session logic, and the web server routing implementation. While the component boundaries are clean and the C code follows ESP-IDF patterns nicely, three critical security issues were identified that compromise the integrity of the login session and CSRF protections. In particular, the JWT verification is vulnerable to timing attacks and potential payload truncation, and the `is_same_origin` CSRF middleware can be completely bypassed by forcing an excessively long `Origin` header.

## Critical Issues

### CR-01: CSRF Bypass via Long Origin/Referer Header

**File:** `main/web_server.c:375-388`
**Issue:** The `is_same_origin` function uses a 128-byte buffer to read the `Origin` and `Referer` headers. If an attacker submits a cross-origin POST from a domain whose length exceeds 127 characters (e.g., a very long subdomain), `httpd_req_get_hdr_value_len` will return a length >= 127. The code checks `if (buf_len > 0 && buf_len < sizeof(buf) - 1)`, which fails for the long header. As a result, the buffer remains empty (`buf[0] == '\0'`). The function then interprets this as a "missing header" and falls back to returning the `allow_missing_header` parameter. Because critical endpoints like `/api/wifi/connect` use `allow_missing_header = true`, the attacker successfully bypasses the CSRF check entirely.
**Fix:** Explicitly reject requests where the header length exceeds the buffer size, rather than falling through to the missing header logic.

```c
    size_t origin_len = httpd_req_get_hdr_value_len(req, "Origin");
    size_t referer_len = httpd_req_get_hdr_value_len(req, "Referer");
    char buf[128] = {0};

    if (origin_len > 0) {
        if (origin_len >= sizeof(buf)) return false; // Reject, header too long
        httpd_req_get_hdr_value_str(req, "Origin", buf, sizeof(buf));
    } else if (referer_len > 0) {
        if (referer_len >= sizeof(buf)) return false; // Reject, header too long
        httpd_req_get_hdr_value_str(req, "Referer", buf, sizeof(buf));
    } else {
        return allow_missing_header;
    }
```

### CR-02: JWT HMAC Timing Attack Vulnerability

**File:** `components/session/session.c:148`
**Issue:** In `session_validate`, the signature is compared using standard `strcmp(signature_b64_in, expected_sig_b64) != 0`. `strcmp` compares strings byte-by-byte and terminates early when a difference is found. This exposes the validation to a timing attack: an attacker can iteratively guess the valid HMAC signature by measuring the exact time the function takes to return, enabling them to forge a session token.
**Fix:** Use a constant-time comparison loop instead of `strcmp` for the signature validation.

```c
    // Constant-time string comparison to mitigate HMAC timing attacks
    volatile int diff = 0;
    for (int i = 0; i < 44; i++) {
        if (expected_sig_b64[i] == '\0' && signature_b64_in[i] == '\0') break;
        diff |= (expected_sig_b64[i] ^ signature_b64_in[i]);
    }
    if (diff != 0 || strlen(signature_b64_in) != strlen(expected_sig_b64)) {
        return false; // Signature mismatch
    }
```

### CR-03: JWT Payload Truncation Bypass

**File:** `components/session/session.c:95,152`
**Issue:** The `payload_json` buffer in `session_create` is statically sized to 128 bytes. While currently safe with "admin" and IPv4, if `DEFAULT_USERNAME` is changed to a longer string or an IPv6 address is logged, `snprintf` will truncate the JSON string. The truncated payload is then signed and returned. During `session_validate`, the code searches for `"iat":` and optionally verifies token expiration. If truncation cuts off the `"iat"` field, `iat_start` is NULL, and the expiration block is silently skipped, yielding a token that never expires. Additionally, `base64url_decode` limits the payload decoding to 127 bytes, which will reject valid but large tokens completely.
**Fix:** Increase `payload_json` to 256 bytes, bump the `payload_b64` buffer accordingly, and enforce that the `"iat"` field exists during validation.

```c
    // In session_create:
    char payload_json[256];
    char payload_b64[384];

    // In session_validate:
    unsigned char payload_json[256];
    if (!base64url_decode(payload_b64, payload_json, sizeof(payload_json) - 1, &payload_len)) {
        return false;
    }
    
    // Enforce that 'iat' is present if expiration is enabled
    if (SESSION_MAX_AGE_SEC > 0) {
        char iat_key[8] = "\"iat\":";
        char *iat_start = strstr((char *)payload_json, iat_key);
        if (!iat_start) return false; // Fail securely if iat is missing
        // ... proceed with iat evaluation
    }
```

## Warnings

### WR-01: Silent Wi-Fi Password Truncation

**File:** `main/web_server.c:2086`
**Issue:** The Wi-Fi password buffer is declared as `char password[64] = {0};`. When reading from the JSON payload, it copies up to `sizeof(password) - 1` (63 bytes). ESP-IDF allows exactly 64 characters for a raw WPA2 PMK. Because the buffer is capped at 63 bytes + null terminator, a 64-character PMK will be silently truncated to 63 characters. The subsequent validation `if (strlen(password) > 64)` becomes dead code, and the truncated password causes a silent connection failure.
**Fix:** Increase the buffer size to 65 bytes so it can safely hold a 64-character password plus a null terminator.

```c
    char password[65] = {0}; // Increased to hold 64-char PMK + null
    // ...
    if (strlen(password) > 64) {
        return send_json(req, "{\"ok\":false,\"error\":\"password_too_long\"}", "400 Bad Request");
    }
```

### WR-02: Global Rate Limit Allows Denial of Service

**File:** `main/web_server.c:1947`
**Issue:** In `handle_api_login`, the `s_login_attempts` and `s_login_block_until` variables are static globals. If any user (or attacker) fails to log in `RATE_LIMIT_MAX` times, the entire web interface will block all login attempts across all IP addresses. This allows a trivial Denial of Service (DoS) attack where a bad actor can constantly spam incorrect credentials to lock out the actual administrator.
**Fix:** Considering this runs locally on ESP-IDF (SoftAP + STA), it may be better to scope rate limiting per connection/IP, or explicitly document this risk in `README.md` if memory constraints preclude a per-IP rate limit table.

## Info

### IN-01: Potential JSON Injection in JWT Session Payload

**File:** `components/session/session.c:97`
**Issue:** `snprintf(payload_json, sizeof(payload_json), "{\"sub\":\"%s\",...}", username, ...)` injects `username` directly into the JSON string without any escaping. While this is currently safe because `handle_api_login` forces the username to exactly match `DEFAULT_USERNAME` ("admin"), any future usage of `session_create` with dynamic or external usernames could lead to JSON injection vulnerabilities (where an attacker crafts a username containing `","ip":"...` to forge session fields).
**Fix:** As a future-proofing measure, properly escape the username string before formatting it, or use the `cJSON` library to safely construct the JWT payload string.

---

_Reviewed: 2026-06-11T08:35:00Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
