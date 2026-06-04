---
status: complete
---
# Quick Task Summary: 260604-remove-dark-theme

Successfully removed dark theme styles, switcher widgets, and API persistence.

- Removed NVS load/save theme methods from `components/nvs_store`.
- Removed UI theme endpoints `/api/theme` and handler functions in `web_server.c`.
- Deleted `:root.theme-dark` rules in `style.css`.
- Deleted theme toggle buttons and inline scripts in `login.html`, `dashboard.html`, `hardware.html`, `status.html`, and `wifi.html`.
- Flashed the updated light-only firmware to COM5.
