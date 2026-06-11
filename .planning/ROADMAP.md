# Roadmap: Fish Pump Relay Timer Control

## Milestones

- 🟡 **v1.7 Authentication & Recovery** - Phases 1-3 (planned)

## Phases

- [ ] **Phase 1: Persistent Sessions & Storage Foundation** - Establish the core security token architecture required for authentication
- [ ] **Phase 2: Credential Management UI & Logic** - Allow users to securely update the default password
- [ ] **Phase 3: Hardware Recovery & Anti-Lockout** - Provide physical fail-safes for lost credentials and connectivity loss

## Phase Details

### Phase 1: Persistent Sessions & Storage Foundation

**Goal**: Establish secure, persistent session token architecture to support "Remember Me" without NVS wear.
**Depends on**: Nothing (first phase of v1.7)
**Requirements**: AUTH-01, AUTH-02, AUTH-03, AUTH-04, SEC-01, SEC-02
**Success Criteria** (what must be TRUE):

  1. User can check "Remember Me" during login to stay authenticated across device reboots.
  2. User session remains valid despite offline time drift.
  3. System gracefully recovers if NVS partition is corrupted or needs an upgrade.
  4. Oldest sessions are cleanly evicted if user logs in on more than 5 devices.

**Plans:** 2/3 plans executed

- [x] 01-01-PLAN.md — Core NVS Storage Foundation
- [x] 01-02-PLAN.md — Stateless JWT Crypto
- [ ] 01-03-PLAN.md — "Remember Me" Integration

### Phase 2: Credential Management UI & Logic

**Goal**: Users can securely change the default administrator credentials and invalidate old sessions.
**Depends on**: Phase 1
**Requirements**: AUTH-05, AUTH-06, AUTH-07
**Success Criteria** (what must be TRUE):

  1. User can access a "Change Password" UI from the dashboard.
  2. User can successfully change the username and password, requiring the old password.
  3. Changing password immediately invalidates all active sessions (user is forced to log in again).
  4. System rejects replay attacks during credential change.

**Plans**: TBD
**UI hint**: yes

### Phase 3: Hardware Recovery & Anti-Lockout

**Goal**: Provide physical hardware fail-safes for lost credentials and connectivity loss.
**Depends on**: Phase 2
**Requirements**: RECOV-01, RECOV-02, RECOV-03
**Success Criteria** (what must be TRUE):

  1. User can hold the recovery button to reset credentials to `admin`/`admin123` without losing other settings.
  2. SoftAP fallback only opens when user physically presses the recovery button.
  3. If new credentials or Wi-Fi settings fail validation, the device reboots and rolls back to previous known-good settings.

**Plans**: TBD

## Progress

**Execution Order:**
Phases execute in numeric order: 1 ➔ 2 ➔ 3

| Phase | Milestone | Plans Complete | Status | Completed |
|-------|-----------|----------------|--------|-----------|
| 1. Persistent Sessions & Storage Foundation | v1.7 | 2/3 | In Progress|  |
| 2. Credential Management UI & Logic | v1.7 | 0/0 | Not started | - |
| 3. Hardware Recovery & Anti-Lockout | v1.7 | 0/0 | Not started | - |
