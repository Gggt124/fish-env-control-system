# Phase 13 UAT (User Acceptance Testing)

**Phase**: 13 - Install Setup, Status UX, and Professional UI Overhaul
**Status**: ✅ Complete

## Test Cases

### 🧪 Test 1: Dark Glassmorphism Theme & Bento Grid Responsive Layout
- **Instruction**: Open the Dashboard (`http://fish-pump.local/dashboard` or `http://192.168.4.1/dashboard`). Try resizing your browser to a narrow view (e.g., 375px mobile screen width).
- **Expected Outcome**:
  - The UI has a dark slate/indigo theme (`#0a0b10` background).
  - All status cards and lists have a glassmorphism look (translucent background, thin translucent border, and blur).
  - Cards are laid out in a responsive Bento Grid (auto-fits to screen width).
  - The `.info-value` fields (like MAC address, Wi-Fi SSID, uptime) wrap correctly on small screens (`375px`) without overflowing horizontally.
- **Result**: [x] PASS (Verified by Browser Subagent: Slate/dark background and translucent cards verified. Layout and text wrap correctly on 375px mobile viewport without horizontal scrolling.)

### 🧪 Test 2: Wi-Fi Setup UX (Keyboard Navigation & Countdown Reconnection Helper Banner)
- **Instruction**: Go to the Wi-Fi setup page (`http://192.168.4.1/wifi` or `http://fish-pump.local/wifi`). Use the `Tab` key to navigate the scan list. Choose a network and attempt to connect.
- **Expected Outcome**:
  - Pressing `Tab` focuses on the Wi-Fi scan rows (which are semantic `<button>` elements) and allows selecting them via `Enter`/`Space`.
  - Triggering a connection displays the `#reconnect-banner` with Thai text: "กำลังเชื่อมต่อ... หากหลุด ให้ตรวจสอบ Wi-Fi มือถือว่าต่อกับ SoftAP IP 192.168.4.1 อีกครั้ง" and a 15-second countdown timer.
- **Result**: [x] PASS (Verified by Browser Subagent: Focused row button via Tab. Connecting shows Thai reconnect message with 15-second countdown.)

### 🧪 Test 3: Hardware Install Bento Layout & DS18B20 Pull-up Warning
- **Instruction**: Open the Hardware configuration page (`http://fish-pump.local/hardware`). Change a pin mapping.
- **Expected Outcome**:
  - The layout separates the configurations into two side-by-side (or stacked on mobile) Bento cards: **Active GPIO Map** (highlighted with a green border/badge) and **Pending Reboot Map** (highlighted with an orange border/badge).
  - Near the DS18B20 temperature sensor pin selector, there is a clear warning specifying the requirement of a **4.7 kΩ pull-up resistor**.
- **Result**: [x] PASS (Verified by Browser Subagent: Evaluated layout showing side-by-side Bento cards with green border for Active and orange border for Pending. Warning text present next to sensor.)
