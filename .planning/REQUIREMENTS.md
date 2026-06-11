# Requirements: v1.7

**Milestone:** v1.7 (Authentication & Recovery)

## 1. Authentication & Sessions (AUTH)

- [ ] **AUTH-01:** Persistent Sessions ("Remember Me").
- [ ] **AUTH-02:** Stateless JWT Validation. Use a single Secret Key stored in NVS, verified in RAM. (Overrides 5-device Token Ring Buffer limit per D-02).
- [ ] **AUTH-03:** Non-expiring tokens. Tokens are valid until explicit logout or password change. (Overrides RTR & Monotonic Time per D-03).
- [ ] **AUTH-04:** Token Binding. Tokens must be cryptographically bound to the client's Local IP. (Overrides User-Agent binding per D-04).
- [ ] **AUTH-05:** Credential Management UI. Web dashboard must allow changing the administrator username and password.
- [ ] **AUTH-06:** Global Session Invalidation. Changing the password must increment a `session_gen` counter in NVS, instantly invalidating all existing persistent tokens.
- [ ] **AUTH-07:** Challenge-Response Nonce. Critical parameter/password changes must use a one-time cryptographic nonce to prevent replay attacks.

## 2. Recovery & Anti-Lockout (RECOV)

- [ ] **RECOV-01:** Hardware Factory Reset. Pressing a designated GPIO button (active-low, 10k pull-up, debounced) for 5-10 seconds must reset credentials to `admin`/`admin123` without affecting Wi-Fi or pump configurations.
- [ ] **RECOV-02:** Timed Setup with Physical Consent (Secure AP Fallback). SoftAP (`FishPump-Setup`) must NOT open automatically on STA failure. It only opens for 5 minutes after the user physically presses the recovery button for 2 seconds.
- [ ] **RECOV-03:** Anti-Lockout Validation Rollback. Credential/Wi-Fi updates write to a temporary NVS staging namespace. The ESP32 reboots and waits 3 minutes for a user "Confirm" API call. If unconfirmed, it rolls back to the previous known-good credentials.

## 3. Storage & Integrity (SEC)

- [ ] **SEC-01:** NVS Corruption Recovery. On boot, if `nvs_flash_init` returns `ESP_ERR_NVS_NO_FREE_PAGES` or `ESP_ERR_NVS_NEW_VERSION_FOUND`, the system must gracefully erase and re-initialize the NVS partition.
- [ ] **SEC-02:** Safe Flash Commits. NVS write operations for credentials must be properly finalized and flushed to prevent corruption during unexpected power loss.

## Future Requirements

- **FUT-01:** MbedTLS SHA-256 Hashing for NVS passwords (Deferred to security hardening phase).
- **FUT-02:** Flash Encryption & Secure Boot v2 integration.

## Out of Scope

- OAuth, multi-user Role-Based Access Control (RBAC).
- HTTPS / SSL Certificates for the local web server.

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| AUTH-01 | Phase 1 | Complete |
| AUTH-02 | Phase 1 | Complete |
| AUTH-03 | Phase 1 | Complete |
| AUTH-04 | Phase 1 | Complete |
| AUTH-05 | Phase 2 | Pending |
| AUTH-06 | Phase 2 | Pending |
| AUTH-07 | Phase 2 | Pending |
| RECOV-01 | Phase 3 | Pending |
| RECOV-02 | Phase 3 | Pending |
| RECOV-03 | Phase 3 | Pending |
| SEC-01 | Phase 1 | Complete |
| SEC-02 | Phase 1 | Complete |
