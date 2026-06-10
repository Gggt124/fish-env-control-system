# Architecture Patterns

**Domain:** Authentication & Recovery in ESP-IDF Firmware
**Researched:** 2026-06-11

## Recommended Architecture

### Component Boundaries

| Component | Responsibility | Communicates With |
|-----------|---------------|-------------------|
| `session.c` | Token generation, validation, memory tracking. | `nvs_store.c` (for persistent token). |
| `nvs_store.c` | Setting/Getting credentials and persistent tokens. | ESP-IDF NVS API. |
| `web_server.c` | Route protection, checking cookies, calling `session` API. | HTTP handlers, `session.c`. |

### Data Flow

1. **Login (Remember Me):** User POSTs to `/api/login`. `web_server` validates credentials against `nvs_store`. If valid and "Remember Me" is true, `session.c` generates a long-lived token -> saves to `nvs_store`. A cookie is returned.
2. **Session Validation:** On subsequent requests, `web_server` reads cookie. If RAM session expired, it checks the token against `nvs_store`.
3. **Change Password:** User POSTs to `/api/credentials`. `nvs_store` updates password. `session.c` invalidates all RAM and NVS tokens.

## Patterns to Follow

### Pattern 1: Ephemeral + Persistent Token Split
**What:** Keep short-lived tokens in RAM and long-lived tokens in NVS.
**When:** Implementing "Remember Me" on flash-limited devices.

### Pattern 2: Surgical NVS Recovery
**What:** The recovery button explicitly deletes `admin_user` and `admin_pass` keys, leaving `wifi_cfg` untouched.
**When:** The user triggers a password reset.

## Anti-Patterns to Avoid

### Anti-Pattern 1: NVS Wear Token Rotation
**What:** Updating the persistent token in NVS on every request.
**Why bad:** Destroys flash memory after ~100k cycles.
**Instead:** Update NVS only on explicit login, logout, or password change.

## Scalability Considerations

| Concern | At 1 User | At 5 Users | At 100 Users |
|---------|--------------|--------------|-------------|
| Token Storage | 1 token in NVS | Fixed array of 5 in NVS | Do not use NVS; switch to JWT or database. |

## Sources

- ESP-IDF NVS Wear Leveling Guidelines
