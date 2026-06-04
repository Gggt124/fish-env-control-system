---
status: complete
---
# Quick Task Summary: 20260603-theme-switch

Implemented a dark/light mode toggle with NVS persistence and zero-flash loading.

- Added `nvs_store_save_ui_theme` and `nvs_store_load_ui_theme` to ESP-IDF backend.
- Registered `/api/theme` GET/POST endpoints in `web_server.c`.
- Appended premium dark theme tokens to `style.css`.
- Injected `<script>` blocks in the `<head>` of HTML files to read `localStorage` immediately, preventing white flashes.
- Added `app.js` logic to toggle themes, update `localStorage`, and sync with NVS.
- Added `🌙` theme toggle buttons in the navbar and sidebar.