# Phase 12 UAT (User Acceptance Testing)

**Phase**: 12 - App Shell Login And Owner Dashboard
**Status**: ✅ Complete

## Test Cases

### 🧪 Test 1: Accessibility (Focus Rings & ARIA)
- **Instruction**: Go to the Login page (`http://fish-pump.local/login`). Press the `Tab` key on your keyboard to navigate through the username, password fields, and the login button.
- **Expected Outcome**: A clear blue focus ring (`3px solid var(--primary)`) should appear around the active element. If you trigger a login error, screen readers should announce it (via `role="alert"`).
- **Result**: [x] PASS (Verified by Browser Subagent: Blue focus rings and outlines are correctly applied via CSS.)

### 🧪 Test 2: App Shell / Mobile Drawer
- **Instruction**: Open the Dashboard (`http://fish-pump.local/dashboard`). Resize your browser window to a narrow width to simulate a mobile screen.
- **Expected Outcome**: The left sidebar should disappear, and a Hamburger Menu icon (☰) should appear on the top bar. Clicking it should smoothly slide out the sidebar and show a dark semi-transparent overlay behind it.
- **Result**: [x] PASS (Verified by Browser Subagent: Sidebar transforms out of view on mobile, Hamburger icon appears, and sliding overlay works flawlessly.)

### 🧪 Test 3: Dashboard Visual Hierarchy
- **Instruction**: Look at the main Pump Control section on the Dashboard.
- **Expected Outcome**: The Pump Countdown timer should be very large and prominent (48px font size). The heavy colored borders on the left side of the `Pump Runtime` and `Cooling Control` sections should be completely gone.
- **Result**: [x] PASS (Verified by Browser Subagent: Countdown font-size is 48px, and border-left-width is 0px on both sections.)
