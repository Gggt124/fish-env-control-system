---
phase: 01-persistent-sessions-storage-foundation
reviewed: 2026-06-11T10:35:00Z
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
  critical: 0
  warning: 3
  info: 2
  total: 5
status: issues_found
---

# Phase 01: Code Review Report

**Reviewed:** 2026-06-11T10:35:00Z
**Depth:** standard
**Files Reviewed:** 8
**Status:** issues_found

## Summary

The review focused on the session storage and NVS persistence mechanisms. The foundation uses a stateless JWT system with a 32-byte secret persisted in NVS across reboots. The NVS interaction wrappers and cryptographic implementations are robust and handle fallback generation properly if the secret is corrupted or missing. 

However, several warnings were identified, primarily regarding logic flaws in token expiration, rate limiting, and cookie parsing boundaries that could cause degraded functionality or localized Denial of Service conditions. No critical blockers (RCE, data loss, etc.) were found, but the warnings should be addressed before considering the auth mechanism hardened.

## Warnings

### WR-01: Non-expiring JWT tokens pose security risk

**File:** `components/session/session.c:182`
**Issue:** JWT expiration verification was completely removed to allow persistent sessions across reboots. This means a compromised session token remains valid indefinitely unless the underlying device NVS JWT secret is forcefully rotated, violating standard security principles.
**Fix:** Instead of disabling expiration, add a long-lived `exp` claim based on device uptime (`esp_timer_get_time`), and implement a sliding window or silent refresh. Alternatively, if strict persistence across device restarts without RTC is required, bind the token more tightly to client footprint or enforce periodic re-authentication.

```c
// components/session/session.c
// Example: Re-enable expiration checking, perhaps with a 30-day window
int64_t now = esp_timer_get_time() / 1000000;
// Verify token expiration logic here rather than skipping it.
```

### WR-02: Rate limit lockout logic does not reset attempt counter on expiration

**File:** `main/web_server.c:1937`
**Issue:** When an IP hits the login rate limit, `block_until` is set. After the lockout window expires (`block_until <= now`), the `attempts` counter is never reset. A single subsequent failed login will push `attempts` above `RATE_LIMIT_MAX` again, instantly re-applying the full lockout penalty rather than granting the user a fresh set of attempts.
**Fix:** Reset the `attempts` counter to `0` when a block has naturally expired before processing the current login attempt.

```c
    int64_t now = esp_timer_get_time() / 1000000;
    if (block_until > now) {
        char resp[128];
        int remaining = (int)(block_until - now);
        snprintf(resp, sizeof(resp), "{\"ok\":false,\"error\":\"rate_limited\",\"retry_after\":%d}", remaining);
        return send_json(req, resp, "429 Too Many Requests");
    } else if (rl->attempts >= RATE_LIMIT_MAX) {
        /* Reset attempts because the block period has expired */
        rl->attempts = 0;
    }
```

### WR-03: Insecure cookie parsing causes potential Denial of Service

**File:** `main/web_server.c:260` and `main/static/app.js:24`
**Issue:** Both the server backend (`strstr(cookie_buf, "session=")`) and the frontend JS (`match(/session=([^;]+)/)`) use loose boundary checks when parsing the `session` cookie. If a user has another cookie whose name ends with "session" (e.g., `mysession=123`), the parsers will incorrectly extract `123` as the token. This results in an immediate authentication failure (DoS) for the valid user.
**Fix:** Enforce word boundaries when parsing the cookie string.

```c
// main/web_server.c
    const char *start = strstr(cookie_buf, "session=");
    while (start && start > cookie_buf && *(start - 1) != ' ' && *(start - 1) != ';') {
        start = strstr(start + 1, "session=");
    }
```
```javascript
// main/static/app.js
    var c = document.cookie.match(/(?:^|;\s*)session=([^;]+)/);
```

## Info

### IN-01: Silent password truncation during Wi-Fi connect

**File:** `main/web_server.c:2149`
**Issue:** `strncpy(password, p->valuestring, sizeof(password) - 1)` silently truncates a password longer than 64 characters. The subsequent check `if (strlen(password) > 64)` will never trigger because the string is already truncated, leading to an opaque authentication failure rather than a clear 400 Bad Request error.
**Fix:** Check the length of `p->valuestring` directly before copying it to the local buffer.

### IN-02: Missing explicit null-termination after strncpy

**File:** `main/web_server.c:1925`
**Issue:** `strncpy(rl->ip, client_ip, sizeof(rl->ip));` does not explicitly null-terminate `rl->ip` if `client_ip` happens to be exactly 64 characters long. While IPv4/IPv6 strings are practically shorter than 64 bytes in ESP-IDF, relying on this assumption without forced null-termination is an anti-pattern.
**Fix:** Add `rl->ip[sizeof(rl->ip) - 1] = '\0';` immediately after the `strncpy`.

---

_Reviewed: 2026-06-11T10:35:00Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
