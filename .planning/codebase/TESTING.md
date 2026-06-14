# Testing Patterns

**Analysis Date:** 2026-06-14

## Test Framework

**Runner:**
- Python `unittest` framework

**Assertion Library:**
- Standard Python `unittest` class assertions (`assertIn`, `assertNotIn`, `assertTrue`, `assertRegex`, `assertEqual`)

**Run Commands:**
```powershell
# Run all static tests
python -m unittest discover -s tests -p "test_*.py"

# Run a specific test suite
python -m unittest tests/test_ui_phase16.py

# Run a single test case within a suite
python -m unittest tests.test_ui_phase16.TestUIPhase16.test_wifi_empty_state_card_exists
```

## Test File Organization

**Location:**
- Located in the root `tests/` directory (separate from `main/` and `components/`).

**Naming:**
- Naming convention: `test_ui_phase[N].py` (where `N` represents the development phase).

**Structure:**
```
fish-pump-RelayTimerControl/
├── main/
│   └── static/
│       ├── style.css
│       └── app.js
└── tests/
    ├── test_ui_phase13.py   # Style and layout assertions
    └── test_ui_phase16.py   # Fades and transitions assertions
```

## Test Structure

**Suite Organization:**
```python
import unittest
import os

class TestUIPhaseExample(unittest.TestCase):
    def setUp(self):
        # Load files under test
        with open('main/static/app.js', 'r', encoding='utf-8') as f:
            self.app = f.read()

    def test_logic_checks(self):
        # Assert specific functions or keywords exist
        self.assertIn('fadeSwap', self.app, "app.js must implement fadeSwap")
```

## Mocking

- **Mocking Strategy:** No mock objects or mock frameworks are used.
- Since tests are static code validation audits (reading raw file characters and checking tags/styling patterns), they do not compile the C code or execute runtime JavaScript.

## Test Types

**Static Layout & Accessibility Audits (Automated):**
- Verify that standard dark theme background colors are defined (`#0a0b10` or `#0f111a`).
- Verify that touch targets are accessible (minimum height/width of 44px).
- Verify that Wi-Fi scan items use standard `<button>` tags or specify `role="button"` for keyboard accessibility.
- Verify that no external HTTP/HTTPS CDN references are present in frontend assets (must work local-only).
- Verify that `@media (prefers-reduced-motion: reduce)` block disables CSS transitions.
- Verify sequential opacity transition durations (0.2s) and empty card components exist.

**Hardware Integration Testing (Manual):**
1. Compile the C firmware by running:
   ```powershell
   .\scripts\build.ps1
   ```
2. Flash the binary to the ESP32 DevKit board:
   ```powershell
   idf.py -p COMx flash monitor
   ```
3. Connect to the SoftAP network `FishPump-Setup`.
4. Navigate to `http://192.168.4.1` and log in with default credentials `admin` / `admin123`.
5. Verify Wi-Fi network scans load, save a staged profile, and verify reconnect logic works.
6. Verify active float switch changes toggle the system between Timer 1 and Timer 2, and observe relays switching states.

---

*Testing analysis: 2026-06-14*
*Update when test patterns change*
