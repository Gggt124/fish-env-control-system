# Phase 3: Authenticated Pump Control API - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md - this log preserves the alternatives considered.

**Date:** 2026-05-20
**Phase:** 3-Authenticated Pump Control API
**Areas discussed:** Route Shape, Config Contract, Start/Stop Response, Error Style, Status Detail

---

## Route Shape

| Question | Selected | Alternatives Considered |
|----------|----------|-------------------------|
| Pump API route namespace | Dedicated `/api/pump/*` namespace | Add pump summary to `/api/status`; use shorter generic routes |
| Config endpoint shape | `GET /api/pump/config` and `POST /api/pump/config` | `GET/PUT`; split `/config` and `/config/save` |
| Pump status location | Use only `GET /api/pump/status` | Add summary to existing `/api/status`; add only lightweight fields to `/api/status` |
| Start/stop route shape | Separate `POST /api/pump/start` and `POST /api/pump/stop` | Single command endpoint; declarative state endpoint |

**User's choice:** Use dedicated pump routes and keep the existing system status endpoint unchanged.
**Notes:** The user selected the recommended option for all Route Shape questions.

---

## Config Contract

| Question | Selected | Alternatives Considered |
|----------|----------|-------------------------|
| Config response fields | Return timer durations, `auto_start`, `relay_polarity`, and read-only GPIO/debounce defaults | Editable fields only; separate settings and hardware objects |
| Timer duration format | Seconds only | Minutes/seconds object; both response formats with seconds-only save |
| Read-only fields in save payload | Reject with a read-only-field error | Ignore silently; accept only if equal to defaults |
| Save payload completeness | Require all editable fields as a full replacement | Partial update; mixed full/partial mode |

**User's choice:** Full replacement config API using seconds-only editable fields, with hardware/debounce fields visible but read-only.
**Notes:** The user selected the recommended option for all Config Contract questions.

---

## Start/Stop Response

| Question | Selected | Alternatives Considered |
|----------|----------|-------------------------|
| Start while already running | `ok:true` with `already_running:true` | Plain `ok:true`; conflict/error |
| Stop while already stopped | `ok:true` with `already_stopped:true` | Plain `ok:true`; conflict/error |
| Success response content | Include pump status snapshot | Return only action result; return only partial status fields |
| Start failure handling | Confirm fail-safe stop/relay inactive, return error and status if available | Return error only; auto re-init default config |

**User's choice:** Keep start/stop idempotent and make responses useful for UI updates and debugging.
**Notes:** The user selected the recommended option for all Start/Stop Response questions.

---

## Error Style

| Question | Selected | Alternatives Considered |
|----------|----------|-------------------------|
| Config validation HTTP status | HTTP 400 with `ok:false` | HTTP 200 with `ok:false`; mixed severity handling |
| Runtime/internal failure status | HTTP 500 for internal failures and HTTP 409 for state/config conflicts | HTTP 200 for all business failures; HTTP 503 for runtime unavailable |
| Error body detail | Stable `error` code plus short `message` | Error code only; message only |
| Multiple validation errors | Return the first error only | Return an errors array; return first error plus field name |

**User's choice:** Use clearer HTTP status codes and stable error bodies for pump APIs.
**Notes:** The user selected the recommended option for all Error Style questions.

---

## Status Detail

| Question | Selected | Alternatives Considered |
|----------|----------|-------------------------|
| Status field type | Machine-friendly fields | Machine plus display-ready fields; display-ready fields as primary |
| Enum/string values | Lowercase stable strings | Numeric enum values; both string and numeric |
| Config included in status | Runtime fields plus `auto_start`; no full timer config duplication | Runtime only; full config summary |
| Settings/load health | Include `settings_status` | Omit in Phase 3; include only `config_valid` |

**User's choice:** Make status machine-friendly and stable, with enough health information for diagnostics.
**Notes:** The user selected the recommended option for all Status Detail questions.

---

## the agent's Discretion

- Exact C helper function names.
- Exact JSON object nesting where not explicitly locked.
- Exact final stable error-code names, provided the selected error semantics are preserved.

## Deferred Ideas

None - discussion stayed within phase scope.
