#include "nvs_store.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "nvs_store";

#define NVS_NAMESPACE    "wifi_cfg"
#define NVS_KEY_SSID     "sta_ssid"
#define NVS_KEY_PASS     "sta_pass"
#define NVS_KEY_AP_TMO   "ap_stop_tmo"
#define NVS_KEY_AP_AUTO  "ap_auto_stop"
#define NVS_KEY_IP       "sta_ip"
#define NVS_KEY_GW       "sta_gw"
#define NVS_KEY_MASK     "sta_netmask"
#define NVS_KEY_DNS      "sta_dns"

#define NVS_PUMP_NAMESPACE       "pump_cfg"
#define NVS_PUMP_KEY_T1_ON       "t1_on"
#define NVS_PUMP_KEY_T1_OFF      "t1_off"
#define NVS_PUMP_KEY_T2_ON       "t2_on"
#define NVS_PUMP_KEY_T2_OFF      "t2_off"
#define NVS_PUMP_KEY_RELAY_LOW   "relay_low"
#define NVS_PUMP_KEY_AUTO_START  "auto_start"

static bool pump_duration_valid(uint32_t seconds)
{
    return seconds >= APP_TEMPLATE_PUMP_TIMER_MIN_SEC &&
           seconds <= APP_TEMPLATE_PUMP_TIMER_MAX_SEC;
}

static bool pump_bool_raw_valid(uint8_t value)
{
    return value == 0 || value == 1;
}

static bool pump_settings_valid(const nvs_store_pump_settings_t *settings)
{
    return settings &&
           pump_duration_valid(settings->timer1_on_sec) &&
           pump_duration_valid(settings->timer1_off_sec) &&
           pump_duration_valid(settings->timer2_on_sec) &&
           pump_duration_valid(settings->timer2_off_sec);
}

static esp_err_t load_u32_key(nvs_handle_t handle, const char *key, uint32_t *out)
{
    return nvs_get_u32(handle, key, out);
}

static esp_err_t load_bool_key(nvs_handle_t handle, const char *key, bool *out)
{
    uint8_t raw = 0;
    esp_err_t ret = nvs_get_u8(handle, key, &raw);
    if (ret != ESP_OK) {
        return ret;
    }
    if (!pump_bool_raw_valid(raw)) {
        return ESP_ERR_INVALID_STATE;
    }
    *out = raw != 0;
    return ESP_OK;
}

static bool erase_optional_key(nvs_handle_t handle, const char *key)
{
    esp_err_t ret = nvs_erase_key(handle, key);
    return ret == ESP_OK || ret == ESP_ERR_NVS_NOT_FOUND;
}

bool nvs_store_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    if (ret == ESP_OK) {
#if defined(CONFIG_NVS_ENCRYPTION) && CONFIG_NVS_ENCRYPTION
        ESP_LOGI(TAG, "NVS init OK (encryption enabled)");
#else
        ESP_LOGI(TAG, "NVS init OK (no encryption)");
#endif
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

bool nvs_store_save_ap_config(uint32_t stop_timeout_ms, bool auto_stop)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) return false;

    bool ok = true;
    if (nvs_set_u32(handle, NVS_KEY_AP_TMO, stop_timeout_ms) != ESP_OK) ok = false;
    uint8_t val = auto_stop ? 1 : 0;
    if (nvs_set_u8(handle, NVS_KEY_AP_AUTO, val) != ESP_OK) ok = false;

    if (ok) nvs_commit(handle);
    nvs_close(handle);
    return ok;
}

bool nvs_store_load_ap_config(uint32_t *stop_timeout_ms, bool *auto_stop)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) != ESP_OK) {
        if (stop_timeout_ms) *stop_timeout_ms = AP_STOP_TMO_DEFAULT_MS;
        if (auto_stop) *auto_stop = AP_AUTO_STOP_DEFAULT;
        return false;
    }

    uint32_t tmo = AP_STOP_TMO_DEFAULT_MS;
    if (nvs_get_u32(handle, NVS_KEY_AP_TMO, &tmo) != ESP_OK) {
        tmo = AP_STOP_TMO_DEFAULT_MS;
    }
    if (stop_timeout_ms) *stop_timeout_ms = tmo;

    uint8_t val = AP_AUTO_STOP_DEFAULT ? 1 : 0;
    if (nvs_get_u8(handle, NVS_KEY_AP_AUTO, &val) != ESP_OK) {
        val = AP_AUTO_STOP_DEFAULT ? 1 : 0;
    }
    if (auto_stop) *auto_stop = (val != 0);

    nvs_close(handle);
    return true;
}

bool nvs_store_save_sta_ip(const char *ip, const char *gateway, const char *netmask, const char *dns)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) return false;

    bool ok = true;
    if (ip && ip[0]) {
        if (nvs_set_str(handle, NVS_KEY_IP, ip) != ESP_OK) ok = false;
    } else {
        nvs_erase_key(handle, NVS_KEY_IP);
    }
    if (gateway && gateway[0]) {
        if (nvs_set_str(handle, NVS_KEY_GW, gateway) != ESP_OK) ok = false;
    } else {
        nvs_erase_key(handle, NVS_KEY_GW);
    }
    if (netmask && netmask[0]) {
        if (nvs_set_str(handle, NVS_KEY_MASK, netmask) != ESP_OK) ok = false;
    } else {
        nvs_erase_key(handle, NVS_KEY_MASK);
    }
    if (dns && dns[0]) {
        if (nvs_set_str(handle, NVS_KEY_DNS, dns) != ESP_OK) ok = false;
    } else {
        nvs_erase_key(handle, NVS_KEY_DNS);
    }

    if (ok) nvs_commit(handle);
    nvs_close(handle);
    return ok;
}

bool nvs_store_load_sta_ip(char *ip_out, size_t ip_len, char *gw_out, size_t gw_len,
                           char *mask_out, size_t mask_len, char *dns_out, size_t dns_len)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) != ESP_OK) {
        if (ip_out) ip_out[0] = '\0';
        if (gw_out) gw_out[0] = '\0';
        if (mask_out) mask_out[0] = '\0';
        if (dns_out) dns_out[0] = '\0';
        return false;
    }

    bool has_ip = false;
    size_t len = ip_len;
    if (nvs_get_str(handle, NVS_KEY_IP, ip_out, &len) == ESP_OK && ip_out[0]) {
        has_ip = true;
    } else if (ip_out) {
        ip_out[0] = '\0';
    }

    len = gw_len;
    if (nvs_get_str(handle, NVS_KEY_GW, gw_out, &len) != ESP_OK && gw_out) {
        gw_out[0] = '\0';
    }

    len = mask_len;
    if (nvs_get_str(handle, NVS_KEY_MASK, mask_out, &len) != ESP_OK && mask_out) {
        mask_out[0] = '\0';
    }

    len = dns_len;
    if (nvs_get_str(handle, NVS_KEY_DNS, dns_out, &len) != ESP_OK && dns_out) {
        dns_out[0] = '\0';
    }

    nvs_close(handle);
    return has_ip;
}

bool nvs_store_clear_sta_ip(void)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) return false;
    nvs_erase_key(handle, NVS_KEY_IP);
    nvs_erase_key(handle, NVS_KEY_GW);
    nvs_erase_key(handle, NVS_KEY_MASK);
    nvs_erase_key(handle, NVS_KEY_DNS);
    nvs_commit(handle);
    nvs_close(handle);
    return true;
}

void nvs_store_pump_settings_defaults(nvs_store_pump_settings_t *out)
{
    if (!out) {
        return;
    }

    out->timer1_on_sec = APP_TEMPLATE_PUMP_TIMER1_ON_SEC;
    out->timer1_off_sec = APP_TEMPLATE_PUMP_TIMER1_OFF_SEC;
    out->timer2_on_sec = APP_TEMPLATE_PUMP_TIMER2_ON_SEC;
    out->timer2_off_sec = APP_TEMPLATE_PUMP_TIMER2_OFF_SEC;
    out->relay_active_low = APP_TEMPLATE_PUMP_RELAY_ACTIVE_LOW;
    out->auto_start = APP_TEMPLATE_PUMP_AUTO_START_DEFAULT;
}

nvs_store_pump_settings_load_status_t nvs_store_load_pump_settings(nvs_store_pump_settings_t *out)
{
    if (!out) {
        return NVS_STORE_PUMP_SETTINGS_DEFAULTS_ERROR;
    }

    nvs_store_pump_settings_defaults(out);

    nvs_handle_t handle;
    esp_err_t open_ret = nvs_open(NVS_PUMP_NAMESPACE, NVS_READONLY, &handle);
    if (open_ret == ESP_ERR_NVS_NOT_FOUND) {
        return NVS_STORE_PUMP_SETTINGS_DEFAULTS_MISSING;
    }
    if (open_ret != ESP_OK) {
        ESP_LOGE(TAG, "Open %s failed: %s", NVS_PUMP_NAMESPACE, esp_err_to_name(open_ret));
        return NVS_STORE_PUMP_SETTINGS_DEFAULTS_ERROR;
    }

    nvs_store_pump_settings_t loaded = {0};
    bool relay_active_low = false;
    bool auto_start = false;

    esp_err_t results[] = {
        load_u32_key(handle, NVS_PUMP_KEY_T1_ON, &loaded.timer1_on_sec),
        load_u32_key(handle, NVS_PUMP_KEY_T1_OFF, &loaded.timer1_off_sec),
        load_u32_key(handle, NVS_PUMP_KEY_T2_ON, &loaded.timer2_on_sec),
        load_u32_key(handle, NVS_PUMP_KEY_T2_OFF, &loaded.timer2_off_sec),
        load_bool_key(handle, NVS_PUMP_KEY_RELAY_LOW, &relay_active_low),
        load_bool_key(handle, NVS_PUMP_KEY_AUTO_START, &auto_start),
    };
    nvs_close(handle);

    for (size_t i = 0; i < sizeof(results) / sizeof(results[0]); i++) {
        if (results[i] == ESP_ERR_NVS_NOT_FOUND) {
            return NVS_STORE_PUMP_SETTINGS_DEFAULTS_INVALID;
        }
        if (results[i] != ESP_OK) {
            ESP_LOGW(TAG, "Invalid pump settings in NVS (%s)", esp_err_to_name(results[i]));
            return NVS_STORE_PUMP_SETTINGS_DEFAULTS_INVALID;
        }
    }

    loaded.relay_active_low = relay_active_low;
    loaded.auto_start = auto_start;
    if (!pump_settings_valid(&loaded)) {
        return NVS_STORE_PUMP_SETTINGS_DEFAULTS_INVALID;
    }

    *out = loaded;
    return NVS_STORE_PUMP_SETTINGS_LOADED;
}

bool nvs_store_save_pump_settings(const nvs_store_pump_settings_t *settings)
{
    if (!pump_settings_valid(settings)) {
        return false;
    }

    nvs_handle_t handle;
    if (nvs_open(NVS_PUMP_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) return false;

    bool ok = true;
    if (nvs_set_u32(handle, NVS_PUMP_KEY_T1_ON, settings->timer1_on_sec) != ESP_OK) ok = false;
    if (nvs_set_u32(handle, NVS_PUMP_KEY_T1_OFF, settings->timer1_off_sec) != ESP_OK) ok = false;
    if (nvs_set_u32(handle, NVS_PUMP_KEY_T2_ON, settings->timer2_on_sec) != ESP_OK) ok = false;
    if (nvs_set_u32(handle, NVS_PUMP_KEY_T2_OFF, settings->timer2_off_sec) != ESP_OK) ok = false;
    if (nvs_set_u8(handle, NVS_PUMP_KEY_RELAY_LOW, settings->relay_active_low ? 1 : 0) != ESP_OK) ok = false;
    if (nvs_set_u8(handle, NVS_PUMP_KEY_AUTO_START, settings->auto_start ? 1 : 0) != ESP_OK) ok = false;

    if (ok && nvs_commit(handle) != ESP_OK) ok = false;
    nvs_close(handle);
    return ok;
}

bool nvs_store_clear_pump_settings(void)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_PUMP_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) return false;

    bool ok = true;
    if (!erase_optional_key(handle, NVS_PUMP_KEY_T1_ON)) ok = false;
    if (!erase_optional_key(handle, NVS_PUMP_KEY_T1_OFF)) ok = false;
    if (!erase_optional_key(handle, NVS_PUMP_KEY_T2_ON)) ok = false;
    if (!erase_optional_key(handle, NVS_PUMP_KEY_T2_OFF)) ok = false;
    if (!erase_optional_key(handle, NVS_PUMP_KEY_RELAY_LOW)) ok = false;
    if (!erase_optional_key(handle, NVS_PUMP_KEY_AUTO_START)) ok = false;

    if (ok && nvs_commit(handle) != ESP_OK) ok = false;
    nvs_close(handle);
    return ok;
}
