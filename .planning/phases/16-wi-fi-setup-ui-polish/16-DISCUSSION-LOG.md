# Phase 16: Wi-Fi Setup UI Polish - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-06-05
**Phase:** 16-wi-fi-setup-ui-polish
**Areas discussed:** Empty State Card Design, Transitions & Animations, Mobile Responsive Layout

---

## Empty State Card Design

| Option | Description | Selected |
|--------|-------------|----------|
| ดีไซน์สไตล์คลีน (Subtle Border) | มีไอคอนเสาสัญญาณ 📶 หรือชี้นิ้ว 👈, หัวข้อ "โปรดเลือกเครือข่าย" และกรอบเส้นประบางตาม CSS variables | ✓ |
| ดีไซน์สไตล์มินิมอล | เน้นเฉพาะข้อความขนาดกลางกึ่งกลางกล่องแบบเรียบง่าย ไร้กรอบและไอคอน | |

**User's choice:** ดีไซน์สไตล์คลีน (Subtle Border)
**Notes:** -

---

## Transitions & Animations

| Option | Description | Selected |
|--------|-------------|----------|
| Fade-In | ใช้ CSS opacity transition (0.2s) ค่อย ๆ สลับหน้าจออย่างนุ่มนวล (และซัพพอร์ต prefers-reduced-motion) | ✓ |
| Instant Swap | สลับทันทีด้วยการสลับคลาส CSS (display: none / block) โดยไม่มีเอฟเฟกต์ เพื่อประหยัดทรัพยากรการประมวลผลสูงสุด | |

**User's choice:** Fade-In
**Notes:** -

---

## Mobile Responsive Layout

| Option | Description | Selected |
|--------|-------------|----------|
| แสดงผลต่อท้ายตาราง Wi-Fi (Normal Stack) | แสดง Empty State Card ต่อท้ายตารางสแกนบนจอมือถือตามโครงสร้างหลัก เพื่อให้รักษาขั้นตอนการทำงานให้เข้าใจง่าย | ✓ |
| ซ่อนแผงตั้งค่าทั้งหมดจนกว่าจะกดเลือก (Hide Until Selection) | บนมือถือจะไม่แสดงแผงขวา/Empty State เลยเพื่อประหยัดพื้นที่หน้าจอ เมื่อผู้ใช้กดเลือกเครือข่ายแล้ว ฟอร์มกรอกรหัสผ่านจึงจะแสดงขึ้นมาต่อท้ายตารางสแกน | |

**User's choice:** แสดงผลต่อท้ายตาราง Wi-Fi (Normal Stack)
**Notes:** -

---

## the agent's Discretion

- Sizing and spacing details of elements inside the Empty State Card.
- Specific CSS class naming structure for switching states (such as `.hidden` or styling rules for swapping visibility).

## Deferred Ideas

None — discussion stayed within phase scope.

---

*Phase: 16-wi-fi-setup-ui-polish*
*Discussion log generated: 2026-06-05*
