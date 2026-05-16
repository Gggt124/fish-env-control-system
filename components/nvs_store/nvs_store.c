#include "nvs_store.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>

#define NVS_NAMESPACE "wifi_cfg"
#define NVS_KEY_SSID  "sta_ssid"
#define NVS_KEY_PASS  "sta_pass"

bool nvs_store_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    return ret == ESP_OK;
}

bool nvs_store_save_wifi(const char *ssid, const char *password)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) return false;

    bool ok = true;
    if (nvs_set_str(handle, NVS_KEY_SSID, ssid) != ESP_OK) ok = false;
    if (password && password[0]) {
        if (nvs_set_str(handle, NVS_KEY_PASS, password) != ESP_OK) ok = false;
    } else {
        nvs_erase_key(handle, NVS_KEY_PASS);
    }

    if (ok) nvs_commit(handle);
    nvs_close(handle);
    return ok;
}

bool nvs_store_load_wifi(char *ssid_out, size_t ssid_len, char *pass_out, size_t pass_len)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) != ESP_OK) {
        if (ssid_out) ssid_out[0] = '\0';
        if (pass_out) pass_out[0] = '\0';
        return false;
    }

    bool has_ssid = false;
    size_t len = ssid_len;
    if (nvs_get_str(handle, NVS_KEY_SSID, ssid_out, &len) == ESP_OK && ssid_out[0]) {
        has_ssid = true;
    } else {
        ssid_out[0] = '\0';
    }

    len = pass_len;
    if (nvs_get_str(handle, NVS_KEY_PASS, pass_out, &len) != ESP_OK) {
        pass_out[0] = '\0';
    }

    nvs_close(handle);
    return has_ssid;
}

bool nvs_store_clear_wifi(void)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) return false;
    nvs_erase_key(handle, NVS_KEY_SSID);
    nvs_erase_key(handle, NVS_KEY_PASS);
    nvs_commit(handle);
    nvs_close(handle);
    return true;
}
