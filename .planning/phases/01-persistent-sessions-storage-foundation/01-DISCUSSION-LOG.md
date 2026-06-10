# Phase 1 Discussion Log

**Date:** 2026-06-11

## 1. รูปแบบ Token (Token Format)
- **Options presented:** Opaque Token (esp_random), JWT, You Decide
- **Selected:** ใช้ JWT (เข้ารหัสและตรวจสอบในตัว ESP32 เอง ไม่ใช้เน็ต)
- **Notes:** ผู้ใช้ยืนยันการใช้ JWT โดยไม่ต้องต่ออินเทอร์เน็ต เพื่อลด NVS Wear

## 2. กลยุทธ์ถนอม NVS (Wear Mitigation)
- **Options presented:** ผ่อนปรนโควต้า 5 อุปกรณ์, เคร่งครัดโควต้า 5 อุปกรณ์, You Decide
- **Selected:** ยอมผ่อนปรนเรื่องโควต้า 5 อุปกรณ์
- **Notes:** ไม่จำกัดจำนวนอุปกรณ์เพื่อให้ไม่ต้องเขียน NVS เลยตอนล็อกอิน ถนอม Flash ได้ 100%

## 3. อายุของเซสชัน (Session Expiration)
- **Options presented:** 30 วัน, 90 วัน, ไม่มีวันหมดอายุ
- **Selected:** ไม่มีวันหมดอายุ
- **Notes:** ผู้ใช้ระบุ Persona "เจ้าของบ่อปลาคาร์ฟ" เน้นความสะดวกสบาย ไม่ต้องล็อกอินบ่อยๆ

## 4. การตรวจสอบความถูกต้องของเซสชัน (Session Validation)
- **Options presented:** ไม่ผูกกับ IP/User-Agent, ผูก IP, ผูก User-Agent
- **Selected:** ผูก IP Address
- **Notes:** ผู้ใช้เปลี่ยนใจหลังจากการซักถาม (Grilling) โดยยอมแลกความสะดวก (อาจต้องล็อกอินใหม่หาก IP เปลี่ยน) เพื่ออุดช่องโหว่การดักจับคุกกี้ (HTTP Sniffing) ในวง Wi-Fi

## 5. การจัดการช่องโหว่เพิ่มเติม (Grilling Session)
- **Logout Behavior:** ยอมรับความเสี่ยงที่การกด Logout จะเป็นเพียงการลบคุกกี้จากฝั่งเบราว์เซอร์เท่านั้น (ไม่ยอมเขียน Blacklist ลง NVS)
- **Weak Entropy:** ยอมรับให้ใช้ `esp_random()` ตอนเปิดเครื่องตามเดิม แม้ Wi-Fi จะยังไม่ทำงานก็ตาม (รับความเสี่ยงเรื่อง Entropy ได้)
