#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "app_config.h"
#include "hardware_map.h"

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
    bool relay1_active_low;
    bool relay2_active_low;
    hardware_timer_start_phase_t timer1_start_phase;
    hardware_timer_start_phase_t timer2_start_phase;
    bool auto_start;
} nvs_store_pump_settings_t;

typedef struct {
    int32_t threshold_c_x10;
    int32_t hysteresis_c_x10;
    bool auto_enable;
    hardware_cooling_mode_t mode;
    uint32_t test_timeout_sec;
    uint32_t compressor_min_off_sec;
    hardware_relay_polarity_t relay_polarity;
} nvs_store_cooling_settings_t;

typedef enum {
    NVS_STORE_PUMP_SETTINGS_LOADED = 0,
    NVS_STORE_PUMP_SETTINGS_DEFAULTS_MISSING,
    NVS_STORE_PUMP_SETTINGS_DEFAULTS_INVALID,
    NVS_STORE_PUMP_SETTINGS_DEFAULTS_ERROR,
} nvs_store_pump_settings_load_status_t;

typedef enum {
    NVS_STORE_HARDWARE_MAP_LOADED = 0,
    NVS_STORE_HARDWARE_MAP_DEFAULTS_MISSING,
    NVS_STORE_HARDWARE_MAP_DEFAULTS_INVALID,
    NVS_STORE_HARDWARE_MAP_DEFAULTS_ERROR,
} nvs_store_hardware_map_load_status_t;

typedef enum {
    NVS_STORE_PENDING_HARDWARE_MAP_LOADED = 0,
    NVS_STORE_PENDING_HARDWARE_MAP_NOT_FOUND,
    NVS_STORE_PENDING_HARDWARE_MAP_INVALID,
    NVS_STORE_PENDING_HARDWARE_MAP_ERROR,
} nvs_store_pending_hardware_map_load_status_t;

typedef enum {
    NVS_STORE_COOLING_SETTINGS_LOADED = 0,
    NVS_STORE_COOLING_SETTINGS_DEFAULTS_MISSING,
    NVS_STORE_COOLING_SETTINGS_DEFAULTS_INVALID,
    NVS_STORE_COOLING_SETTINGS_DEFAULTS_ERROR,
} nvs_store_cooling_settings_load_status_t;

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

void nvs_store_hardware_map_defaults(hardware_map_t *out);
nvs_store_hardware_map_load_status_t nvs_store_load_hardware_map(hardware_map_t *out);
bool nvs_store_save_hardware_map(const hardware_map_t *map);
bool nvs_store_clear_hardware_map(void);
nvs_store_pending_hardware_map_load_status_t nvs_store_load_pending_hardware_map(hardware_map_t *out);
bool nvs_store_save_pending_hardware_map(const hardware_map_t *map);
bool nvs_store_clear_pending_hardware_map(void);
bool nvs_store_hardware_reboot_required(bool *reboot_required);

void nvs_store_cooling_settings_defaults(nvs_store_cooling_settings_t *out);
nvs_store_cooling_settings_load_status_t nvs_store_load_cooling_settings(nvs_store_cooling_settings_t *out);
bool nvs_store_save_cooling_settings(const nvs_store_cooling_settings_t *settings);
bool nvs_store_clear_cooling_settings(void);


#ifdef __cplusplus
}
#endif
