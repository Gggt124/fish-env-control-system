import unittest
import os
import re

BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
STATIC_DIR = os.path.join(BASE_DIR, 'main', 'static')

def read_file(name):
    with open(os.path.join(STATIC_DIR, name), 'r', encoding='utf-8') as f:
        return f.read()

class TestUIPhase16(unittest.TestCase):
    def setUp(self):
        self.wifi = read_file('wifi.html')
        self.style = read_file('style.css')
        self.app = read_file('app.js')

    def test_wifi_empty_state_card_exists(self):
        self.assertIn('id="empty-state-card"', self.wifi, "wifi.html does not contain empty-state-card ID")
        self.assertTrue(
            '📶' in self.wifi or '👈' in self.wifi,
            "Empty state card must contain '📶' or '👈' icon"
        )
        self.assertIn('โปรดเลือกเครือข่าย', self.wifi, "Empty state card must contain heading 'โปรดเลือกเครือข่าย'")

    def test_no_overlay_hint_or_backdrop_filter_in_wifi(self):
        self.assertNotIn('overlay-hint', self.wifi, "wifi.html must not contain old overlay-hint elements")
        self.assertNotIn('backdrop-filter', self.wifi, "wifi.html must not contain inline backdrop-filter attributes")

    def test_css_empty_state_card_styles(self):
        # Must define styles for .empty-state-card using design system variables
        self.assertIn('.empty-state-card', self.style, "style.css must define .empty-state-card rules")
        self.assertIn('var(--surface-container-low)', self.style, "style.css must use var(--surface-container-low) for background")
        self.assertIn('var(--outline-variant)', self.style, "style.css must use var(--outline-variant) for border")
        self.assertIn('var(--radius-lg)', self.style, "style.css must use var(--radius-lg) for border-radius")

    def test_css_transitions_defined(self):
        # Opacity transitions must be configured
        self.assertRegex(self.style, r'\.empty-state-card\s*,\s*\.input-panel', "style.css must define common styles or transition group for both components")
        self.assertIn('transition', self.style, "style.css must define transition rules")
        self.assertRegex(self.style, r'opacity\s+0\.2s', "Opacity transition duration must be 0.2s")

    def test_css_no_backdrop_blur(self):
        # style.css must not use 'backdrop-filter: blur' or '-webkit-backdrop-filter: blur' inside Wi-Fi setup rules
        # We can just check the whole style.css doesn't contain blur unless it's outside. But let's check it's not present at all, or not in wifi context
        self.assertNotIn('backdrop-filter: blur', self.style)
        self.assertNotIn('-webkit-backdrop-filter: blur', self.style)

    def test_css_prefers_reduced_motion(self):
        self.assertIn('prefers-reduced-motion: reduce', self.style, "prefers-reduced-motion block must be present")
        self.assertRegex(self.style, r'prefers-reduced-motion:\s*reduce.*transition:\s*0s', "prefers-reduced-motion must override transitions to 0s")

    def test_js_references_and_fadeswap(self):
        self.assertTrue(
            'empty-state-card' in self.app or 'emptyCard' in self.app,
            "app.js must reference empty-state-card or emptyCard"
        )
        self.assertIn('fadeSwap', self.app, "app.js must implement fadeSwap sequential transition logic")

if __name__ == '__main__':
    unittest.main()
