# GSD Debug Knowledge Base

Resolved debug sessions. Used by `gsd-debugger` to surface known-pattern hypotheses at the start of new investigations.

---

## ap-not-starting-after-boot — AP is not broadcasting after boot
- **Date:** 2026-06-13T22:15:40+07:00
- **Error patterns:** disconnect, AP ไม่ถูกเปิด, fish pump ap, ไฟสถานะ
- **Root cause:** `wifi_manager_init()` returns early without calling `esp_wifi_start()` because `configure_ap()` fails when the Wi-Fi mode is set to `WIFI_MODE_STA`. This was inadvertently introduced during Phase 3 when trying to keep SoftAP active during staging.
- **Fix:** Modified `wifi_manager_init()` to unconditionally set `WIFI_MODE_APSTA` at boot, matching the original requirement. The SoftAP will start at boot and correctly be stopped by the 10-minute auto-stop timer after a successful STA connection, preventing lockout.
- **Files changed:** components/wifi_manager/wifi_manager.c
---
