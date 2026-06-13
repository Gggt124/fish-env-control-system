---
phase: 03
reviewers: [opencode, antigravity]
reviewed_at: 2026-06-13T18:55:00+07:00
plans_reviewed: [03-01-PLAN.md, 03-02-PLAN.md, 03-03-PLAN.md, 03-04-PLAN.md]
---

# Cross-AI Plan Review — Phase 03

## OpenCode Review

# OpenCode Review

## Summary
The plans outline a comprehensive recovery strategy that combines physical hardware interaction with software fallbacks. The use of NVS staging is well thought out, ensuring that invalid configurations don't brick the device. However, there are significant logic gaps in the sequence of session management during credential changes and UI integration.

## Strengths
- **Safe Fallback:** The 3-minute NVS rollback timer in `app_main.c` is a highly resilient way to prevent lockout.
- **Hardware Tasks:** The dedicated FreeRTOS task for button polling with debouncing and mutual exclusion is a robust embedded pattern.
- **API Protection:** Securing `/api/confirm` ensures only authenticated users can commit staged settings.

## Concerns
- **HIGH:** Missing Frontend Flow. If the web UI does not call `/api/confirm` after a credential update, the device will always reboot and roll back after 3 minutes. This renders the credential update feature completely unusable.
- **HIGH:** Session Invalidation Flaw. While a factory reset increments `session_gen`, regular credential updates (via `nvs_store_commit`) do not appear to increment `session_gen`. This means a changed password does not invalidate existing sessions.
- **MEDIUM:** Wi-Fi Commit Ambiguity. Plan 01 commits Wi-Fi automatically on `WIFI_EVENT_STA_GOT_IP`. It should be clear that `/api/confirm` is not needed for Wi-Fi, or else the UI might unnecessarily prompt the user to confirm a Wi-Fi change that was already committed.

## Suggestions
- Ensure `nvs_store_commit()` increments `session_gen` when committing a credential change.
- Detail the frontend changes required to present and call the `/api/confirm` endpoint.
- Add an API endpoint to cancel the rollback timer, or ensure it's cancelled upon successful commit.

## Risk Assessment
**HIGH**. The lack of frontend confirmation and the session invalidation loophole pose critical functional and security risks that must be fixed before execution.

---

## Antigravity Review

# Antigravity Review

## Summary
The plans provide a solid and thoughtful approach to implementing physical hardware recovery and preventing accidental lockout. The use of an NVS staging mechanism to validate Wi-Fi and credential changes before committing them is an excellent pattern. However, there are critical gaps in how the frontend interacts with the new `/api/confirm` endpoint, and a missing security step regarding session invalidation during normal credential updates.

## Strengths
- **NVS Staging:** The 3-state NVS staging mechanism (`staging_type`) perfectly isolates risky configuration changes.
- **Hardware Integration:** The recovery task effectively handles button debouncing, mutual exclusion, and clear LED feedback.
- **Resilience:** Relying on standard FreeRTOS timers and the ESP-IDF event loop ensures non-blocking and reliable timeout behavior.
- **Safety First:** The 30-second Wi-Fi IP wait and 3-minute credential confirmation timers are robust defenses against permanent lockout.

## Concerns
- **HIGH:** Missing Frontend Integration for `/api/confirm`. None of the plans modify the frontend HTML/JS to actually invoke the new `/api/confirm` endpoint. For credential changes, if the UI doesn't call this after the user logs back in, the device will unconditionally reboot and roll back after 3 minutes, making it impossible to permanently change the password.
- **HIGH:** Session Invalidation Bypass. Plan 02 specifies that `nvs_store_factory_reset_credentials` increments `session_gen` to invalidate old sessions, but it fails to mention incrementing `session_gen` inside `nvs_store_commit` when `staging_type == 2`. Without this, changing the password via the web UI will leave all existing sessions (created with the old password) valid.
- **MEDIUM:** Wi-Fi Auto-Commit vs API Confirm. Plan 01 automatically calls `nvs_store_commit()` on `WIFI_EVENT_STA_GOT_IP`. If it auto-commits, the frontend doesn't need to call `/api/confirm` for Wi-Fi. However, this discrepancy should be explicitly documented so the frontend logic is clear on when `/api/confirm` is required.
- **LOW:** FreeRTOS Timer Lifecycles. In `03-04-PLAN.md`, the 3-minute rollback timer should be explicitly deleted or stopped if `nvs_store_commit()` is called via `/api/confirm`, otherwise it might fire unexpectedly or leak memory if not handled cleanly.

## Suggestions
- **Update Plan 03:** Add a task to update `main/static/app.js` (and potentially the HTML files). When the frontend detects a successful login after a credential change, it must automatically call `/api/confirm` or present a "Confirm New Password" button.
- **Update Plan 02:** Modify the `nvs_store_commit()` logic to increment `session_gen` whenever `staging_type == 2` is committed, ensuring old sessions are invalidated.
- **Update Plan 03:** Ensure the `/api/confirm` handler stops the 3-minute rollback timer created in `app_main.c` (e.g., by exposing a `recovery_cancel_rollback_timer()` function).
- **Update Plan 01:** Clarify that Wi-Fi changes do not require `/api/confirm` because acquiring an IP is sufficient proof of connectivity.

## Risk Assessment
**HIGH**
Without frontend integration for `/api/confirm`, the credential update feature is fundamentally broken (it will always roll back). Additionally, the missing `session_gen` increment upon commit leaves a severe security hole where old sessions remain valid after a password change. These must be addressed before implementation.

---

## Consensus Summary

Both reviewers independently identified the same two critical flaws in the current plans. The proposed NVS staging and rollback mechanisms are architecturally sound, but their integration with the frontend and session management logic is incomplete.

### Agreed Strengths
- The NVS staging mechanism (`staging_type`) is a robust way to isolate unverified changes.
- The hardware recovery task provides clear LED feedback and safely handles debouncing/mutual exclusion.
- The 30-second Wi-Fi and 3-minute credential rollback timers offer strong protection against permanent lockouts.

### Agreed Concerns
- **HIGH:** Missing frontend integration. None of the plans detail modifying `app.js` or the UI to call `/api/confirm`. This guarantees that credential changes will always roll back after 3 minutes.
- **HIGH:** Session invalidation bypass. `nvs_store_commit` does not increment `session_gen` when committing a credential change (`staging_type == 2`), leaving old sessions valid after a password change.

### Divergent Views
- No significant disagreements. Both reviewers raised the same issues regarding the commit flow and session management. Both also noted ambiguity regarding whether Wi-Fi changes require `/api/confirm` since Plan 01 automatically commits upon getting an IP.
