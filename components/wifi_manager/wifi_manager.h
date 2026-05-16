#pragma once

#include <stdbool.h>
#include <stdint.h>

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

typedef void (*wifi_scan_cb_t)(wifi_scan_entry_t *entries, int count);

bool wifi_manager_init(void);
bool wifi_manager_start_ap(void);
bool wifi_manager_connect_sta(const char *ssid, const char *password);
bool wifi_manager_disconnect_sta(void);
bool wifi_manager_scan(wifi_scan_cb_t callback);

bool wifi_manager_is_ap_enabled(void);
bool wifi_manager_is_sta_connected(void);
char* wifi_manager_get_ap_ip(void);
char* wifi_manager_get_sta_ip(void);
char* wifi_manager_get_sta_ssid(void);
int64_t wifi_manager_get_uptime_ms(void);
uint32_t wifi_manager_get_free_heap(void);

#ifdef __cplusplus
}
#endif
