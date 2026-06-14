# Phase 2: Credential Management UI & Logic - Research

**Researched:** 2026-06-12
**Domain:** C (ESP-IDF) firmware, NVS storage, and embedded web security
**Confidence:** HIGH

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
- **D-01:** ใช้รูปแบบ **Popup/Modal บนหน้า Dashboard** เพื่อให้ไม่ต้องโหลดหน้าใหม่หรือเปลี่ยนหน้าไปมา ช่วยให้กระบวนการเปลี่ยนรหัสผ่านรวดเร็ว ไม่รกหน้า Dashboard
- **D-02:** **ขอแค่ "รหัสผ่านเดิม" (Current Password)** ในการยืนยันตัวตนก็พอ ผู้ใช้จะได้ไม่ต้องจำและพิมพ์ Username เดิมให้ยุ่งยาก (ระบบจะดึง Username ปัจจุบันจาก Session) ส่วนฟิลด์สำหรับข้อมูลใหม่จะมีให้ระบุทั้ง Username ใหม่ และ Password ใหม่ 
- **D-03:** **แสดงข้อความแจ้งเตือนความสำเร็จ** ค้างไว้บนหน้า Dashboard ประมาณ 2-3 วินาทีเพื่อให้ผู้ใช้ทราบและรู้ตัวว่าทำรายการสำเร็จแล้ว จากนั้นระบบค่อยทำการเตะออก (Force Logout) และเปลี่ยนเส้นทาง (Redirect) ไปยังหน้า Login เพื่อบังคับล็อกอินด้วยรหัสผ่านใหม่

### the agent's Discretion
None

### Deferred Ideas (OUT OF SCOPE)
None
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| AUTH-05 | Web dashboard must allow changing the administrator username and password. | Add NVS key storage for admin credentials (`auth_cfg` namespace) with fallback to `APP_TEMPLATE_DEFAULT_USERNAME/PASSWORD`. Add a modal to `dashboard.html` (D-01, D-02). |
| AUTH-06 | Changing the password must increment a `session_gen` counter in NVS, instantly invalidating all existing persistent tokens. | Invalidate sessions by regenerating the JWT secret (`s_jwt_secret`) in NVS. This achieves the exact intent (instant global invalidation) without hand-rolling a custom `session_gen` counter payload. |
| AUTH-07 | Challenge-Response Nonce. Critical parameter/password changes must use a one-time cryptographic nonce to prevent replay attacks. | Implement `/api/auth/nonce` to issue random 16-byte hex nonces and require the exact nonce in the password change POST payload. Zero it out upon use. |
</phase_requirements>

## Summary

This phase implements the credential management UI and logic, allowing users to securely update the administrator username and password. The system currently hardcodes the default credentials (`admin`/`admin123`) from `app_config.h` in `handle_api_login`. We need to introduce an NVS storage layer for credentials, exposing a `/api/auth/credentials` route for updates.

To satisfy the global session invalidation requirement (AUTH-06), we will take advantage of the existing stateless JWT implementation. Instead of adding a `session_gen` counter to the payload, we will simply regenerate the 32-byte JWT secret (`s_jwt_secret`) in NVS whenever the password is changed. This instantly invalidates all existing issued JWTs across all devices.

For replay protection (AUTH-07), the backend will provide a `/api/auth/nonce` endpoint that generates a one-time random nonce. The frontend must fetch this nonce before submitting the password change request, embedding it in the POST payload. The server validates and consumes the nonce, preventing replay attacks.

**Primary recommendation:** Implement a new `auth_cfg` NVS namespace for credentials, regenerate the JWT secret on password change for instant invalidation, and use a simple one-time nonce system for replay protection.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| "Change Password" UI | Browser / Client | — | The UI is a SPA modal (D-01) on the dashboard. Handles validation and API interaction. |
| Credential Storage | API / Backend | Database (NVS) | The ESP32 backend stores credentials in NVS and falls back to defaults if empty. |
| Global Session Invalidation | API / Backend | — | The ESP32 backend regenerates the JWT secret, rendering old tokens cryptographically invalid. |
| Challenge-Response Nonce | API / Backend | Browser / Client | Backend generates and consumes the nonce; client fetches and submits it. |

## Standard Stack

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| ESP-IDF `nvs` | 6.0.1 | Non-Volatile Storage | Standard ESP-IDF API for persistent settings. |
| ESP-IDF `esp_random` | 6.0.1 | Hardware RNG | Required to generate cryptographically secure nonces and new JWT secrets. |
| ESP-IDF `esp_http_server` | 6.0.1 | API Endpoints | Existing web server used for routing HTTP requests. |
| Vanilla JS | ES6 | Frontend Logic | Existing project uses zero-dependency vanilla JS and standard DOM APIs. |

## Architecture Patterns

### Pattern 1: One-Time Nonce for Replay Protection
**What:** The server issues a single-use random string (nonce) that must be included in the next critical state-changing request.
**When to use:** When protecting against replay attacks over unencrypted local networks (like ESP32 AP mode without HTTPS).
**Example:**
```c
static char s_auth_nonce[33] = {0};

// On GET /api/auth/nonce
void generate_nonce() {
    uint8_t random_bytes[16];
    esp_fill_random(random_bytes, sizeof(random_bytes));
    // Convert to hex string and store in s_auth_nonce
}

// On POST /api/auth/credentials
bool verify_nonce(const char* client_nonce) {
    if (s_auth_nonce[0] == '\0' || strcmp(s_auth_nonce, client_nonce) != 0) {
        return false; // Invalid or already used
    }
    s_auth_nonce[0] = '\0'; // Consume the nonce immediately
    return true;
}
```

### Pattern 2: Global Session Invalidation via Secret Rotation
**What:** Instead of tracking session counters or individual tokens, the symmetric signing key (JWT secret) is replaced.
**When to use:** When you need a guaranteed, instantaneous global logout for stateless tokens without adding payload overhead.
**Example:**
```c
void session_regenerate_secret(void) {
    uint8_t new_secret[32];
    esp_fill_random(new_secret, sizeof(new_secret));
    nvs_store_set_jwt_secret(new_secret);
    memcpy(s_jwt_secret, new_secret, sizeof(s_jwt_secret));
}
```

### Anti-Patterns to Avoid
- **Anti-pattern:** Relying solely on `session_gen` counter for JWTs. The current system is stateless JWT, so adding `session_gen` to the JWT payload and NVS is redundant when you can just rotate the JWT secret to invalidate all tokens securely and natively.
- **Anti-pattern:** Plaintext credentials in source control. The defaults are hardcoded in `app_config.h`, but user-changed credentials must be safely persisted in NVS and NEVER leaked via logs.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Cross-Origin Protection | Custom Referer validation | `is_same_origin(req)` | `web_server.c` already has a robust origin checker mitigating CSRF. |
| Random Number Generation | `rand()` or `os_random()` | `esp_fill_random()` | Hardware RNG is required for cryptographic nonces and JWT secrets. |
| Base64 Encoding | Custom lookup tables | `mbedtls_base64_encode()` | mbedTLS is already included and used in `session.c`. |

## Common Pitfalls

### Pitfall 1: Weak Nonce Generation
**What goes wrong:** Using predictable PRNGs like `rand()` allows an attacker to guess the next nonce and execute a replay attack.
**Why it happens:** Developer uses standard C libraries instead of hardware RNG.
**How to avoid:** Always use `esp_fill_random()` or `bootloader_random_enable()` / `esp_random()` for security-critical values.

### Pitfall 2: Confusing Session Architecture Requirements
**What goes wrong:** Implementing a `session_gen` counter logic when the sessions are actually stateless JWTs.
**Why it happens:** The requirements document (`REQUIREMENTS.md`) incorrectly states that Phase 1 implemented "Stateful Slot-based Session Validation in RAM". However, Phase 1 actually fixed the JWT signature issue and retained the stateless JWT architecture.
**How to avoid:** Acknowledge the discrepancy and rotate the JWT secret to achieve the exact same functional goal (global session invalidation) securely.

### Pitfall 3: Replay Attack vs. Plaintext Sniffing
**What goes wrong:** Assuming a nonce prevents an attacker from sniffing the new password over an open Wi-Fi network.
**Why it happens:** Lack of HTTPS on the ESP32 in AP mode.
**How to avoid:** Understand that AUTH-07's nonce strictly prevents *replay* of the exact POST request. Sniffing is out of scope for this phase (future phase FUT-01 addresses MbedTLS SHA-256 Hashing). The payload will send the new password in plaintext, protected only by the one-time nonce against replay.

## Code Examples

Verified patterns from existing codebase (`components/session/session.c`):

### Cryptographically Secure Random Generation
```c
#include "esp_random.h"

// For 32-byte JWT secret
bootloader_random_enable();
for (int i = 0; i < sizeof(s_jwt_secret); i++) {
    s_jwt_secret[i] = esp_random() & 0xFF;
}
bootloader_random_disable();
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Hardcoded Credentials | NVS-Backed Credentials | This Phase | Users can change the default `admin`/`admin123` credentials securely. |
| Stateful session IDs | Stateless JWTs | Phase 1 | The project uses JWTs, so global invalidation is done by rotating the JWT secret. |

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | None - all verified | - | - |

## Open Questions (RESOLVED)

1. **Client IP Validation during password change**
   - What we know: Sessions currently validate the client IP encoded in the JWT payload.
   - What's unclear: Does changing the password and forcing a logout require the user to reconnect if their IP changed?
   - RESOLVED: Since we are forcing a logout and redirecting to the login page (D-03), IP validation is a non-issue. The new login will generate a fresh token with the current IP.

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | Manual Validation |
| Config file | none — see Wave 0 |
| Quick run command | `idf.py build` |
| Full suite command | `idf.py build` |

### Phase Requirements → Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| AUTH-05 | Web dashboard allows changing username/password | manual | `idf.py build` | ✅ Wave 0 |
| AUTH-06 | Changing password invalidates sessions (forces re-login) | manual | `idf.py build` | ✅ Wave 0 |
| AUTH-07 | POST includes valid one-time nonce | manual | `idf.py build` | ✅ Wave 0 |

### Sampling Rate
- **Per task commit:** `idf.py build`
- **Per wave merge:** `idf.py build`
- **Phase gate:** Manual UI test (login with new credentials)

## Security Domain

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2 Authentication | yes | NVS-backed credential storage |
| V3 Session Management | yes | JWT secret rotation |
| V4 Access Control | yes | Same-origin checks and Nonce |
| V5 Input Validation | yes | Manual string length / character checks |
| V6 Cryptography | yes | `esp_random()` / `esp_fill_random()` |

### Known Threat Patterns for ESP-IDF HTTP

| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| CSRF | Spoofing/Tampering | Origin/Referer check (`is_same_origin()`) |
| Replay Attack | Spoofing | One-time server-issued nonce |
| Weak Randomness | Information Disclosure | Use hardware RNG (`esp_fill_random`) |

## Sources

### Primary (HIGH confidence)
- Codebase audit: `web_server.c`, `nvs_store.c`, `session.c`.
- `REQUIREMENTS.md` and `PROJECT.md` documents.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - Extracted from existing firmware codebase.
- Architecture: HIGH - JWT secret rotation is standard industry practice for stateless tokens.
- Pitfalls: HIGH - Documented mismatch between `REQUIREMENTS.md` and `session.c` verified via git log and source code.

**Research date:** 2026-06-12
**Valid until:** 30 days
