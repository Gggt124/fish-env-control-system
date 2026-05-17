#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NVS store for Wi-Fi credentials.
 * Namespace: "wifi_cfg"
 * Keys: "sta_ssid", "sta_pass"
 */

#define AP_STOP_TMO_DEFAULT_MS  60000
#define AP_AUTO_STOP_DEFAULT    true

bool nvs_store_init(void);
bool nvs_store_save_wifi(const char *ssid, const char *password);
bool nvs_store_load_wifi(char *ssid_out, size_t ssid_len, char *pass_out, size_t pass_len);
bool nvs_store_clear_wifi(void);

bool nvs_store_save_ap_config(uint32_t stop_timeout_ms, bool auto_stop);
bool nvs_store_load_ap_config(uint32_t *stop_timeout_ms, bool *auto_stop);

#ifdef __cplusplus
}
#endif
