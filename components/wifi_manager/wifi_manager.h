#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_wifi_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AP_SSID         "ESP32-Control-Setup"
#define AP_MAX_CONN     4
#define WIFI_SCAN_MAX   20

typedef struct {
    char ssid[33];
    int8_t rssi;
    char auth[16];
    uint8_t channel;
} wifi_scan_entry_t;

typedef void (*wifi_scan_cb_t)(void *user_ctx, wifi_scan_entry_t *entries, int count);

bool wifi_manager_init(void);
bool wifi_manager_start_ap(void);
bool wifi_manager_stop_ap(void);
bool wifi_manager_connect_sta(const char *ssid, const char *password);
bool wifi_manager_disconnect_sta(void);
bool wifi_manager_forget_sta(void);
bool wifi_manager_scan(void *user_ctx, wifi_scan_cb_t callback);

void wifi_manager_sta_disconnect_for_scan(void);
void wifi_manager_sta_reconnect_after_scan(void);

bool wifi_manager_is_ap_enabled(void);
bool wifi_manager_is_sta_connected(void);
char* wifi_manager_get_ap_ip(void);
char* wifi_manager_get_sta_ip(void);
char* wifi_manager_get_sta_ssid(void);
int64_t wifi_manager_get_uptime_ms(void);
uint32_t wifi_manager_get_free_heap(void);

const char* wifi_auth_mode_to_string(wifi_auth_mode_t mode);

#ifdef __cplusplus
}
#endif
