#pragma once

/*
 * Template-level configuration.
 *
 * When using this repository as a base for another project, start here:
 * change product names, AP SSID, mDNS hostname, credentials, and limits in
 * this file before touching the Wi-Fi or web-server internals.
 */

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define APP_TEMPLATE_NAME                  "Fish Pump Relay Timer Control"
#define APP_TEMPLATE_FIRMWARE_VERSION      "v0.1.8"
#define APP_TEMPLATE_PHASE_LABEL           "Wi-Fi Setup and Control Dashboard"

#define APP_TEMPLATE_AP_SSID               "FishPump-Setup"
#define APP_TEMPLATE_AP_PASSWORD           "admin123"
#define APP_TEMPLATE_AP_CHANNEL            1
#define APP_TEMPLATE_AP_MAX_CONN           4
#define APP_TEMPLATE_WIFI_SCAN_MAX         20
#define APP_TEMPLATE_WIFI_MAX_TX_POWER_QDBM 80
#define APP_TEMPLATE_WIFI_SCAN_ACTIVE_MIN_MS 30
#define APP_TEMPLATE_WIFI_SCAN_ACTIVE_MAX_MS 60
#define APP_TEMPLATE_WIFI_SCAN_HOME_DWELL_MS 30

#define APP_TEMPLATE_MDNS_HOSTNAME         "fish-pump"
#define APP_TEMPLATE_MDNS_INSTANCE_NAME    "Fish Pump Dashboard"

#define APP_TEMPLATE_DEFAULT_USERNAME      "admin"
#define APP_TEMPLATE_DEFAULT_PASSWORD      "admin123"

#define APP_TEMPLATE_SESSION_MAX_AGE_SEC   2592000
#define APP_TEMPLATE_LOGIN_RATE_LIMIT_MAX  5
#define APP_TEMPLATE_LOGIN_RATE_LIMIT_SEC  30

#define APP_TEMPLATE_AP_STOP_TMO_DEFAULT_MS 60000
#define APP_TEMPLATE_AP_AUTO_STOP_DEFAULT   true

#define APP_TEMPLATE_HTTP_MAX_URI_HANDLERS 64
#define APP_TEMPLATE_HTTP_MAX_OPEN_SOCKETS 14
#define APP_TEMPLATE_HTTP_RECV_TIMEOUT_SEC 8
#define APP_TEMPLATE_HTTP_SEND_TIMEOUT_SEC 2
#define APP_TEMPLATE_HTTP_STATIC_CHUNK_BYTES 2048
#define APP_TEMPLATE_HTTP_STATIC_MAX_DURATION_MS 6000
#define APP_TEMPLATE_HTTP_KEEP_ALIVE_IDLE_SEC 15
#define APP_TEMPLATE_HTTP_KEEP_ALIVE_INTERVAL_SEC 3
#define APP_TEMPLATE_HTTP_KEEP_ALIVE_COUNT 3
#define APP_TEMPLATE_HTTP_SLOW_REQUEST_MS  1000
#define APP_TEMPLATE_MAIN_WDT_TIMEOUT_MS   15000
#define APP_TEMPLATE_STATUS_LOG_INTERVALS  6

/* FLOAT SWITCH PULL-UP NOTE: ESP32 internal pull-up (~45kΩ) may be insufficient
 * to reject EMI from the pump's electromagnetic field.
 * Recommended: add a 4.7kΩ external pull-up resistor between FLOAT_GPIO and 3.3V.
 * See hardware installation guide in the web dashboard for wiring diagram. */
#define APP_TEMPLATE_PUMP_FLOAT_GPIO        32
#define APP_TEMPLATE_PUMP_FLOAT_ACTIVE_LOW  true
#define APP_TEMPLATE_PUMP_RELAY_GPIO        26
#define APP_TEMPLATE_PUMP_RELAY_ACTIVE_LOW  true

#define APP_TEMPLATE_HW_DEFAULT_FLOAT_GPIO              32
#define APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY1_GPIO        26
#define APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY2_GPIO        27
#define APP_TEMPLATE_HW_DEFAULT_DS18B20_GPIO            33
#define APP_TEMPLATE_HW_DEFAULT_COOLING_RELAY_GPIO      25
#define APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY1_ACTIVE_LOW  true
#define APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY2_ACTIVE_LOW  true
#define APP_TEMPLATE_HW_DEFAULT_COOLING_RELAY_ACTIVE_LOW true
#define APP_TEMPLATE_PUMP_TIMER_START_ON                true
#define APP_TEMPLATE_COOLING_THRESHOLD_C_X10            300
#define APP_TEMPLATE_COOLING_HYSTERESIS_C_X10           10
#define APP_TEMPLATE_COOLING_AUTO_ENABLE_DEFAULT        false
#define APP_TEMPLATE_COOLING_TEST_TIMEOUT_SEC           10
#define APP_TEMPLATE_COOLING_MIN_OFF_SEC                180

#define APP_TEMPLATE_PUMP_TIMER1_ON_SEC     20
#define APP_TEMPLATE_PUMP_TIMER1_OFF_SEC    60
#define APP_TEMPLATE_PUMP_TIMER2_ON_SEC     10
#define APP_TEMPLATE_PUMP_TIMER2_OFF_SEC    180
#define APP_TEMPLATE_PUMP_TIMER_MIN_SEC     5
#define APP_TEMPLATE_PUMP_TIMER_MAX_SEC     86400
#define APP_TEMPLATE_PUMP_AUTO_START_DEFAULT false
#define APP_TEMPLATE_PUMP_FLOAT_DEBOUNCE_MS 100

#define APP_TEMPLATE_TFT_CS_GPIO            5
#define APP_TEMPLATE_TFT_RESET_GPIO         22
#define APP_TEMPLATE_TFT_DC_GPIO            21
#define APP_TEMPLATE_TFT_MOSI_GPIO          23
#define APP_TEMPLATE_TFT_SCK_GPIO           18
#define APP_TEMPLATE_TFT_LED_GPIO           4

// Enable ST7789 panel driver instead of ILI9341 driver
#define APP_TEMPLATE_TFT_PANEL_ST7789

// SoftAP recovery and rollback timing constants
#define APP_CONFIG_AP_RECOVERY_TIMEOUT_MS         300000
#define APP_CONFIG_AP_IDLE_TIMEOUT_MS             600000
#define APP_CONFIG_ROLLBACK_WIFI_TIMEOUT_MS       30000
#define APP_CONFIG_ROLLBACK_CONFIRM_TIMEOUT_MS    180000

// Hardware recovery buttons and LEDs GPIO mapping
/* STRAPPING PIN WARNING: GPIO 0 is ESP32's boot-mode strapping pin.
 * Holding it LOW during power-on forces Download Mode.
 * For production: move factory-reset button to GPIO 14 (APP_CONFIG_EXT_BTN_GPIO)
 * which is already defined above and free of boot-mode side effects.
 * Current GPIO 0 assignment is acceptable for development if no external
 * component pulls it LOW at power-on. Do NOT add external pull-down to GPIO 0. */
#define APP_CONFIG_BOOT_BTN_GPIO                  0
#define APP_CONFIG_EXT_BTN_GPIO                   14
#define APP_CONFIG_LED_GPIO                       2   // Strapping pin, avoid external pull-ups that might affect boot mode.
#define APP_CONFIG_EXT_LED_GPIO                   13

// 32-byte (256-bit) AES key in hex format for Pre-Encrypted OTA.
#define APP_CONFIG_OTA_ENCRYPTION_KEY "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"

// Critical thresholds (below these = add a strike)
#define APP_CONFIG_OOM_MIN_FREE_HEAP_BYTES         10240  // 10KB
#define APP_CONFIG_OOM_MIN_LARGEST_BLOCK_BYTES     5120   // 5KB

// Recovery thresholds (must exceed BOTH of these to reset strikes to 0)
#define APP_CONFIG_OOM_RECOVER_FREE_HEAP_BYTES     15360  // 15KB
#define APP_CONFIG_OOM_RECOVER_LARGEST_BLOCK_BYTES 8192   // 8KB

#define APP_CONFIG_OOM_CONSECUTIVE_FAILURES_RESTART 3     // 15 seconds (3 * 5s loop)

#ifdef __cplusplus
}
#endif

