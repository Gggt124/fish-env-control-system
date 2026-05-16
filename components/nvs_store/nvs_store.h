#pragma once

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NVS store for Wi-Fi credentials.
 * Namespace: "wifi_cfg"
 * Keys: "sta_ssid", "sta_pass"
 */

bool nvs_store_init(void);
bool nvs_store_save_wifi(const char *ssid, const char *password);
bool nvs_store_load_wifi(char *ssid_out, size_t ssid_len, char *pass_out, size_t pass_len);
bool nvs_store_clear_wifi(void);

#ifdef __cplusplus
}
#endif
