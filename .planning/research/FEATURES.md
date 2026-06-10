# Feature Landscape

**Domain:** Authentication & Recovery in ESP-IDF
**Researched:** 2026-06-11

## Table Stakes

Features users expect. Missing = product feels incomplete.

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Change Password | Default admin/admin123 must be changeable. | Low | Requires UI form with confirmation, and API endpoint to update NVS. |
| Logout / Session Invalidation | Clicking Logout must explicitly destroy the active token. | Low | Must invalidate both the browser cookie and the persistent token in NVS. |
| Hardware Reset | Users forget passwords. Physical access should grant recovery. | Med | Requires reading a GPIO for X seconds and selectively clearing NVS. |

## Differentiators

Features that set product apart. Not expected, but valued.

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| "Remember Me" Checkbox | Bypass login on trusted personal devices for 30+ days. | Med | Requires generating and storing a long-lived cryptographically secure token. |

## Anti-Features

Features to explicitly NOT build.

| Anti-Feature | Why Avoid | What to Do Instead |
|--------------|-----------|-------------------|
| Multi-user Roles | Too complex for a local fish pump controller. | Stick to a single Admin user. |
| Email / SMS Password Reset | Device is local-only and offline (SoftAP). | Use a physical hardware button for credential recovery. |

## Feature Dependencies

Change Password → Logout/Session Invalidation (Changing password must log out other sessions)
"Remember Me" → Logout/Session Invalidation (Logout must clear the persistent token)

## MVP Recommendation

Prioritize:
1. Change Password
2. Logout / Session Invalidation
3. Hardware Reset

Defer: Multi-user Roles: Unnecessary complexity for the local use case.

## Sources

- ESP-IDF offline capabilities
