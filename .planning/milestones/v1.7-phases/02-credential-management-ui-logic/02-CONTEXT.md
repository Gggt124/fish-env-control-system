# Phase 2: Credential Management UI & Logic - Context

**Gathered:** 2026-06-12
**Status:** Ready for planning

<domain>
## Phase Boundary

Users can securely change the default administrator credentials and invalidate old sessions.
</domain>

<decisions>
## Implementation Decisions

### 1. ตำแหน่งของ UI (UI Location)
- **D-01:** ใช้รูปแบบ **Popup/Modal บนหน้า Dashboard** เพื่อให้ไม่ต้องโหลดหน้าใหม่หรือเปลี่ยนหน้าไปมา ช่วยให้กระบวนการเปลี่ยนรหัสผ่านรวดเร็ว ไม่รกหน้า Dashboard

### 2. ข้อมูลที่ใช้ยืนยันตัวตน (Form Fields & Validation)
- **D-02:** **ขอแค่ "รหัสผ่านเดิม" (Current Password)** ในการยืนยันตัวตนก็พอ ผู้ใช้จะได้ไม่ต้องจำและพิมพ์ Username เดิมให้ยุ่งยาก (ระบบจะดึง Username ปัจจุบันจาก Session) ส่วนฟิลด์สำหรับข้อมูลใหม่จะมีให้ระบุทั้ง Username ใหม่ และ Password ใหม่ 

### 3. พฤติกรรมหลังเปลี่ยนรหัสเสร็จ (Post-Change Behavior)
- **D-03:** **แสดงข้อความแจ้งเตือนความสำเร็จ** ค้างไว้บนหน้า Dashboard ประมาณ 2-3 วินาทีเพื่อให้ผู้ใช้ทราบและรู้ตัวว่าทำรายการสำเร็จแล้ว จากนั้นระบบค่อยทำการเตะออก (Force Logout) และเปลี่ยนเส้นทาง (Redirect) ไปยังหน้า Login เพื่อบังคับล็อกอินด้วยรหัสผ่านใหม่
</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Guidelines
- `.planning/PROJECT.md` — Project context and current state
- `.planning/ROADMAP.md` — Milestones and phase success criteria
- `.planning/REQUIREMENTS.md` — Authentication requirements (AUTH-05, AUTH-06, AUTH-07)
</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `main/static/app.js`: มีระบบ Modal ที่ใช้กับส่วนอื่นอยู่แล้ว (เช่น Wi-Fi Connect หรือ Logout Confirmation) สามารถนำมาใช้กับ Popup เปลี่ยนรหัสผ่านได้เลย
- `components/nvs_store/nvs_store.h`: มี API สำหรับบันทึก Username/Password ลง NVS ที่สามารถเรียกใช้จาก HTTP Handler ได้

### Established Patterns
- ใช้รูปแบบ SPA (Single Page Application) ในการดัก Submit ฟอร์มและส่งข้อมูลผ่าน `apiPost()`
- การตอบกลับจาก API จะใช้ cJSON และส่งคืนสถานะ HTTP เพื่อแสดง Error หรือความสำเร็จบน UI
</code_context>

<deferred>
## Deferred Ideas

None
</deferred>
