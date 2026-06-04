# Phase 11 Shared UI State Language

## Purpose

This contract gives Login, App Shell, Dashboard, Hardware/Install, Status, and
Wi-Fi one compact vocabulary for user-visible state. Phase 12 and Phase 13
should reuse these meanings in the embedded web UI. The wording is intentionally
short enough to reuse later on a small TFT display.

The product voice is Thai-first, calm, and operational. Keep precise technical
nouns such as `GPIO`, `STA`, `AP`, `IP`, `Relay`, and `Wi-Fi` where they reduce
ambiguity.

## Availability Conditions

Do not collapse every unavailable action into a dimmed button. Use one of these
three conditions:

| Condition | Meaning | Required treatment | Example |
| --- | --- | --- | --- |
| `processing` | A request is already running. | Disable only the affected control, show action-specific loading text, and announce a polite status update. | `กำลังสแกน Wi-Fi...` |
| `unavailable` | A prerequisite or safe runtime condition is absent. | Show the reason next to the control or status area and preserve any safe recovery action. | `ยังสั่งงานไม่ได้: รอข้อมูลล่าสุดจากอุปกรณ์` |
| `pending reboot` | Saved hardware values exist but are not active. | Show active and pending values separately and state that reboot is required before wiring behavior changes. | `บันทึก GPIO แล้ว ต้องรีบูต ESP32 ก่อนค่าชุดใหม่จะทำงาน` |

### Unavailable Reasons

Use a short reason, not a generic disabled label:

| Reason | When to use | Example Thai-first copy |
| --- | --- | --- |
| Feature unsupported | Firmware or hardware does not expose the capability. | `ยังใช้คำสั่งนี้ไม่ได้: firmware ชุดนี้ไม่รองรับ` |
| Data not yet available | Latest device state has not loaded or is stale. | `ยังสั่งงานไม่ได้: รอข้อมูลล่าสุดจากอุปกรณ์` |
| Device or network temporarily unreachable | Request failed because the local controller cannot be reached. | `ติดต่ออุปกรณ์ไม่ได้ชั่วคราว ตรวจสอบการเชื่อมต่อแล้วลองอีกครั้ง` |
| Sensor fault | A safety-relevant reading is absent or invalid. | `ยังเปิด Cooling ไม่ได้: อ่านค่า DS18B20 ไม่สำเร็จ` |
| Missing selection | User must choose an item first. | `โปรดเลือกเครือข่าย Wi-Fi ก่อนเชื่อมต่อ` |

## Canonical States

| State | User meaning | Thai-first message pattern | Visual treatment | Allowed action | Assistive treatment | Current source or target |
| --- | --- | --- | --- | --- | --- | --- |
| `loading` | A request is in flight. | `กำลัง<verb-object>...` such as `กำลังสแกน Wi-Fi...` | Keep layout stable, disable the relevant control, and use a small spinner only where useful. | Preserve unrelated actions; prevent duplicate submit. | Announce with `aria-live="polite"`. | Existing login, pump, cooling, hardware, Wi-Fi paths; normalize in Phase 12 and Phase 13. |
| `success` | The requested action completed. | `<verb-object>เรียบร้อย` such as `บันทึกค่า Cooling เรียบร้อย` | Show durable contextual confirmation near the initiating area; add a short toast when useful. | Offer the next safe action. | Announce with `aria-live="polite"`. | Existing toasts and contextual status targets; normalize in Phase 12 and Phase 13. |
| `error` | The action failed or current data is unsafe. | `<action>ไม่สำเร็จ <recovery>` such as `เชื่อมต่อ Wi-Fi ไม่สำเร็จ ตรวจสอบรหัสผ่านแล้วลองอีกครั้ง` | Show an error surface near the relevant control with a concise recovery instruction. | Keep a retry or corrective action available when safe. | Use `role="alert"` for errors requiring immediate action. | Existing inline errors, pump alerts, hardware error, Wi-Fi feedback, and toast error; normalize in both phases. |
| `empty` | A completed request returned no usable items. | `<nothing-found> <next-action>` such as `ไม่พบเครือข่าย Wi-Fi ลองสแกนอีกครั้ง` | Use a quiet empty result row or panel, not an error color. | Offer the next available recovery action. | Announce dynamic results with `aria-live="polite"`. | Wi-Fi no-network row; complete in Phase 13. |
| `disabled` | An action cannot run now. | Pair with a `processing`, `unavailable`, or `pending reboot` reason when not obvious. | Preserve semantic `disabled`, reduce emphasis, and keep the reason readable. | Do not hide a valid recovery action. | Preserve native disabled semantics and adjacent explanatory text. | Existing buttons and Wi-Fi panel; normalize in Phase 12 and Phase 13. |
| `unavailable` | A safe prerequisite is absent. | `ยัง<action>ไม่ได้: <reason>` | Keep the affected state visible, use warning or neutral treatment, and avoid color-only meaning. | Offer refresh, reconnect, selection, or wiring guidance when applicable. | Announce changed dynamic state with `aria-live="polite"`. | Pump stale/fault, cooling sensor fault, Wi-Fi missing selection; normalize in both phases. |
| `pending reboot` | New hardware values are saved but not active. | `บันทึก GPIO แล้ว ต้องรีบูต ESP32 ก่อนค่าชุดใหม่จะทำงาน` | Show active and pending GPIO values separately with durable reboot guidance. | Allow review; do not imply pending values are active. | Announce save confirmation politely and keep the durable reboot status programmatically recognizable. | Hardware/Install active and pending map; complete in Phase 13. |
| `disconnected` | STA is not connected to a router. | `STA ยังไม่ได้เชื่อมต่อ เลือกเครือข่าย Wi-Fi หรือใช้ AP <ip>` | Keep SoftAP fallback visible and distinguish disconnected from device unreachable. | Offer scan and connect recovery. | Expose the status as recognizable text and announce dynamic changes politely. | Wi-Fi connection status and Status STA card; complete in Phase 13. |

## Contextual Message Versus Toast

| Message type | Use for | Persistence | Examples |
| --- | --- | --- | --- |
| Contextual status | Safety state, unavailable reason, pending reboot, field validation, connect progress, or durable result. | Remains near the affected control until state changes. | Pump stale state, hardware reboot requirement, Wi-Fi connection result. |
| Toast | Short confirmation that does not carry the only copy of a safety or recovery message. | Temporary. | Pump start accepted, settings saved, Wi-Fi disconnected. |

Never use a toast as the only place for an error, pending reboot requirement, or
safety-relevant unavailable reason.

## Presentation Rules

- Use semantic CSS variables from `DESIGN.md`: blue for primary action and
  focus, green for healthy or completed state, orange for warning, and red for
  error or destructive action.
- Pair every semantic color with text or a meaningful icon.
- Keep action labels specific: `บันทึกค่า Timer`, `บันทึกค่า Cooling`,
  `บันทึก GPIO สำหรับรีบูต`, and `เชื่อมต่อ Wi-Fi`.
- Keep a clear `2-4px` blue `:focus-visible` ring on every interactive control.
- Keep every mobile interactive target at least `44x44px`.
- Use `aria-live="polite"` for informational and success regions.
- Use `role="alert"` for immediate errors requiring user action.
- Respect `prefers-reduced-motion: reduce` for pulse, toast, spinner, and
  progress transitions.
- Preserve offline operation: no CDN, remote font, remote icon package, or
  framework runtime.

## Ownership Map

| Surface | Phase 12 responsibility | Phase 13 responsibility |
| --- | --- | --- |
| Login | Loading and authentication-error recovery plus shared focus and announcement primitives. | None beyond regression. |
| App Shell | Consistent desktop and mobile navigation plus accessible names. | Apply shared shell behavior to remaining pages if needed. |
| Dashboard | Pump-first hierarchy, cooling separation, daily-control emphasis, and shared state primitives. | Complete setup-state consistency where Dashboard and Hardware/Install overlap. |
| Hardware/Install | Shared primitives inherited from Phase 12. | Active versus pending map, pending reboot, save feedback, and installer mobile behavior. |
| Wi-Fi | Shared primitives inherited from Phase 12. | Loading, empty, disconnected, connected, error recovery, touch targets, and row semantics. |
| Status | Shared primitives inherited from Phase 12. | Scannable diagnostic layout and long-value wrapping. |
