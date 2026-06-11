# Pitfalls Research

**Domain:** Authentication & Recovery in ESP-IDF Firmware
**Researched:** 2026-06-11
**Confidence:** HIGH

## Critical Pitfalls

### Pitfall 1: NVS Flash Wear from Token Rotation

**What goes wrong:**
The device flash memory degrades rapidly, eventually causing NVS write failures, random crashes, or boot loops. 

**Why it happens:**
Developers implement "rolling sessions" (where the persistent session token is regenerated and saved on every request or every login) directly into NVS without realizing that ESP32 flash has a limited number of erase cycles (typically 100k).

**How to avoid:**
Do not write to NVS on every request. Issue a single, long-lived "Remember Me" token that is saved in NVS once upon login. For active session tracking, keep ephemeral tokens in RAM (using the existing `session.c` component) and only validate the NVS token when the RAM session expires or the device reboots.

**Warning signs:**
High frequency of `nvs_set_str()` calls in the logs; `ESP_ERR_NVS_NOT_ENOUGH_SPACE` errors appearing during uptime.

**Phase to address:**
Phase 1: Persistent Sessions ("Remember Me")

---

### Pitfall 2: "Nuke the World" Recovery Button

**What goes wrong:**
The user presses the hardware recovery button intending to reset their forgotten password, but the device also forgets its Wi-Fi configuration, pump timer settings, and cooling parameters.

**Why it happens:**
It is easier to call `nvs_flash_erase()` on the entire partition than it is to surgically remove specific keys or namespaces.

**How to avoid:**
The recovery mechanism must specifically target only the authentication NVS keys (e.g., deleting or resetting the `admin_user` and `admin_pass` keys in the `app_config` namespace). Do not erase the `wifi_cfg` or `pump_cfg` namespaces.

**Warning signs:**
The recovery function calls `nvs_flash_erase()` instead of `nvs_erase_key()` or `nvs_set_str()` for just the credentials.

**Phase to address:**
Phase 3: Credential Recovery Mechanism

---

### Pitfall 3: Offline Time Drift Breaking Expirations

**What goes wrong:**
Persistent sessions immediately expire upon reboot, or never expire, making the "Remember Me" feature either useless or a permanent security risk.

**Why it happens:**
ESP32 lacks a battery-backed RTC. When operating offline (e.g., in SoftAP mode or without SNTP), the system time resets to the UNIX epoch on every boot. Using absolute wall-clock time (`time_t`) to validate token expiration will fail.

**How to avoid:**
For a local, offline device, do not rely on absolute timestamps for token expiration. Instead, treat the "Remember Me" token as valid until explicitly revoked (Logout or Password Change), or use uptime (`esp_timer_get_time()`) combined with an active session window rather than absolute wall-clock expiration.

**Warning signs:**
Cookie attributes using `Expires=` or code checking `time(NULL)` against a saved timestamp.

**Phase to address:**
Phase 1: Persistent Sessions ("Remember Me")

---

### Pitfall 4: Weak Entropy for Persistent Tokens

**What goes wrong:**
An attacker can easily guess valid session tokens and bypass the login screen without credentials.

**Why it happens:**
The developer uses the standard C `rand()` function (which is predictable) or unseeded logic to generate the long-lived token.

**How to avoid:**
Always use `esp_random()` to generate token bytes. Ensure Wi-Fi/RF is enabled before calling it to guarantee cryptographic entropy, then encode it to hex or base64.

**Warning signs:**
Calls to `rand()` or predictable timestamps used as token values.

**Phase to address:**
Phase 1: Persistent Sessions ("Remember Me")

---

## Technical Debt Patterns

Shortcuts that seem reasonable but create long-term problems.

| Shortcut | Immediate Benefit | Long-term Cost | When Acceptable |
|----------|-------------------|----------------|-----------------|
| Plaintext Credentials in NVS | Faster implementation, no hashing library needed. | Flash dumps expose the password. | Acceptable in MVP if flash encryption is planned later, but must be documented. |
| Non-HttpOnly Persistent Cookie | Easier JS interaction. | High risk of XSS stealing the long-lived token. | Never acceptable for *persistent* tokens. Only for ephemeral UI state. |
| Hardcoded Recovery GPIO | Quick hardware hookup. | Inflexible if the board layout changes. | Only acceptable if defined in `app_config.h` alongside other GPIOs. |

## Integration Gotchas

Common mistakes when connecting to external services (or in this case, internal modules).

| Integration | Common Mistake | Correct Approach |
|-------------|----------------|------------------|
| NVS Storage | Calling NVS APIs from HTTP request context without mutexes. | NVS is thread-safe, but concurrent reads/writes of interdependent keys should be atomic or guarded. |
| HTTP Server | Storing the persistent token validation in every route handler. | Implement auth validation as middleware or a central helper before route logic. |
| UI Forms | Changing password with `type="text"`. | Use `type="password"` with a toggle visibility button and a "Confirm Password" field. |

## Performance Traps

Patterns that work at small scale but fail as usage grows (or on embedded hardware).

| Trap | Symptoms | Prevention | When It Breaks |
|------|----------|------------|----------------|
| Large Token Strings | Stack overflow in HTTP handlers when parsing cookies. | Keep persistent tokens short (e.g., 32 hex chars / 16 bytes). | If token exceeds 128 bytes. |
| NVS Read on Every HTTP Request | Latency spikes on dashboard load. | Load the expected persistent token hash into RAM on boot; compare against RAM. | On heavy polling (e.g., `/api/status`). |

## Security Mistakes

Domain-specific security issues beyond general web security.

| Mistake | Risk | Prevention |
|---------|------|------------|
| Incomplete Revocation | Clicking "Logout" deletes the client cookie but leaves the token valid in NVS. | Logout must invalidate the NVS token (e.g., zeroing it out or rotating it). |
| Predictable Session Fixation | Issuing the same persistent token across password changes. | Changing the password MUST invalidate all existing persistent and ephemeral tokens. |
| Boot-time GPIO Recovery Vulnerability | Any noise on the recovery GPIO triggers a password reset. | Require a long press (e.g., 5-10 seconds) during uptime, with debouncing and visual confirmation. |

## UX Pitfalls

Common user experience mistakes in this domain.

| Pitfall | User Impact | Better Approach |
|---------|-------------|-----------------|
| Silent "Remember Me" Failure | User checks the box, but is logged out anyway on reboot. | Show an error if NVS write fails, and ensure the UI reflects the actual saved state. |
| No Feedback on Recovery | User holds the recovery button but doesn't know if it worked until they try to log in. | Use the device status LED (or TFT display) to visually blink/confirm when credentials are reset. |
| Confusing Login Error | Generic "Error" message when credentials are changed on another device. | Provide clear "Session expired, please log in again" messaging if the persistent token is revoked. |

## "Looks Done But Isn't" Checklist

Things that appear complete but are missing critical pieces.

- [ ] **Persistent Session:** Often missing complete revocation — verify that clicking "Logout" actually destroys the token in NVS, not just the browser cookie.
- [ ] **Change Password:** Often missing active session invalidation — verify that changing the password immediately logs out any other active sessions.
- [ ] **Recovery Mechanism:** Often missing debounce/time-delay — verify that a quick accidental bump of the recovery button does not reset credentials.
- [ ] **UI Sync:** Often missing Remember Me checkbox state — verify that if a user has a persistent session, the login UI properly bypasses or pre-checks the box if shown.

## Recovery Strategies

When pitfalls occur despite prevention, how to recover.

| Pitfall | Recovery Cost | Recovery Steps |
|---------|---------------|----------------|
| NVS Wear / Corruption | HIGH | Re-flash device with `idf.py erase-flash`. Lose all settings. |
| Forgotten Plaintext Password | LOW | Use the implemented hardware recovery button. |
| Stolen Persistent Cookie | MEDIUM | Administrator must trigger "Change Password" to invalidate tokens. |

## Pitfall-to-Phase Mapping

How roadmap phases should address these pitfalls.

| Pitfall | Prevention Phase | Verification |
|---------|------------------|--------------|
| NVS Flash Wear from Token Rotation | Phase 1: Persistent Sessions | Code review confirms NVS write only happens on initial login with "Remember Me" checked. |
| Offline Time Drift Breaking Expirations | Phase 1: Persistent Sessions | Unplug device, reboot, verify persistent login still works without internet. |
| Incomplete Revocation | Phase 1: Persistent Sessions & Phase 2: Credential UI | Test that clicking Logout or changing the password prevents the old token from working. |
| "Nuke the World" Recovery Button | Phase 3: Recovery Mechanism | Trigger recovery, then verify Wi-Fi connects and Pump Settings remain intact. |
| Boot-time GPIO Recovery Vulnerability | Phase 3: Recovery Mechanism | Tap the button briefly; verify password is NOT reset. Hold for 10s; verify reset. |

## Sources

- ESP-IDF NVS Documentation (Flash wear limitations)
- OWASP Session Management Cheat Sheet
- Hardware debouncing and user feedback best practices
- Project Context: Offline requirements, SoftAP fallback, lack of RTC

---
*Pitfalls research for: Authentication & Recovery in ESP-IDF Firmware*
*Researched: 2026-06-11*
