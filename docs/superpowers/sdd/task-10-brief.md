### Task 10: UI static-asset tests for min_dwell field

**Files:**
- Create: `tests/test_min_dwell.py`

**Interfaces:**
- Consumes: `main/static/index.html`, `main/static/app.js` (read as text, like the existing `tests/test_ui_phase16.py`).
- Produces: pytest assertions that the new input id, error id, and badge id exist, and that `app.js` references them.

- [ ] **Step 1: Write the failing test**

Create `tests/test_min_dwell.py`:

```python
import unittest
import os
import re

BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
STATIC_DIR = os.path.join(BASE_DIR, 'main', 'static')

def read_file(name):
    with open(os.path.join(STATIC_DIR, name), 'r', encoding='utf-8') as f:
        return f.read()

class TestMinDwell(unittest.TestCase):
    def setUp(self):
        self.html = read_file('index.html')
        self.app = read_file('app.js')

    def test_min_dwell_input_exists(self):
        self.assertIn('id="pump-min-dwell"', self.html,
                      "index.html must contain the pump-min-dwell input")
        self.assertRegex(self.html, r'id="pump-min-dwell"\s+[^>]*type="number"',
                         "pump-min-dwell must be a number input")
        self.assertRegex(self.html, r'min="0"[^>]*max="3600"|max="3600"[^>]*min="0"',
                         "pump-min-dwell must clamp to 0..3600")

    def test_min_dwell_error_element_exists(self):
        self.assertIn('id="pump-min-dwell-error"', self.html,
                      "index.html must contain the pump-min-dwell-error element")

    def test_cooldown_badge_element_exists(self):
        self.assertIn('id="pump-cooldown-badge"', self.html,
                      "index.html must contain the pump-cooldown-badge element")

    def test_app_js_reads_min_dwell(self):
        self.assertIn("pump-min-dwell", self.app,
                      "app.js must reference the pump-min-dwell element")
        self.assertIn("min_dwell_sec", self.app,
                      "app.js must reference min_dwell_sec in the payload/status")

    def test_app_js_renders_cooldown(self):
        self.assertIn("cooldown_remaining_sec", self.app,
                      "app.js must read cooldown_remaining_sec from status")
        self.assertIn("pump-cooldown-badge", self.app,
                      "app.js must update the pump-cooldown-badge element")

if __name__ == '__main__':
    unittest.main()
```

- [ ] **Step 2: Run the test to verify it passes (Tasks 7+8 already implemented)**

Run: `python -m pytest tests/test_min_dwell.py -v` (in `C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl`)
Expected: PASS (5 tests). If any fail, fix the corresponding HTML/JS from Tasks 7/8 and re-run.

- [ ] **Step 3: Commit**

```powershell
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" add tests/test_min_dwell.py
git -C "C:\Users\Copter\OneDrive\Desktop\fish-pump-RelayTimerControl" commit -m "test(ui): assert min_dwell_sec field and cooldown badge exist in static assets"
```

---

