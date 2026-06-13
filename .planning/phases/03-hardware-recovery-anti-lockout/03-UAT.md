# Phase 3 UAT Checklist: Hardware Recovery & Anti-Lockout

The automated verification checks for Phase 3 have successfully passed. Since this phase involves physical hardware interactions (buttons, LEDs) and Wi-Fi environment behaviors, manual verification is required.

Please perform the following tests on the physical ESP32 device:

---

## 1. Verify Staging Confirmation UI Banner
- **Setup:** Change the admin credentials or save a new Wi-Fi connection profile.
- **Action:** Open the dashboard, status, or Wi-Fi configuration page in your browser.
- **Expectation:**
  - [ ] A yellow warning banner is displayed at the top: **"โหมดทดลองใช้การตั้งค่าใหม่: ระบบจะคืนค่าเดิมและรีบูตใน 3 นาทีหากไม่กดยืนยัน"** (New configuration trial mode: system will revert and reboot in 3 minutes if not confirmed).
  - [ ] The banner contains a **ยืนยัน (Confirm)** button and a **ยกเลิก (Cancel)** button.
  - [ ] Clicking **ยืนยัน (Confirm)** sends a `POST /api/confirm` request, makes the configuration permanent, and hides the banner.
  - [ ] Clicking **ยกเลิก (Cancel)** sends a `DELETE /api/confirm` request, rolls back the staging parameters, and triggers a delayed reboot.

---

## 2. Verify Hardware Button Recovery AP
- **Action:** Press and hold the BOOT/Recovery button (GPIO 0) or the external button (GPIO 4) for **2 to 5 seconds**.
- **Expectation:**
  - [ ] The onboard blue LED (GPIO 2) and external LED (GPIO 5) start blinking slowly (500ms intervals).
  - [ ] Release the button when the slow blink starts.
  - [ ] The LEDs remain **solid ON**.
  - [ ] The SoftAP `FishPump-Setup` becomes visible on a Wi-Fi scan and can be connected to at `http://192.168.4.1`.

---

## 3. Verify Hardware Button Factory Reset
- **Action:** Press and hold the BOOT/Recovery button (GPIO 0) or the external button (GPIO 4) for **more than 5 seconds**.
- **Expectation:**
  - [ ] The onboard blue LED and external LED transition from slow blinking to **fast blinking (100ms intervals)**.
  - [ ] Release the button.
  - [ ] The system logs the credentials reset to default (`admin`/`admin123`).
  - [ ] Active memory sessions are invalidated (forcing re-login).
  - [ ] The SoftAP remains active (no reboot).

---

## 4. Verify Auto-Rollback on Connection Failure (Anti-Lockout)
- **Action:** Configure a staged Wi-Fi connection with a wrong password and let the device reboot to validate. Do not click confirm.
- **Expectation:**
  - [ ] The device boots, tries to connect to the staged profile, fails to get an IP address, and times out after 30 seconds.
  - [ ] The device automatically rolls back the staged credentials and reboots.
  - [ ] On the subsequent boot, the device reconnects to the previous known-good Wi-Fi profile.

---

## Sign-off

If all these behaviors function correctly, you can sign off on this phase to advance to the next step.
