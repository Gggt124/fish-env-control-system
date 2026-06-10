# Phase 1: Persistent Sessions & Storage Foundation - Context

**Gathered:** 2026-06-11
**Status:** Ready for planning

<domain>
## Phase Boundary

Establish secure, persistent session token architecture to support "Remember Me" without NVS wear.
</domain>

<decisions>
## Implementation Decisions

### รูปแบบ Token (Token Format)
- **D-01:** ใช้ JWT (JSON Web Token) โดยเข้ารหัสและตรวจสอบในตัว ESP32 เอง ไม่ต้องใช้เน็ตเวิร์กภายนอก ช่วยลดปัญหา NVS wear เพราะไม่ต้องเก็บ session state ใน NVS

### กลยุทธ์ถนอม NVS (Wear Mitigation)
- **D-02:** ยอมผ่อนปรนเรื่องโควต้า 5 อุปกรณ์ (อนุญาตให้ล็อกอินได้ไม่จำกัดจำนวนอุปกรณ์ตราบใดที่ JWT ยังถูกต้อง) เพื่อให้ไม่ต้องเขียน NVS เลยเวลาล็อกอินใหม่ ถนอม Flash memory ได้ 100%

### อายุของเซสชัน (Session Expiration)
- **D-03:** ไม่มีวันหมดอายุ (Never expire) ล็อกอินครั้งเดียวใช้ได้ยาวๆ จนกว่าจะกด Logout หรือมีการเปลี่ยนรหัสผ่าน เน้นความสะดวกสบายสูงสุดสำหรับเจ้าของบ่อปลาคาร์ฟ

### การตรวจสอบความถูกต้องของเซสชัน (Session Validation)
- **D-04:** ผูก JWT เข้ากับ IP Address ของเครื่องผู้ใช้ (IP Binding) เพื่อป้องกันความเสี่ยงจากการถูกดักจับคุกกี้ (HTTP Sniffing) บนเครือข่าย Wi-Fi แม้ว่าจะทำให้ผู้ใช้ต้องล็อกอินใหม่หาก Router เปลี่ยน IP (DHCP) ก็ตาม

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Guidelines
- `.planning/PROJECT.md` — Project context and current state
- `.planning/ROADMAP.md` — Milestones and phase success criteria

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `esp_random()`: ใช้สุ่มค่า Secret Key สำหรับเซ็น JWT
- `components/nvs_store`: ใช้บันทึก JWT Secret Key ที่สุ่มขึ้นมาเพียงครั้งเดียวลงใน NVS

### Established Patterns
- ใช้ Cookie (non-HttpOnly) แบบเดิมเพื่อให้ `app.js` ในฝั่ง Frontend อ่านค่าได้
- คอมโพเนนต์ `session` ปัจจุบันใช้ In-memory array ต้องปรับไปใช้ระบบตรวจสอบ Signature ของ JWT แทน

</code_context>

<specifics>
## Specific Ideas

Persona ของผู้ใช้งานหลักคือ "เจ้าของบ่อปลาคาร์ฟสวยงาม" ดังนั้น "ความสะดวกสบาย" ต้องมาก่อนความปลอดภัยที่เข้มงวดจนน่ารำคาญ (เช่น ไม่ควรถูกเตะออกจากระบบเพียงเพราะ IP เปลี่ยน หรือต้องล็อกอินใหม่บ่อยๆ)
</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope
</deferred>

---

*Phase: 01-persistent-sessions-storage-foundation*
*Context gathered: 2026-06-11*
