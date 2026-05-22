---
phase: 06
slug: hardware-contract-and-persistent-map-foundation
status: verified
threats_open: 0
asvs_level: 1
created: 2026-05-22
register_authored_at_plan_time: false
---

# Phase 06 - Security

> Per-phase security contract: threat register, accepted risks, and audit trail.

---

## Trust Boundaries

| Boundary | Description | Data Crossing |
|----------|-------------|---------------|
| Local web client to ESP32 HTTP server | Authenticated dashboard/API clients read pump hardware foundation fields and may save editable pump settings. | Session cookie, JSON pump config/status payloads |
| NVS storage to boot runtime | Persisted pump settings, active hardware maps, pending hardware maps, and cooling settings are loaded into firmware state at boot/request time. | GPIO numbers, relay polarities, timer start phases, cooling defaults |
| Firmware hardware map to physical GPIO wiring | Role-scoped GPIO metadata and defaults describe which ESP32 pins may be used for relay, float, DS18B20, and cooling roles. | Hardware role assignments and physical wiring assumptions |
| Installer documentation to physical deployment | `docs/hardware.md` communicates relay polarity, DS18B20 powered-mode wiring, pull-up requirements, and compressor defaults. | Human-applied wiring and safety instructions |

---

## Threat Register

| Threat ID | Category | Component | Disposition | Mitigation | Status |
|-----------|----------|-----------|-------------|------------|--------|
| T-06-01 | Tampering | `hardware_map`, `nvs_store` | mitigate | Role-specific GPIO option lists reject unsafe role/GPIO combinations and duplicate assignments; active and pending maps must pass `hardware_map_validate()` before save/load acceptance. Evidence: `components/hardware_map/hardware_map.c`, `components/nvs_store/nvs_store.c`. | closed |
| T-06-02 | Elevation of privilege / Tampering | `/api/pump/config` | mitigate | Phase 6 hardware/cooling fields are reported as read-only and rejected from POST payloads; protected pump routes still require a valid session and same-origin POST checks. Evidence: `main/web_server.c`. | closed |
| T-06-03 | Tampering / Safety | Boot-time NVS consumption | mitigate | Invalid persisted pump settings or hardware maps fall back to defaults and suppress auto-start for the current boot. Evidence: `components/nvs_store/nvs_store.c`, `main/app_main.c`. | closed |
| T-06-04 | Denial of service / Safety | Relay runtime initialization | mitigate | Pump runtime only consumes float GPIO and Relay 1; Relay 2, DS18B20, and cooling remain stored/reported only. Relay output is driven inactive during init, error paths, stabilization, and stop. Evidence: `main/app_main.c`, `components/pump_control/pump_control.c`. | closed |
| T-06-05 | Information disclosure | Pump config/status API | mitigate | New hardware map fields are exposed only through protected API routes, preserving the existing session gate. No unauthenticated hardware map endpoint was added. Evidence: `main/web_server.c`. | closed |
| T-06-06 | Safety misconfiguration | Installer documentation | mitigate | Hardware documentation warns about relay polarity, defines conservative defaults, documents DS18B20 powered-mode wiring with external 4.7 kOhm pull-up, and states cooling runtime is deferred and must fail off. Evidence: `docs/hardware.md`, `docs/development-notes.md`. | closed |

*Status: open / closed*
*Disposition: mitigate (implementation required) / accept (documented risk) / transfer (third-party)*

---

## Accepted Risks Log

No accepted risks.

---

## Security Audit Trail

| Audit Date | Threats Total | Closed | Open | Run By |
|------------|---------------|--------|------|--------|
| 2026-05-22 | 6 | 6 | 0 | Codex |

---

## Sign-Off

- [x] All threats have a disposition (mitigate / accept / transfer)
- [x] Accepted risks documented in Accepted Risks Log
- [x] `threats_open: 0` confirmed
- [x] `status: verified` set in frontmatter

**Approval:** verified 2026-05-22
