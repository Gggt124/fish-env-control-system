# Phase 13: Install Setup, Status UX, and Professional UI Overhaul - Context

**รวบรวมเมื่อ:** 2026-06-03  
**สถานะ:** พร้อมสำหรับการวางแผนและการพัฒนา (Ready for Planning)

<domain>
## ขอบเขตของเฟส (Phase Boundary)

Phase 13 มุ่งเน้นการปรับปรุงหน้าตั้งค่าการติดตั้งฮาร์ดแวร์ (Hardware/Install), หน้าตั้งค่า Wi-Fi, และหน้าแสดงสถานะระบบ (System Status) โดยนำเสนอการยกเครื่องรูปแบบดีไซน์แบบ Bento Grid, การจัดโครงสร้างหน้าเว็บแบบ Glassmorphism (พื้นหลังธีมมืดโทน Indigo/Slate และแผงควบคุมโปร่งแสง) พร้อมจัดเรียงระดับข้อความและอักขระ (Modern Typography Hierarchy) โดยไม่ส่งผลกระทบต่อเสถียรภาพการทำฟังก์ชันปั๊ม, ระบบทำความเย็น (Cooling) หรือการทำงานของ Wi-Fi/APSTA ในเบื้องหลัง

ผลลัพธ์ของเฟสนี้คือการนำเสนออินเทอร์เฟซผู้ใช้ที่สวยงามระดับพรีเมียม สนับสนุน Accessibility (ระบบสัมผัส 44x44px, `:focus-visible` ชัดเจน, โครงสร้างประกาศ Screen Reader แบบ `aria-live` หรือ `role="alert"`) และการทำงานแบบออฟไลน์ 100%

</domain>

<decisions>
## การตัดสินใจในการออกแบบและพัฒนา (Implementation Decisions)

### 1. การตกแต่งแบบ Bento Grid & Glassmorphism (Visual Theme & Palette)
- **D-01 (Dark Glassmorphism Theme):** ปรับโครงสไตล์ทั้งหมดใน `style.css` ให้เป็นธีมมืดระดับพรีเมียม (Premium Dark Theme) โดยใช้พื้นหลังสีมืดโทนน้ำเงิน/เทาเข้ม (Sleek dark indigo/slate, เช่น `#0a0b10` หรือ `#0f111a`), การใช้สีพื้นการ์ดเป็นแบบกระจกโปร่งแสง (Semi-transparent Glass Cards, เช่น `rgba(255, 255, 255, 0.05)` หรือ `rgba(15, 17, 26, 0.6)`) พร้อมเส้นขอบแบบกระจกที่โปร่งแสงบาง ๆ (Translucent Borders) และใช้สีเน้นเป็นสีสว่างโทนสะท้อนแสง/น้ำเงินนีออน (Neon/Indigo accents, เช่น `#5856d6` หรือ `#0070eb`) เพื่อความพรีเมียม
- **D-02 (Bento Layout):** จัดรูปแบบ Grid ของหน้าจอให้มีขนาดและกล่องฟังก์ชันแบบ Bento Grid (สัดส่วนชัดเจน สม่ำเสมอ มีความกว้างและสัดส่วนที่แตกต่างตามความสำคัญของข้อมูล) โดยเฉพาะในหน้า Status และ Hardware
- **D-03 (Offline Footprint):** ทรัพยากรด้านภาพ, ฟอนต์, หรือไอคอนทั้งหมดห้ามอ้างอิง CDN ภายนอกอย่างเด็ดขาด ใช้ Unicode และการปรับแต่งสไตล์ CSS เท่านั้น

### 2. การควบคุมแถวเครือข่าย Wi-Fi สำหรับผู้ใช้คีย์บอร์ด (Wi-Fi Keyboard Operability)
- **D-04 (Semantic Button Rows):** ปรับแก้ใน `app.js` และ `wifi.html` โดยให้แถวรายการเครือข่าย Wi-Fiแต่ละแถว (Network Item) มีโครงสร้างเป็นแท็ก `<button>` หรือมีคุณลักษณะ `role="button"` และ `tabindex="0"` เพื่อให้ผู้ใช้งานที่ใช้คีย์บอร์ดสามารถกดปุ่ม Tab เลื่อนเพื่อเลือกเครือข่ายและกดใช้งานผ่านปุ่ม Space หรือ Enter ได้โดยตรง โดยจะแสดงวงแหวน `:focus-visible` รอบแถวเครือข่ายเมื่อถูกเลือกอย่างชัดเจน

### 3. การแสดงผลฮาร์ดแวร์แบบ Active vs. Pending (Active vs. Pending GPIO Layout)
- **D-05 (Side-by-side Bento Cards):** ในหน้าจอ Hardware/Install ให้จัดเรียงข้อมูล GPIO ออกเป็น 2 กล่องใหญ่แบบเคียงข้างกัน (หรือแบบเรียงต่อกันในมือถือ):
  - **Active GPIO Map:** แสดงรายการพินที่กำลังทำงานอยู่ ณ ปัจจุบัน ตกแต่งด้วยกรอบหรือป้ายสถานะโทนสีเขียวแห่งความสำเร็จ (Success-green)
  - **Pending Reboot Map:** แสดงรายการพินที่มีการแก้ไขและบันทึกใหม่แล้วแต่กำลังรอรีบูต ตกแต่งด้วยกรอบหรือป้ายสถานะโทนสีส้มแจ้งเตือน (Warning-orange) เพื่อแจ้งให้ผู้ติดตั้งเห็นความต่างอย่างชัดเจนก่อนทำการเปลี่ยนโครงสร้างการเดินสายไฟจริง

### 4. การจัดการกรณีขาดการเชื่อมต่อ Wi-Fi ในช่วงการตั้งค่า (Wi-Fi Connection Handling UX)
- **D-06 (Reconnection Helper Banner):** เมื่อเริ่มกระบวนการกดปุ่มเชื่อมต่อ Wi-Fi (ซึ่งผู้ใช้อาจสูญเสียสัญญาณ SoftAP ชั่วคราวเนื่องจาก ESP32 เปลี่ยนช่องสัญญาณหรือวิทยุเชื่อมต่อไปยังตัวเราเตอร์หลัก) ให้แสดงแผงแบนเนอร์หรือกล่องข้อความช่วยเหลือสีส้มเตือนใจความภาษาไทยชัดเจน (เช่น *"กำลังเชื่อมต่อ... หากหลุด ให้ตรวจสอบ Wi-Fi มือถือว่าต่อกับ SoftAP IP 192.168.4.1 อีกครั้ง"*) พร้อมตัวเลขนับถอยหลังช่วยให้ผู้ใช้รับรู้ขั้นตอน

### 5. การปรับปรุง Accessibility & Spacing & Text Wrap
- **D-07 (Focus & Touch Targets):** ให้ปุ่มและรายการคลิกทุกจุดมีขนาดพื้นที่สัมผัสอย่างน้อย 44x44px เพื่อรองรับนิ้วมือ และอัปเดต `:focus-visible` เป็นวงขอบหนา 2-4px รอบปุ่มโดยไม่มีรอยตัด
- **D-08 (Reduced Motion):** เพิ่ม CSS Media Query `@media (prefers-reduced-motion: reduce)` เพื่อยกเลิกหรือย่นระยะเวลาการสั่นไหว การสไลด์ หรือการเลื่อนของ Dynamic Animations/Toasts
- **D-09 (Text Wrap Proof):** ตรวจสอบและให้ค่าแสดงผลข้อความยาว ๆ เช่น MAC, IP, SSID และ Uptime บนหน้าจอมีความสามารถในการตัดคำหรือย่อขนาดเพื่อไม่ให้ส่งผลต่อขอบเขตหน้าระบบ (Horizontal overflow)

</decisions>

<canonical_refs>
## เอกสารอ้างอิงหลัก (Canonical References)

- `.planning/phases/11-baseline-ui-audit-and-state-language/11-PHASE-13-BRIEF.md` - คำสั่งแนะนำและข้อมูลที่ตรวจพบ (Findings) ทั้งหมดสำหรับเฟสนี้
- `.planning/phases/11-baseline-ui-audit-and-state-language/11-UI-SPEC.md` - ข้อตกลงระดับพารามิเตอร์ ขนาดฟอนต์ สี แบรนดิ้ง
- `main/static/hardware.html` - หน้าการตั้งค่า GPIO ที่จะปรับให้มี Side-by-side Cards
- `main/static/wifi.html` - หน้าจอจัดการตั้งค่า Wi-Fi ที่จะมีปุ่มรายแถวและการสแกนเชื่อมต่อ
- `main/static/status.html` - หน้าแสดงข้อมูลสถานะและระบบตรวจวินิจฉัย
- `main/static/style.css` - ไฟล์สไตล์หลักที่จะเขียนธีมมืด/Glassmorphism ใหม่ทั้งหมด
- `main/static/app.js` - สคริปต์ควบคุมการแสดงผลและ API สำหรับ Wi-Fi/GPIO และการนับถอยหลัง

</canonical_refs>

<code_context>
## การตรวจสอบจุดเชื่อมโยงในโค้ด (Existing Code Insights)

- **style.css:** คาดว่าจะต้องถูกเขียนใหม่เกือบทั้งหมด เพื่อปรับเปลี่ยนตัวแปรสี ระบบจัดเรียง และกรอบ Glassmorphism/Bento โดยยังคงควบคุมขนาดไฟล์ให้เหมาะสมไม่บวมเกินไป
- **app.js:** ฟังก์ชัน `renderWifiList` จะถูกอัปเดตเพื่อสร้าง `<button>` สำหรับแต่ละรายการเครือข่ายแทน `<li>` ธรรมดา พร้อมโครงสร้างสำหรับการจัดการ Reconnection Banner ที่เชื่อมโยงกับ `doConnect` และ `pollWifiConnection`
- **hardware.html & status.html:** จะใช้รูปแบบตู้แผง Bento Grid ใหม่และใช้ตัวแปรโครงสร้างสีมืด Glassmorphism ที่นิยามไว้ใน `style.css`

</code_context>

<specifics>
## รายละเอียดเชิงลึกเฉพาะงาน (Specific Ideas)

- **ไอคอนประกอบ:** ใช้ Unicode อักษรตกแต่งที่สะท้อนแสงเด่นชัดในการสื่อสารบนแถบและบนปุ่ม เช่น `&#128269;` (Scan), `&#128295;` (Hardware), `&#128246;` (Wi-Fi), `&#9881;` (Settings) เพื่อรักษาระดับการใช้พื้นที่จัดเก็บที่เบา
- **แบนเนอร์แจ้งเตือนแบบเรียลไทม์:** ใน `wifi.html` จะใช้แบนเนอร์มี `role="status"` หรือ `aria-live="polite"` เพื่อรองรับ Accessibility อย่างสมบูรณ์

</specifics>

<deferred>
## รายการที่เลื่อนออกไป (Deferred Ideas)

- การจัดเตรียมหน้าจอประวัติกราฟและการส่งข้อมูล Telemetry ถูกกำหนดให้ออกไปนอกกรอบการ Polish v1.2 นี้
- การอัปเกรดความปลอดภัยและ HTTPS รวมถึงโมดูล OTA จะได้รับการจัดการในไมล์สโตนถัดไป

</deferred>
