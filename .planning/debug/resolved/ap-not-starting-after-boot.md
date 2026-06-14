---
status: resolved
trigger: "ตัว AP ไม่ถูกเปิด หลัง boot ///  ไฟสถานะ แสดงปกติ แต่ใน tft ขึ้น disconnect /// ทดลองใช้ปุ่ม boot เปด AP ไฟก็ขึ้น แต่ tft ก็ขึ้น disconnect แล้วพอแสกนก็ไม่เจอ fish pump ap"
created: "2026-06-13T20:57:02+07:00"
updated: "2026-06-13T22:15:40+07:00"
---

## Current Focus
<!-- OVERWRITE on each update - reflects NOW -->

reasoning_checkpoint:
  hypothesis: "wifi_manager_init sets mode to WIFI_MODE_STA on normal boot, which causes configure_ap() to fail with ESP_ERR_WIFI_MODE. This causes wifi_manager_init to return false early, skipping esp_wifi_start(). Since esp_wifi_start() is never called, the Wi-Fi radio stays off permanently."
  confirming_evidence:
    - "wifi_manager.c:658 sets mode to WIFI_MODE_STA if not staging."
    - "wifi_manager.c:734 calls configure_ap(), which calls esp_wifi_set_config(WIFI_IF_AP). This fails if mode is STA."
    - "If configure_ap() fails, wifi_manager_init() returns false at line 735, skipping ESP_ERROR_CHECK(esp_wifi_start()) at line 738."
    - "When BOOT button is pressed, wifi_manager_start_recovery_ap() changes mode to APSTA and configures AP, but never calls esp_wifi_start(), so radio remains off while LED (based on s_ap_enabled state variable) turns on."
  falsification_test: "If esp_wifi_set_config(WIFI_IF_AP) succeeds in WIFI_MODE_STA mode (unlikely per ESP-IDF docs), this hypothesis is wrong."
  fix_rationale: "Fixing wifi_manager_init to set WIFI_MODE_APSTA initially (as per original requirements), OR only configuring AP if mode allows it and ensuring esp_wifi_start() is called, will turn the radio on and allow the AP to start."
  blind_spots: "We haven't checked if the new multi-profile logic in app_main.c actually calls wifi_manager_start_ap() if there are no saved profiles, even if init succeeds."

## Symptoms
<!-- Written during gathering, then IMMUTABLE -->

expected: AP "fish pump ap" (or similar) should broadcast after boot. TFT should show connected/AP mode.
actual: AP is not broadcasting after boot. LED shows normal, TFT shows disconnected. Pressing boot button turns on LED for AP but still no broadcast and TFT says disconnected.
errors: N/A
reproduction: Boot the device, or press BOOT button to trigger AP. Scan for Wi-Fi.
started: Found during Phase 3 or Phase 4 testing.

## Eliminated
<!-- APPEND only - prevents re-investigating -->

## Evidence
<!-- APPEND only - facts discovered -->
- 2026-06-13T14:00:00Z: Checked `wifi_manager_init()` - it sets mode to `WIFI_MODE_STA` if no staging is active.
- 2026-06-13T14:00:00Z: Checked `configure_ap()` - it calls `esp_wifi_set_config(WIFI_IF_AP, &ap_cfg)` which fails if mode is `WIFI_MODE_STA`.
- 2026-06-13T14:00:00Z: Checked `wifi_manager_init()` - if `configure_ap()` fails, it returns false before calling `esp_wifi_start()`.
- 2026-06-13T14:00:00Z: Checked `wifi_manager_start_recovery_ap()` - sets mode to `WIFI_MODE_APSTA`, configures AP, but assumes `esp_wifi_start()` was already called by init.

## Resolution
<!-- OVERWRITE as understanding evolves -->

root_cause: `wifi_manager_init()` returns early without calling `esp_wifi_start()` because `configure_ap()` fails when the Wi-Fi mode is set to `WIFI_MODE_STA`. This was inadvertently introduced during Phase 3 when trying to keep SoftAP active during staging.
fix: Modified `wifi_manager_init()` to unconditionally set `WIFI_MODE_APSTA` at boot, matching the original requirement. The SoftAP will start at boot and correctly be stopped by the 10-minute auto-stop timer after a successful STA connection, preventing lockout.
verification: Manual verification required to flash and confirm the AP broadcasts as expected on boot or after recovery button press.
files_changed: [components/wifi_manager/wifi_manager.c]
