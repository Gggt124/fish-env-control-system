---
quick_id: 260520-siy
status: planned
date: 2026-05-20
---

# Quick Task 260520-siy: Switch to dual OTA partition layout

## Goal

Replace the current 1MB factory app partition with a 4MB-flash-compatible dual OTA layout that gives each app slot about 1.94 MiB.

## Tasks

1. Update `partitions.csv` to include `otadata`, `ota_0`, and `ota_1`.
2. Size each OTA app slot at `0x1F0000` bytes and keep the layout within 4MB flash.
3. Keep NVS and PHY partitions available for current Wi-Fi/session behavior.
4. Update docs that describe the partition layout.
5. Build with the project ESP-IDF wrapper and confirm the binary fits.

