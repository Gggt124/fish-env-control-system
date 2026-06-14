# Phase 3: Hardware Recovery & Anti-Lockout - Context

**Gathered:** 2026-06-13
**Status:** Ready for planning

<domain>
## Phase Boundary

Provide physical hardware fail-safes for lost credentials and connectivity loss. This phase implements the hardware inputs and system recovery flows (credential reset, SoftAP fallback, connection validation rollback).

</domain>

<decisions>
## Implementation Decisions

### Recovery Button & Actions (ปุ่ม Recovery และจังหวะการกด)
- **D-01:** รองรับปุ่ม 2 ชุด: ปุ่ม Boot ภายใน (GPIO 0) และปุ่มภายนอกที่กำหนดขาได้ (เปิดใช้งาน internal pull-up)
- **D-02:** ป้องกันการกดพร้อมกัน (Mutual Exclusion): หากปุ่มหนึ่งถูกกดอยู่ ให้ละเว้นการอ่านค่าจากอีกปุ่มเพื่อป้องกันระบบรวน
- **D-03:** กดค้าง 2 วินาที: เปิดโหมด SoftAP อย่างเดียว (สำหรับใช้เปลี่ยน Wi-Fi โดยไม่รีเซ็ตรหัสผ่าน)
- **D-04:** กดค้าง 5 วินาที: เปิดโหมด SoftAP และทำการรีเซ็ตรหัสผ่านกลับเป็น `admin`/`admin123` (Recovery)

### LED Feedback (การแสดงผลไฟ LED)
- **D-05:** รองรับไฟ LED 2 ชุด: ไฟสีฟ้าบนบอร์ด และไฟ LED ภายนอกที่กำหนดขาได้
- **D-06:** แสดงสถานะการกดดังนี้:
  - 0-2 วินาที: ไฟติดค้าง (Solid ON)
  - 2-5 วินาที: ไฟกะพริบช้า (Slow Blink) แสดงว่าถึงเกณฑ์เปิด AP
  - > 5 วินาที: ไฟกะพริบเร็ว (Fast Blink) แสดงว่าถึงเกณฑ์ Recovery
- **D-07:** เมื่อปล่อยปุ่มและ SoftAP กำลังเปิดใช้งาน ไฟจะติดค้างเพื่อบอกว่าระบบพร้อมให้เชื่อมต่อ และจะดับเมื่อ SoftAP ปิดตัวลง

### SoftAP Fallback Behavior (พฤติกรรมของ SoftAP)
- **D-08:** SoftAP จะปิดตัวเองอัตโนมัติ (Timeout) ภายใน 10 นาที เพื่อเหตุผลด้านความปลอดภัย
- **D-09:** Timeout จะหยุดนับ (หรือถูกยืดออกไป) ตราบใดที่ยังมีอุปกรณ์เชื่อมต่อกับ Wi-Fi 'FishPump-Setup' ค้างไว้

### Rollback Mechanism (กลไกการย้อนกลับการตั้งค่า Wi-Fi)
- **D-10:** หากมีการตั้งค่า Wi-Fi ใหม่ ระบบจะรอรับ IP Address เป็นเวลา 30 วินาที
- **D-11:** หากไม่ได้รับ IP ภายใน 30 วินาที จะถือว่าล้มเหลว ระบบจะทำการรีบูตและย้อนกลับไปใช้การตั้งค่า Wi-Fi ล่าสุดที่ใช้งานได้ทันที

### the agent's Discretion
None

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Architecture & Conventions
- `.planning/PROJECT.md` — Project definition and stack context
- `.planning/REQUIREMENTS.md` — Requirement definitions (RECOV-01, RECOV-02, RECOV-03)
- `.planning/ROADMAP.md` — Milestone context

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `components/nvs_store` (nvs_store.h/c): Required to save or erase current configurations for rollback or reset features.
- `components/wifi_manager` (wifi_manager.h/c): Required to control APSTA mode, STA connection testing, and SoftAP timeout timers.
- `components/app_config` (app_config.h): Holds default admin credentials and timeouts constants. Needs new constants for external GPIOs.

### Established Patterns
- Wi-Fi logic utilizes FreeRTOS timers and ESP-IDF event loops (WIFI_EVENT/IP_EVENT).
- NVS keys (`sta_ssid`, `sta_pass`) are abstracted inside `nvs_store`.

### Integration Points
- `main/app_main.c`: Must integrate the GPIO button initialization, LED initialization, and FreeRTOS task/timer for polling buttons and controlling LEDs.
- `components/wifi_manager`: Must implement the 30-second STA IP wait state and the 10-minute SoftAP timeout state counting active AP clients.

</code_context>

<specifics>
## Specific Ideas

No specific requirements — open to standard approaches.

</specifics>

<deferred>
## Deferred Ideas

- **First Setup Guide**: Provide a wizard UI for the first-time setup (fresh flash) to force the user to change the default username and password. Deferred because it is a new UX capability outside the hardware recovery scope.

</deferred>

---

*Phase: 03-Hardware Recovery & Anti-Lockout*
*Context gathered: 2026-06-13*
