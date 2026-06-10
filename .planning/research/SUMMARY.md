# Project Research Summary

**Project:** Fish Pump Relay Timer Control
**Domain:** Authentication & Recovery in ESP-IDF Firmware
**Researched:** 2026-06-11
**Confidence:** HIGH

## Executive Summary

The project involves adding authentication and recovery mechanisms to an offline ESP32-based Fish Pump Relay Timer Control system. The goal is to establish secure persistent sessions and credential recovery while operating within the limitations of embedded flash memory and the lack of a real-time clock (RTC).

The recommended approach uses a split token system: short-lived sessions in RAM and a single, long-lived "Remember Me" token stored in the native Non-Volatile Storage (NVS). Credential recovery should be handled via a hardware GPIO button to provide physical access overrides, ensuring only authentication data is reset. 

The primary risks include NVS flash wear from overly aggressive token rotation, offline time drift breaking absolute token expirations, and a "Nuke the World" recovery implementation that deletes all device settings instead of just the credentials. Mitigations involve updating NVS only on explicit login/logout, using uptime (`esp_timer_get_time()`) instead of absolute clock time, and performing surgical NVS recovery.

## Key Findings

### Recommended Stack

The core framework leverages ESP-IDF native features for simplicity and efficiency, avoiding external heavy databases.

**Core technologies:**
- **ESP-IDF NVS**: Persistent Storage — Built-in ESP32 Non-Volatile Storage for persistent tokens and credentials.
- **`esp_random()`**: Cryptographic Entropy — Secure random number generation for tokens when Wi-Fi/RF is enabled.
- **cJSON**: JSON API Payloads — Used for parsing the Change Password POST request and formatting auth responses.

### Expected Features

**Must have (table stakes):**
- Change Password — Default `admin`/`admin123` must be changeable.
- Logout / Session Invalidation — Clicking Logout must explicitly destroy active tokens in both RAM and NVS.
- Hardware Reset — Users forget passwords; physical access should grant credential recovery.

**Should have (competitive):**
- "Remember Me" Checkbox — Bypass login on trusted personal devices for 30+ days using a persistent token.

**Defer (v2+):**
- Multi-user Roles — Unnecessary complexity for a local fish pump controller.
- Email / SMS Password Reset — Device is local-only and offline (SoftAP).

### Architecture Approach

The architecture clearly separates storage, token validation, and API routing. Keep short-lived tokens in RAM and long-lived tokens in NVS.

**Major components:**
1. **`session.c`** — Token generation, validation, memory tracking.
2. **`nvs_store.c`** — Setting/Getting credentials and persistent tokens from ESP-IDF NVS API.
3. **`web_server.c`** — Route protection, checking cookies, calling HTTP handlers and the `session` API.

### Critical Pitfalls

1. **NVS Flash Wear from Token Rotation** — Do not write to NVS on every request. Issue a long-lived "Remember Me" token saved in NVS once upon login.
2. **"Nuke the World" Recovery Button** — The recovery mechanism must specifically target only the authentication NVS keys, not erase the entire partition.
3. **Offline Time Drift Breaking Expirations** — Use uptime (`esp_timer_get_time()`) combined with an active session window rather than absolute wall-clock expiration.
4. **Weak Entropy for Persistent Tokens** — Always use `esp_random()` with Wi-Fi/RF enabled instead of standard C `rand()`.

## Implications for Roadmap

Based on research, suggested phase structure:

### Phase 1: Persistent Sessions & "Remember Me"
**Rationale:** Establishes the core security token architecture required for authentication without hitting flash wear issues.
**Delivers:** Ephemeral RAM session + persistent NVS token logic, `esp_random()` token generation, and uptime-based expiration.
**Addresses:** "Remember Me" Checkbox, Logout / Session Invalidation.
**Avoids:** NVS Flash Wear from Token Rotation, Offline Time Drift, Weak Entropy.

### Phase 2: Credential Management UI & Logic
**Rationale:** Built on the token architecture to allow users to securely update the default password.
**Delivers:** Change Password API endpoint, UI form, and session revocation on password change.
**Addresses:** Change Password feature.
**Avoids:** Incomplete Revocation, Predictable Session Fixation.

### Phase 3: Hardware Credential Recovery
**Rationale:** Provides a physical fail-safe for lost credentials after users have been given the ability to change them.
**Delivers:** GPIO button reading, debouncing, and surgical NVS credential deletion.
**Addresses:** Hardware Reset feature.
**Avoids:** "Nuke the World" Recovery Button, Boot-time GPIO Recovery Vulnerability.

### Phase Ordering Rationale

- Persistent sessions come first to lay the underlying token foundation before updating passwords or resetting them.
- Credential Management builds on the session tokens because changing a password requires revoking existing sessions.
- Hardware Credential Recovery comes last to provide the fail-safe physical override for the modified passwords.
- This grouping ensures the ESP-IDF specific patterns like NVS write wear and offline time drift are handled in the first token implementation.

### Research Flags

Phases with standard patterns (skip research-phase):
- **Phase 1, Phase 2, Phase 3:** Patterns are well-documented for ESP-IDF, and existing research clearly covers the required architecture, stack, and pitfalls.

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | Uses standard ESP-IDF native libraries and existing `cJSON`. |
| Features | HIGH | Feature set is focused, matching constraints of a local standalone device. |
| Architecture | HIGH | Split component boundaries cleanly separate storage from HTTP server logic. |
| Pitfalls | HIGH | Extensively documented embedded-specific risks with clear mitigations. |

**Overall confidence:** HIGH

### Gaps to Address

- **Hardware Pin Assignment**: The exact GPIO pin for the recovery button needs to be defined in `app_config.h` based on the ESP32 DevKit V1 30-pin board layout.
- **Plaintext vs Hashing**: Currently recommending plaintext credentials in NVS for MVP due to lack of flash encryption; if security requirements change, a hashing library (mbedtls) may be needed.

## Sources

### Primary (HIGH confidence)
- ESP-IDF NVS Documentation — Flash wear limitations
- Project Context — Offline requirements, SoftAP fallback, lack of RTC
- OWASP Session Management Cheat Sheet — Session lifecycle best practices

### Secondary (MEDIUM confidence)
- Hardware debouncing and user feedback best practices — Button recovery logic

---
*Research completed: 2026-06-11*
*Ready for roadmap: yes*
