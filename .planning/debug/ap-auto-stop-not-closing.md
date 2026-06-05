---
status: resolved
trigger: "SoftAP ไม่ยอมปิดตัวเองแม้ว่า STA เชื่อมต่อมานานแล้ว ผู้ใช้สังเกตว่าพฤติกรรมนี้น่าจะเกิดจากการเปลี่ยนแปลงโค้ดในช่วง Phase 14 ขึ้นไป ซึ่งอาจกระทบ AP auto-stop logic"
created: 2026-06-05T01:05:28+07:00
updated: 2026-06-05T12:57:00+07:00
---

## Current Focus

```yaml
reasoning_checkpoint:
  hypothesis: "AP auto-stop ไม่ทำงานเพราะ APP_TEMPLATE_AP_AUTO_STOP_DEFAULT ถูกเปลี่ยนจาก true เป็น false
    ใน commit 8c9d5e1 (Fix pre-phase 14 audit blockers) เมื่อ 2026-06-03 ซึ่งทำให้
    s_ap_auto_stop ถูก init เป็น false ตั้งแต่ต้น และเงื่อนไขที่ IP_EVENT_STA_GOT_IP
    ตรวจสอบ (ap_enabled && ap_auto_stop && s_ap_stop_timer) จึงไม่ผ่าน ทำให้ timer ไม่ถูก start"
  confirming_evidence:
    - "commit 8c9d5e1 เปลี่ยน APP_TEMPLATE_AP_AUTO_STOP_DEFAULT จาก true → false ใน app_config.h"
    - "wifi_manager.c line 40: static bool s_ap_auto_stop = AP_AUTO_STOP_DEFAULT; ซึ่งตอนนี้เป็น false"
    - "wifi_manager.c line 524: if (ap_enabled && ap_auto_stop && s_ap_stop_timer) — เงื่อนไขนี้ fail เพราะ ap_auto_stop=false"
    - "nvs_store_load_ap_config() ใน wifi_manager_init() จะ fallback เป็น AP_AUTO_STOP_DEFAULT=false ถ้าไม่มีใน NVS"
    - "AP auto-stop timer (s_ap_stop_timer) ถูก create และ logic ยังอยู่ครบ — แค่ไม่ถูก start เพราะ flag เป็น false"
  falsification_test: "ถ้า hypothesis ผิด ควรเห็น log 'Starting AP auto-stop timer' หลัง STA ได้ IP
    แต่ถ้า log นี้ไม่ปรากฏ แสดงว่า condition (ap_auto_stop) เป็น false และ hypothesis ถูก"
  fix_rationale: "เปลี่ยน APP_TEMPLATE_AP_AUTO_STOP_DEFAULT กลับเป็น true เพื่อ restore behavior
    เดิมที่ AP auto-stop เปิดใช้งานเป็น default. การเปลี่ยนนี้แก้ที่ root cause โดยตรง
    ไม่ใช่แค่ symptom"
  blind_spots: "ยังไม่ได้ flash จริงเพื่อดู serial log — แต่ code path ชัดเจนมากพอที่จะสรุปได้"
```

hypothesis: APP_TEMPLATE_AP_AUTO_STOP_DEFAULT ถูกเปลี่ยนจาก true เป็น false ใน commit 8c9d5e1 ทำให้ AP auto-stop ไม่ถูก trigger
test: ตรวจสอบ git diff commit 8c9d5e1 กับ code path ใน wifi_manager.c
expecting: เห็นว่า ap_auto_stop=false ทำให้เงื่อนไขใน IP_EVENT_STA_GOT_IP ไม่ผ่าน
next_action: เปลี่ยน APP_TEMPLATE_AP_AUTO_STOP_DEFAULT กลับเป็น true และ build

## Symptoms

expected: หลังจาก STA เชื่อมต่อ Wi-Fi สำเร็จและผ่านไปช่วงระยะเวลาหนึ่ง (ตามที่ตั้งค่า AP auto-stop timeout ไว้ที่ 60000ms) SoftAP ควรจะปิดตัวเองโดยอัตโนมัติเพื่อประหยัดพลังงานและลด radio interference
actual: SoftAP ไม่ปิดตัวเองแม้ว่า STA จะเชื่อมต่อมานานแล้ว AP ยังคงทำงานอยู่ตลอดเวลา
errors: ไม่มี error logs ที่ชัดเจน แต่สังเกตจาก behavior ว่า AP ไม่ได้ปิด
started: หลังการเปลี่ยนแปลงโค้ดใน Phase 14 (commit 8c9d5e1, 2026-06-03)
reproduction: เชื่อมต่อ STA กับ router และรอให้ผ่าน AP auto-stop timeout (60 วินาที) แล้วตรวจสอบว่า SoftAP ยังทำงานอยู่หรือไม่

## Eliminated

- hypothesis: "ap_stop_timer ไม่ถูก create"
  evidence: "wifi_manager_init() line 624-632 create timer เสมอ และ s_ap_stop_timer ไม่เป็น NULL"
  timestamp: 2026-06-05T01:06:00+07:00

- hypothesis: "ap_stop_timer_cb() มี logic ผิด"
  evidence: "callback ที่ line 305-322 ตรวจ s_sta_connected ก่อน stop AP — logic ถูกต้อง"
  timestamp: 2026-06-05T01:06:00+07:00

- hypothesis: "nvs_store_load_ap_config() ส่งค่าผิด"
  evidence: "load function ที่ line 226-249 ใช้ AP_AUTO_STOP_DEFAULT เป็น fallback ซึ่งตอนนี้เป็น false — ถูกต้องตาม code แต่ค่า default ผิด"
  timestamp: 2026-06-05T01:06:30+07:00

## Evidence

- timestamp: 2026-06-05T01:06:00+07:00
  checked: "components/app_config/app_config.h line 41"
  found: "#define APP_TEMPLATE_AP_AUTO_STOP_DEFAULT   false"
  implication: "s_ap_auto_stop ถูก init เป็น false — timer จะไม่ถูก start"

- timestamp: 2026-06-05T01:06:00+07:00
  checked: "components/wifi_manager/wifi_manager.c line 524"
  found: "if (ap_enabled && ap_auto_stop && s_ap_stop_timer) — condition ap_auto_stop เป็น false"
  implication: "AP auto-stop timer จะไม่ถูก start หลัง STA ได้ IP"

- timestamp: 2026-06-05T01:06:10+07:00
  checked: "git show 8c9d5e1 -- components/app_config/app_config.h"
  found: "commit เปลี่ยน APP_TEMPLATE_AP_AUTO_STOP_DEFAULT จาก true → false เมื่อ 2026-06-03"
  implication: "นี่คือ root cause commit ที่ทำให้ AP auto-stop หยุดทำงาน"

- timestamp: 2026-06-05T01:06:30+07:00
  checked: "wifi_manager_init() line 580: nvs_store_load_ap_config()"
  found: "load ap config จาก NVS แต่ถ้าไม่มีค่าใน NVS จะใช้ AP_AUTO_STOP_DEFAULT ซึ่งตอนนี้เป็น false"
  implication: "อุปกรณ์ที่ไม่เคย set ap_auto_stop ผ่าน NVS จะ inherit ค่า false"

## Resolution

root_cause: "APP_TEMPLATE_AP_AUTO_STOP_DEFAULT ถูกเปลี่ยนจาก true เป็น false ใน commit 8c9d5e1
  (Fix pre-phase 14 audit blockers, 2026-06-03) เพื่อจุดประสงค์อื่น แต่ทำให้ AP auto-stop
  feature ถูก disable โดยไม่ตั้งใจ ส่งผลให้เงื่อนไขใน IP_EVENT_STA_GOT_IP handler
  (ap_enabled && ap_auto_stop && s_ap_stop_timer) ไม่ผ่าน และ timer ไม่ถูก start"
fix: "เปลี่ยน APP_TEMPLATE_AP_AUTO_STOP_DEFAULT กลับเป็น true ใน components/app_config/app_config.h"
verification: "idf.py build สำเร็จ — fish_pump_relay_timer_control.bin 0x113bf0 bytes (44% free). รอ human verify บน hardware"
files_changed:
  - components/app_config/app_config.h
