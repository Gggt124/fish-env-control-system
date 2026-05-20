---
phase: 03-authenticated-pump-control-api
phase_number: "03"
phase_name: authenticated-pump-control-api
status: issues_found
depth: standard
files_reviewed: 1
scope_source: SUMMARY.md
reviewed_files:
  - main/web_server.c
findings:
  critical: 0
  warning: 2
  info: 0
  total: 2
created: 2026-05-20
---

# Phase 03 Code Review: Authenticated Pump Control API

## Summary

Reviewed Phase 3 source scope from the phase summary artifacts:

- `main/web_server.c`

The pump API route registration and basic auth gates are present, and the start/stop handlers preserve the fail-safe relay inactive checks. Two warning-level issues should be fixed before relying on these routes for hardware operation.

## Findings

### WR-001: Timer JSON validation casts unbounded doubles to `uint32_t`

**Severity:** Warning  
**File:** `main/web_server.c`  
**Lines:** 408-421  
**Category:** Correctness / input validation

`api_pump_required_u32()` casts `item->valuedouble` to `uint32_t` before proving the value is within the representable range:

```c
item->valuedouble != (double)(uint32_t)item->valuedouble
...
(uint32_t)item->valuedouble < APP_TEMPLATE_PUMP_TIMER_MIN_SEC
```

For a JSON number such as `1e100`, converting the double to `uint32_t` is outside the representable range. In C this is undefined behavior, so a malformed authenticated request can exercise compiler/runtime-dependent behavior before the intended duration range check rejects it.

**Recommendation:** Check the double value against the configured timer range before any integer cast, and only cast after confirming it is finite, non-negative, integral, and `<= APP_TEMPLATE_PUMP_TIMER_MAX_SEC`. For example, reject when `valuedouble < APP_TEMPLATE_PUMP_TIMER_MIN_SEC`, `valuedouble > APP_TEMPLATE_PUMP_TIMER_MAX_SEC`, or `valuedouble != floor(valuedouble)` before assigning `*out = (uint32_t)valuedouble`.

### WR-002: Same-origin check uses substring matching for safety-critical POST routes

**Severity:** Warning  
**File:** `main/web_server.c`  
**Lines:** 166-177, 946, 1045, 1094  
**Category:** Security / request authorization

Phase 3 added pump configuration, start, and stop POST handlers that rely on `is_same_origin()`. That helper accepts requests with no `Origin`/`Referer` header and treats any header containing the AP IP, STA IP, or mDNS hostname as same-origin:

```c
if (buf[0] == '\0') return true;
if (ap_ip && strstr(buf, ap_ip)) return true;
if (sta_ip && sta_ip[0] && strstr(buf, sta_ip)) return true;
if (strstr(buf, APP_TEMPLATE_MDNS_HOSTNAME ".local")) return true;
```

Substring checks can accept unrelated origins such as a host containing `192.168.4.1` as part of a longer hostname. Allowing missing origin metadata also makes the CSRF guard weaker for older browsers, embedded webviews, or hand-crafted clients. This mattered less for Wi-Fi settings, but Phase 3 now exposes actuator control routes where false positives can start or stop the pump.

**Recommendation:** Parse `Origin` or `Referer` as a URL and compare the host exactly against the allowed AP IP, STA IP, and `fish-pump.local` host, with optional explicit port handling. For pump start/stop/config POSTs, reject missing origin metadata unless there is a deliberate non-browser client path with a separate token or API-specific exception.

## Positive Checks

- All five Phase 3 routes are registered in `web_server_start()`: `GET/POST /api/pump/config`, `GET /api/pump/status`, `POST /api/pump/start`, and `POST /api/pump/stop`.
- Pump API handlers consistently require authentication before serving protected data or mutating runtime state.
- Start is idempotent and does not reset the running timer state when already running.
- Stop verifies the relay is inactive before returning success.
- Config save persists settings before runtime apply, then restarts only when the controller was running before the save.

## Verification Notes

- Review depth: standard.
- Scope source: Phase 3 `*-SUMMARY.md` artifacts.
- Automated build was not rerun during this review; Phase 3 summary already records a successful `.\scripts\build.ps1` run.
