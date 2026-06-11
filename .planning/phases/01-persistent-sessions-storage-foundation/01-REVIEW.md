---
phase: 01-persistent-sessions-storage-foundation
reviewed: 2024-06-11T10:16:00Z
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
  critical: 3
  warning: 2
  info: 1
  total: 6
status: issues_found
---

# Phase 01: Code Review Report

**Reviewed:** 2024-06-11T10:16:00Z
**Depth:** standard
**Files Reviewed:** 8
**Status:** issues_found

## Summary

Reviewed the implementation of the new stateless JWT session system and NVS storage extensions. While the JWT and mbedtls integration is functionally well-structured, several significant bugs and security issues were identified. 

The most critical issues involve a logical flaw in session expiration that breaks persistence across reboots, a stack memory disclosure vulnerability in client IP parsing, and predictable JWT secret generation due to premature `esp_random()` usage before Wi-Fi initialization.

## Critical Issues

### CR-01: JWT `iat` claim uses uptime, breaking persistent sessions across reboots

**File:** `components/session/session.c:193`
**Issue:** `session_validate` rejects tokens if `now < iat`, where `now` is `esp_timer_get_time() / 1000000`. Since `esp_timer_get_time()` resets to 0 on reboot, any valid session from a previous boot will have a much larger `iat` than the newly reset `now`. The token will be immediately rejected. This fundamentally breaks the "persistent sessions" requirement, as sessions cannot survive a reboot despite the secret being stored in NVS.
**Fix:** Since the device lacks an RTC, time-since-boot cannot be used for `iat` if sessions must survive reboots. Remove the `iat` claim and expiration check from the JWT validation for this local prototype, relying only on client-side cookie clearing, OR generate an abstract epoch counter persisted in NVS.
```c
// Remove the 'iat' generation in session_create and the validation block in session_validate:
// if (SESSION_MAX_AGE_SEC > 0) { ... } 
```

### CR-02: Predictable JWT Secret Generation (Pseudo-RNG)

**File:** `components/session/session.c:76`
**Issue:** `session_init()` generates a new JWT secret using `esp_random()`. However, `session_init()` is called in `app_main.c` *before* `wifi_manager_init()`. The ESP32 hardware RNG requires the RF subsystem (Wi-Fi or Bluetooth) to be active. If called before Wi-Fi is initialized, `esp_random()` returns predictable pseudo-random numbers, compromising the security of all future session tokens.
**Fix:** Temporarily enable the hardware RNG during secret generation.
```c
#include "bootloader_random.h"

// Inside session_init():
        bootloader_random_enable();
        for (int i = 0; i < sizeof(s_jwt_secret); i++) {
            s_jwt_secret[i] = esp_random() & 0xFF;
        }
        bootloader_random_disable();
```

### CR-03: Stack Memory Disclosure / Invalid IP Parsing

**File:** `main/web_server.c:233`
**Issue:** `get_client_ip` assumes `getpeername` always populates a `struct sockaddr_in6`. If the client connects over IPv4, `getpeername` populates a `struct sockaddr_in` (16 bytes). `inet_ntop(AF_INET6, &addr.sin6_addr...)` then reads 16 bytes of uninitialized stack memory immediately following the IPv4 address. This results in garbage IPv6 addresses, causing unpredictable rate-limiting failures, session validation failures (`ip` claim mismatch), and potential stack data leakage.
**Fix:**
```c
    if (getpeername(sockfd, (struct sockaddr *)&addr, &addr_size) == 0) {
        if (addr.sin6_family == AF_INET) {
            struct sockaddr_in *addr4 = (struct sockaddr_in *)&addr;
            inet_ntop(AF_INET, &addr4->sin_addr, ipstr, ipstr_len);
        } else if (addr.sin6_family == AF_INET6) {
            inet_ntop(AF_INET6, &addr.sin6_addr, ipstr, ipstr_len);
        }
    }
```

## Warnings

### WR-01: Potential Null Pointer Dereferences in NVS Getters

**File:** `components/nvs_store/nvs_store.c:217`
**Issue:** `nvs_get_str(handle, NVS_KEY_SSID, ssid_out, &len) == ESP_OK && ssid_out[0]` is unsafe if `ssid_out` is `NULL`. `nvs_get_str` handles `NULL` output gracefully, returning `ESP_OK` and the required length. However, the subsequent check `ssid_out[0]` will dereference `NULL` and crash. The same issue exists on line 220 (`ssid_out[0] = '\0'`) and in `nvs_store_load_sta_ip` for `ip_out[0]`.
**Fix:** Add `NULL` checks before dereferencing array indices.
```c
    if (nvs_get_str(handle, NVS_KEY_SSID, ssid_out, &len) == ESP_OK && ssid_out && ssid_out[0]) {
        has_ssid = true;
    } else if (ssid_out) {
        ssid_out[0] = '\0';
    }
```

### WR-02: Rate Limiting Array Thread-Safety and LRU Evasion

**File:** `main/web_server.c:1882`
**Issue:** The `s_rate_limits` LRU cache tracks login attempts per IP. The size `LOGIN_LRU_SIZE` is very small (4). An attacker rotating 5 different IPs can flush the LRU queue and bypass the rate limit. Additionally, the array is not protected by a mutex, leading to potential race conditions if multiple login requests are handled concurrently.
**Fix:** Increase `LOGIN_LRU_SIZE` to 16, and wrap access with `taskENTER_CRITICAL(&s_web_diag_lock)` / `taskEXIT_CRITICAL` (or a dedicated mutex).

## Info

### IN-01: Unnecessary base64 padding calculation

**File:** `components/session/session.c:46`
**Issue:** Manual padding calculation loop for base64 decoding could be simplified, though it functions correctly as written.
**Fix:** Keep as is if it passes validation, but consider optimizing using standard URL-safe decoding logic.

---

_Reviewed: 2024-06-11T10:16:00Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
