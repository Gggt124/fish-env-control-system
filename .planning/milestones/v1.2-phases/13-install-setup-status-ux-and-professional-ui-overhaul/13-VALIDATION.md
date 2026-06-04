# Phase 13 Validation

## Requirements Map

| ID | Requirement | Test File | Test Type | Command | Status |
|---|---|---|---|---|---|
| REQ-1 | `style.css` must define premium dark theme (`#0a0b10` or `#0f111a` surfaces). | `tests/test_ui_phase13.py` | Unit (Static) | `python tests/test_ui_phase13.py` | escalated |
| REQ-2 | `app.js` must render Wi-Fi items as `<button>` or `role="button"` (Keyboard Operability). | `tests/test_ui_phase13.py` | Unit (Static) | `python tests/test_ui_phase13.py` | green |
| REQ-3 | `hardware.html` must contain "Active" (green/secondary) and "Pending" (orange/tertiary) bento cards. | `tests/test_ui_phase13.py` | Unit (Static) | `python tests/test_ui_phase13.py` | green |
| REQ-4 | `wifi.html` must contain a reconnection helper banner referencing "192.168.4.1". | `tests/test_ui_phase13.py` | Unit (Static) | `python tests/test_ui_phase13.py` | green |
| REQ-5 | `style.css` must include touch targets min 44x44px and `@media (prefers-reduced-motion: reduce)`. | `tests/test_ui_phase13.py` | Unit (Static) | `python tests/test_ui_phase13.py` | green |
| REQ-6 | No external CDNs (fonts/icons) used in `.html` or `.css` files. | `tests/test_ui_phase13.py` | Unit (Static) | `python tests/test_ui_phase13.py` | green |
