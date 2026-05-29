---
status: complete
date: 2026-05-29
---

# Quick Task 260529-kso: Use ESP-IDF from C:\esp

## Goal

Update the local ESP-IDF workflow to use the new install under `C:\esp`, remove the old `C:\esp-idf` directory, then build and attempt flash/monitor validation.

## Tasks

1. Update PowerShell build/flash scripts so their default ESP-IDF discovery uses `C:\esp\v6.0.1\esp-idf` and can resolve an ESP-IDF root from a parent install directory.
2. Update operator docs and AGENTS instructions to show the new path.
3. Remove the old `C:\esp-idf` install after verifying the resolved path, then run build and monitor/flash as far as connected hardware allows.
