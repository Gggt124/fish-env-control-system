import unittest
import os
import re

BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
STATIC_DIR = os.path.join(BASE_DIR, 'main', 'static')

def read_file(name):
    with open(os.path.join(STATIC_DIR, name), 'r', encoding='utf-8') as f:
        return f.read()

class TestUIPhase13(unittest.TestCase):
    def setUp(self):
        self.style = read_file('style.css')
        self.app = read_file('app.js')
        self.hardware = read_file('hardware.html')
        self.wifi = read_file('wifi.html')
        self.status = read_file('status.html')

    def test_req1_dark_theme(self):
        # Must define premium dark theme (#0a0b10 or #0f111a)
        self.assertTrue(
            '#0a0b10' in self.style.lower() or '#0f111a' in self.style.lower() or 
            'rgba(15, 17, 26' in self.style.lower(),
            "style.css does not contain the required dark theme background colors."
        )

    def test_req2_wifi_keyboard_operability(self):
        # app.js must render Wi-Fi items as <button> or role="button"
        self.assertTrue(
            '<button' in self.app and 'network-item' in self.app or 'role="button"' in self.app,
            "app.js does not seem to render network items as <button> or with role='button'"
        )
        # More precise check
        has_button = re.search(r'<button[^>]*network-item', self.app)
        has_role = re.search(r'role=[\'"]button[\'"]', self.app)
        self.assertTrue(has_button or has_role, "No valid button or role='button' for network items in app.js")

    def test_req3_active_vs_pending_gpio(self):
        # hardware.html must contain active and pending cards
        self.assertIn('Active GPIO Map', self.hardware)
        self.assertIn('Pending Reboot Map', self.hardware)
        self.assertIn('var(--secondary)', self.hardware)
        self.assertIn('var(--tertiary)', self.hardware)

    def test_req4_wifi_reconnection_banner(self):
        self.assertIn('192.168.4.1', self.wifi)
        
    def test_req5_accessibility(self):
        # min-height: 44px or min-width: 44px for touch targets
        self.assertRegex(self.style, r'min-(height|width):\s*44px', "Touch targets min 44px not found")
        # prefers-reduced-motion
        self.assertIn('prefers-reduced-motion: reduce', self.style, "prefers-reduced-motion not found in CSS")

    def test_req6_no_cdn(self):
        # Ensure no http/https imports for fonts or icons
        for name, content in [('style.css', self.style), ('hardware.html', self.hardware), ('wifi.html', self.wifi), ('status.html', self.status)]:
            matches = re.findall(r'https?://[^\s\'"]+', content)
            self.assertEqual(len(matches), 0, f"External CDN links found in {name}: {matches}")

if __name__ == '__main__':
    unittest.main()
