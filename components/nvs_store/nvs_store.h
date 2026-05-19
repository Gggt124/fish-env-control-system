#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "app_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NVS store for Wi-Fi credentials and static IP config.
 * Namespace: "wifi_cfg"
 * Keys: "sta_ssid", "sta_pass", "sta_ip", "sta_gw", "sta_netmask", "sta_dns"
 */

#define AP_STOP_TMO_DEFAULT_MS  APP_TEMPLATE_AP_STOP_TMO_DEFAULT_MS
#define AP_AUTO_STOP_DEFAULT    APP_TEMPLATE_AP_AUTO_STOP_DEFAULT

typedef struct {
    uint32_t timer1_on_sec;
    uint32_t timer1_off_sec;
    uint32_t timer2_on_sec;
    uint32_t timer2_off_sec;
    bool relay_active_low;
    bool auto_start;
} nvs_store_pump_settings_t;

typedef enum {
    NVS_STORE_PUMP_SETTINGS_LOADED = 0,
    NVS_STORE_PUMP_SETTINGS_DEFAULTS_MISSING,
    NVS_STORE_PUMP_SETTINGS_DEFAULTS_INVALID,
    NVS_STORE_PUMP_SETTINGS_DEFAULTS_ERROR,
} nvs_store_pump_settings_load_status_t;

bool nvs_store_init(void);
bool nvs_store_save_wifi(const char *ssid, const char *password);
bool nvs_store_load_wifi(char *ssid_out, size_t ssid_len, char *pass_out, size_t pass_len);
bool nvs_store_clear_wifi(void);

bool nvs_store_save_ap_config(uint32_t stop_timeout_ms, bool auto_stop);
bool nvs_store_load_ap_config(uint32_t *stop_timeout_ms, bool *auto_stop);

bool nvs_store_save_sta_ip(const char *ip, const char *gateway, const char *netmask, const char *dns);
bool nvs_store_load_sta_ip(char *ip_out, size_t ip_len, char *gw_out, size_t gw_len,
                           char *mask_out, size_t mask_len, char *dns_out, size_t dns_len);
bool nvs_store_clear_sta_ip(void);

void nvs_store_pump_settings_defaults(nvs_store_pump_settings_t *out);
nvs_store_pump_settings_load_status_t nvs_store_load_pump_settings(nvs_store_pump_settings_t *out);
bool nvs_store_save_pump_settings(const nvs_store_pump_settings_t *settings);
bool nvs_store_clear_pump_settings(void);

#ifdef __cplusplus
}
#endif
