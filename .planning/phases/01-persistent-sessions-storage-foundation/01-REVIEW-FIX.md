---
phase: 01
fixed_at: 2026-06-11T08:38:51Z
review_path: .planning/phases/01-persistent-sessions-storage-foundation/01-REVIEW.md
iteration: 1
findings_in_scope: 5
fixed: 5
skipped: 0
status: all_fixed
---

# Phase 01: Code Review Fix Report

**Fixed at:** 2026-06-11T08:38:51Z
**Source review:** .planning/phases/01-persistent-sessions-storage-foundation/01-REVIEW.md
**Iteration:** 1

**Summary:**
- Findings in scope: 5
- Fixed: 5
- Skipped: 0

## Fixed Issues

### CR-01: CSRF Bypass via Long Origin/Referer Header

**Files modified:** main/web_server.c
**Commit:** b6fc316
**Applied fix:** Replaced the vulnerable length check logic in is_same_origin to correctly reject overly long Origin/Referer headers instead of falling back to the missing header allowance, effectively mitigating the CSRF bypass.

### CR-02: JWT HMAC Timing Attack Vulnerability

**Files modified:** components/session/session.c
**Commit:** d34bab7
**Applied fix:** Replaced strcmp with a constant-time comparison loop in session_validate to verify the HMAC signature securely without leaking timing information.

### CR-03: JWT Payload Truncation Bypass

**Files modified:** components/session/session.c
**Commit:** 6d574aa
**Applied fix:** Increased the payload buffer size to 256 bytes in both token creation and validation, and signing_input to 512 bytes. Updated the iat field logic to explicitly fail validation if the field is missing when expiration is enabled, preventing the bypass condition when JSON gets truncated.

### WR-01: Silent Wi-Fi Password Truncation

**Files modified:** main/web_server.c
**Commit:** 1329edd
**Applied fix:** Increased the password buffer from `char password[64]` to `char password[65]` in handle_api_wifi_connect so that a 64-character PMK plus null terminator is correctly loaded into the buffer.

### WR-02: Global Rate Limit Allows Denial of Service

**Files modified:** README.md
**Commit:** 45da256
**Applied fix:** Documented the DoS risk associated with the global rate limit in the README.md Security Notes section, as this is an acceptable tradeoff for a local SoftAP environment.

---

_Fixed: 2026-06-11T08:38:51Z_
_Fixer: the agent (gsd-code-fixer)_
_Iteration: 1_
