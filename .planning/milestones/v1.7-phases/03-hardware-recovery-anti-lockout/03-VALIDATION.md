---
phase: 03
slug: hardware-recovery-anti-lockout
status: draft
nyquist_compliant: false
wave_0_complete: false
created: 2026-06-13
---

# Phase 03 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | esp-idf build / manual hardware testing |
| **Config file** | none — see Wave 0 |
| **Quick run command** | `idf.py build` |
| **Full suite command** | `idf.py build` |
| **Estimated runtime** | ~60 seconds |

---

## Sampling Rate

- **After every task commit:** Run `idf.py build`
- **After every plan wave:** Run `idf.py build`
- **Before `/gsd-verify-work`:** Full suite must be green
- **Max feedback latency:** 60 seconds

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 03-01-01 | 01 | 1 | RECOV-01 | T-03-01 / — | N/A | manual | `idf.py build` | ✅ W0 | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `idf.py build` — compile check

*If none: "Existing infrastructure covers all phase requirements."*

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Hardware button reset | RECOV-01 | Hardware physical | Hold recovery button, check credentials |
| SoftAP fallback | RECOV-02 | Wi-Fi network | Verify SoftAP appears on button press |
| Rollback on fail | RECOV-03 | Wi-Fi network | Enter bad credentials, verify device reverts |

*If none: "All phase behaviors have automated verification."*

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 60s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
