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

#define APP_TEMPLATE_NAME                  "ESP32 Control Template"
#define APP_TEMPLATE_FIRMWARE_VERSION      "v1.0.0"
#define APP_TEMPLATE_PHASE_LABEL           "Wi-Fi Setup/Web Server Template"

#define APP_TEMPLATE_AP_SSID               "ESP32-Control-Setup"
#define APP_TEMPLATE_AP_CHANNEL            1
#define APP_TEMPLATE_AP_MAX_CONN           4
#define APP_TEMPLATE_WIFI_SCAN_MAX         20

#define APP_TEMPLATE_MDNS_HOSTNAME         "esp32-setup"
#define APP_TEMPLATE_MDNS_INSTANCE_NAME    "ESP32 Dashboard"

#define APP_TEMPLATE_DEFAULT_USERNAME      "admin"
#define APP_TEMPLATE_DEFAULT_PASSWORD      "admin123"

#define APP_TEMPLATE_SESSION_MAX_AGE_SEC   1800
#define APP_TEMPLATE_LOGIN_RATE_LIMIT_MAX  5
#define APP_TEMPLATE_LOGIN_RATE_LIMIT_SEC  30

#define APP_TEMPLATE_AP_STOP_TMO_DEFAULT_MS 60000
#define APP_TEMPLATE_AP_AUTO_STOP_DEFAULT   true

#define APP_TEMPLATE_HTTP_MAX_URI_HANDLERS 18
#define APP_TEMPLATE_MAIN_WDT_TIMEOUT_MS   30000
#define APP_TEMPLATE_STATUS_LOG_INTERVALS  6

#ifdef __cplusplus
}
#endif
