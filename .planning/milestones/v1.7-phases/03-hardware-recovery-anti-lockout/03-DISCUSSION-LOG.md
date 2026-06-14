# Phase 3: Hardware Recovery & Anti-Lockout - Discussion Log

**Date:** 2026-06-13

This log records the human context, rationale, and specific options presented during the phase discussion. It is for human reference only (audits, retrospectives). Downstream agents consume `03-CONTEXT.md` instead.

## Area 1: Recovery Button (ปุ่ม Recovery)
**Options Presented:**
- (Recommended) ใช้ปุ่ม Boot (GPIO 0) กดค้าง 5 วินาที — ง่าย ไม่ต้องต่อสายเพิ่ม
- ใช้ปุ่มภายนอกที่ต่อกับ GPIO ขาอื่น — เพื่อให้ผู้ใช้กดง่ายถ้าใส่เคส (โปรดระบุขา GPIO ภายหลัง)
- ใช้ปุ่ม Boot แต่ขอเปลี่ยนเวลากดค้าง (เช่น 10 วินาที)

**User Selection:**
- ใส่ทั้ง 2 ที่เลยได้ไหม แล้วก็ขาที่จะใส่ควรมี internal pull ด้วย

**Notes:**
User wants both GPIO 0 and another external GPIO with internal pull-up enabled. 

## Area 2: SoftAP Fallback Behavior (พฤติกรรมของ SoftAP)
**Options Presented:**
- (Recommended) มี Timeout ปิดอัตโนมัติภายใน 5-10 นาที (ถ้าไม่มีการเชื่อมต่อหรือตั้งค่า) เพื่อความปลอดภัย
- เปิดค้างไว้ตลอดจนกว่าผู้ใช้จะกดรีบูตเครื่อง หรือตั้งค่า Wi-Fi ใหม่สำเร็จ

**User Selection:**
- (Recommended) มี Timeout ปิดอัตโนมัติภายใน 5-10 นาที (ถ้าไม่มีการเชื่อมต่อหรือตั้งค่า) เพื่อความปลอดภัย
- แค่มีอุปกรณ์ (มือถือ/คอมพิวเตอร์) เชื่อมต่อค้างไว้กับ Wi-Fi 'FishPump-Setup' ตัว Timeout ก็จะหยุดนับ (ง่ายและเสถียร)

**Notes:**
Set a timeout for 10 minutes. It pauses if any AP client is actively connected.

## Area 3: Rollback Mechanism (กลไกการย้อนกลับการตั้งค่า)
**Options Presented:**
- (Recommended) รอ 30 วินาที ถ้าไม่ได้รับ IP Address ถือว่าล้มเหลว (ปลอดภัย เผื่อ Router แจก IP ช้า)
- รอ 15 วินาทีก็พอ (รู้ผลเร็ว แต่ถ้า Router ช้าอาจจะพลาด)
- ใช้จำนวนครั้งแทนเวลา (เช่น พยายามเชื่อมต่อ 3 ครั้ง ถ้าไม่สำเร็จให้ Rollback ทันที)

**User Selection:**
- (Recommended) รอ 30 วินาที ถ้าไม่ได้รับ IP Address ถือว่าล้มเหลว (ปลอดภัย เผื่อ Router แจก IP ช้า)

**Notes:**
Wait exactly 30 seconds for IP_EVENT. If it fails, reboot and rollback.

## Area 4: LED Feedback & Button Action Timing (เจาะลึกจาก /grill-me)
**Options Presented / Discussion Flow:**
- Separating SoftAP opening and Password Reset into two different actions.
- What feedback the LED should provide.
- Should there be a First Setup Wizard.

**User Selection:**
- "2วิเปิด AP /// 5 วิ recovery โอเคไหม"
- "ต้องมีการ block อีกปุ่มนึงด้วยหรือป่าว ถ้ากดค้างที่ปุ่มนึงอยู่ จะหว่างปุ่มที่ boot กับที่ gpio"
- "เรื่องuser กับ password เบื้องต้น ถ้าเป็นตอนที่ยังไม่เคยมีการบันทึกอะไรเลย(พึ่ง flash) ควรให้มี first setup guide รึป่าว"
- "(Recommended) LED ติดค้างตอนเริ่มกด -> กะพริบช้าเมื่อถึง 2 วิ (AP) -> กะพริบเร็วเมื่อถึง 5 วิ (Recovery) -> เมื่อปล่อยปุ่ม LED ติดค้างเพื่อบอกว่า SoftAP ทำงานอยู่"

**Notes:**
- Adopted 2s / 5s timing.
- Adopted mutual exclusion for buttons.
- Adopted LED flashing feedback based on hold duration. LED stays solid while SoftAP is active.
- Noted First Setup Guide as a Deferred Idea (Scope Creep).
