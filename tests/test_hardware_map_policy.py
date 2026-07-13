import os
import re
import unittest


BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
APP_CONFIG_PATH = os.path.join(
    BASE_DIR, "components", "app_config", "app_config.h"
)
HARDWARE_MAP_HEADER_PATH = os.path.join(
    BASE_DIR, "components", "hardware_map", "hardware_map.h"
)
HARDWARE_MAP_SOURCE_PATH = os.path.join(
    BASE_DIR, "components", "hardware_map", "hardware_map.c"
)
WEB_SERVER_PATH = os.path.join(BASE_DIR, "main", "web_server.c")
PUMP_CONTROL_PATH = os.path.join(
    BASE_DIR, "components", "pump_control", "pump_control.c"
)
NVS_STORE_PATH = os.path.join(
    BASE_DIR, "components", "nvs_store", "nvs_store.c"
)


def read_file(path):
    with open(path, "r", encoding="utf-8") as file:
        return file.read()


class TestHardwareMapPolicy(unittest.TestCase):
    def setUp(self):
        self.app_config = read_file(APP_CONFIG_PATH)
        self.hardware_map_header = read_file(HARDWARE_MAP_HEADER_PATH)
        self.hardware_map_source = read_file(HARDWARE_MAP_SOURCE_PATH)
        self.web_server = read_file(WEB_SERVER_PATH)
        self.pump_control = read_file(PUMP_CONTROL_PATH)
        self.nvs_store = read_file(NVS_STORE_PATH)
        self.s3_options = self.hardware_map_source.split(
            "#ifdef CONFIG_IDF_TARGET_ESP32S3", 1
        )[1].split("#else", 1)[0]

    def test_s3_defaults_use_usb_and_boot_safe_gpio_pool(self):
        expected_defaults = {
            "APP_TEMPLATE_PUMP_FLOAT_GPIO": 16,
            "APP_TEMPLATE_PUMP_RELAY_GPIO": 5,
            "APP_TEMPLATE_HW_DEFAULT_FLOAT_GPIO": 16,
            "APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY1_GPIO": 5,
            "APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY2_GPIO": 6,
            "APP_TEMPLATE_HW_DEFAULT_DS18B20_GPIO": 7,
            "APP_TEMPLATE_HW_DEFAULT_COOLING_RELAY_GPIO": 8,
        }

        for macro, expected in expected_defaults.items():
            pattern = rf"#define\s+{macro}\s+{expected}\b"
            self.assertRegex(
                self.app_config,
                pattern,
                f"{macro} must use GPIO{expected} for the S3 profile",
            )

    def test_s3_options_exclude_system_and_special_function_pins(self):
        forbidden_pins = {
            0, 3, 4, 9, 10, 11, 12, 13, 14,
            19, 20, 21, 27, 28, 29, 30, 31, 32,
            33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
            43, 44, 45, 46, 47, 48,
        }
        configured_pins = {
            int(pin)
            for pin in re.findall(r"HARDWARE_PIN_GPIO(\d+)", self.s3_options)
        }

        self.assertTrue(configured_pins)
        self.assertTrue(
            configured_pins.isdisjoint(forbidden_pins),
            f"S3 option arrays expose forbidden GPIOs: "
            f"{sorted(configured_pins & forbidden_pins)}",
        )

    def test_pin_policy_is_typed_and_centralized(self):
        self.assertIn("hardware_pin_t", self.hardware_map_header)
        self.assertIn("hardware_pin_capability_t", self.hardware_map_header)
        self.assertIn("hardware_map_pin_descriptor", self.hardware_map_header)
        self.assertIn("hardware_map_pin_is_reserved", self.hardware_map_header)
        self.assertIn("hardware_map_pin_descriptor", self.hardware_map_source)
        self.assertIn("hardware_map_pin_is_reserved", self.hardware_map_source)

    def test_hardware_api_exposes_typed_pin_identity(self):
        self.assertIn('"pin_id"', self.web_server)
        self.assertIn("options[i].pin", self.web_server)

    def test_float_role_allows_external_pull_only_inputs(self):
        self.assertRegex(
            self.hardware_map_source,
            re.compile(
                r"case HARDWARE_ROLE_FLOAT_INPUT:.*?return HARDWARE_PIN_CAP_INPUT;",
                re.DOTALL,
            ),
        )

    def test_input_only_float_pins_disable_internal_pull(self):
        self.assertIn("bool float_input_only", self.pump_control)
        self.assertRegex(
            self.pump_control,
            r"\.pull_up_en = float_input_only\s*\?\s*GPIO_PULLUP_DISABLE",
        )
        self.assertRegex(
            self.pump_control,
            r"\.pull_down_en = float_input_only\s*\?\s*GPIO_PULLDOWN_DISABLE",
        )

    def test_legacy_relay2_polarity_inherits_legacy_relay_polarity(self):
        self.assertRegex(
            self.nvs_store,
            re.compile(
                r"if \(r2_ret == ESP_ERR_NVS_NOT_FOUND\).*?"
                r"relay2_active_low = relay_active_low;",
                re.DOTALL,
            ),
        )

    def test_incompatible_pump_blob_is_not_reported_as_loaded(self):
        incompatible_branch = re.search(
            r'if \(blob_len > 0.*?\n        \} else \{(.*?)\n        \}',
            self.nvs_store,
            re.DOTALL,
        )
        self.assertIsNotNone(incompatible_branch)
        self.assertIn(
            "NVS_STORE_PUMP_SETTINGS_DEFAULTS_INVALID",
            incompatible_branch.group(1),
        )

    def test_invalid_pump_blob_length_or_read_error_is_rejected(self):
        self.assertIn("blob_len <= 1", self.nvs_store)
        self.assertIn(
            "Pump blob read failed; suppressing pump start",
            self.nvs_store.replace(
                "Pump blob read failed (%s); suppressing pump start",
                "Pump blob read failed; suppressing pump start",
            ),
        )


if __name__ == "__main__":
    unittest.main()
