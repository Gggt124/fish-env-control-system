#include "wifi_manager.h"
#include "nvs_store.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "wifi_mgr";

static bool s_ap_enabled = false;
static bool s_sta_connected = false;
static bool s_sta_configured = false;
static bool s_scan_in_progress = false;
static char s_sta_ssid[33] = {0};
static char s_ap_ip[16] = "192.168.4.1";
static char s_sta_ip[16] = {0};
static int64_t s_boot_time_ms = 0;

static esp_netif_t *s_ap_netif = NULL;
static esp_netif_t *s_sta_netif = NULL;

static wifi_scan_cb_t s_scan_callback = NULL;
static void *s_scan_user_ctx = NULL;
static wifi_scan_entry_t s_scan_results[WIFI_SCAN_MAX];
static int s_scan_count = 0;

/* --------------- Helpers --------------- */

const char* wifi_auth_mode_to_string(wifi_auth_mode_t mode)
{
    switch (mode) {
    case WIFI_AUTH_OPEN:          return "Open";
    case WIFI_AUTH_WEP:           return "WEP";
    case WIFI_AUTH_WPA_PSK:       return "WPA";
    case WIFI_AUTH_WPA2_PSK:      return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK:  return "WPA/WPA2";
    case WIFI_AUTH_WPA3_PSK:      return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/WPA3";
    default:                       return "Unknown";
    }
}

/* --------------- Event Handler --------------- */

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
        case WIFI_EVENT_STA_START:
            if (s_sta_configured) {
                esp_wifi_connect();
            }
            break;
        case WIFI_EVENT_STA_DISCONNECTED: {
            wifi_event_sta_disconnected_t *ev = (wifi_event_sta_disconnected_t *)event_data;
            ESP_LOGW(TAG, "STA disconnected, reason=%d", ev->reason);
            s_sta_connected = false;
            s_sta_ip[0] = '\0';
            s_sta_ssid[0] = '\0';
            /* Reconnect attempt - skip if scan is in progress */
            if (s_sta_configured && !s_scan_in_progress) {
                esp_wifi_connect();
            }
            break;
        }
        case WIFI_EVENT_AP_STACONNECTED:
            ESP_LOGI(TAG, "Client connected to AP");
            break;
        case WIFI_EVENT_AP_STADISCONNECTED:
            ESP_LOGI(TAG, "Client disconnected from AP");
            break;
        case WIFI_EVENT_SCAN_DONE: {
            uint16_t num = 0;
            esp_wifi_scan_get_ap_num(&num);
            if (num > WIFI_SCAN_MAX) num = WIFI_SCAN_MAX;

            wifi_ap_record_t *ap_info = calloc(num, sizeof(wifi_ap_record_t));
            if (ap_info) {
                esp_wifi_scan_get_ap_records(&num, ap_info);
                for (int i = 0; i < num; i++) {
                    strncpy(s_scan_results[i].ssid, (char *)ap_info[i].ssid, 32);
                    s_scan_results[i].ssid[32] = '\0';
                    s_scan_results[i].rssi = ap_info[i].rssi;
                    strncpy(s_scan_results[i].auth,
                            wifi_auth_mode_to_string(ap_info[i].authmode),
                            sizeof(s_scan_results[i].auth) - 1);
                    s_scan_results[i].channel = ap_info[i].primary;
                }
                s_scan_count = num;
                free(ap_info);
            }
            if (s_scan_callback) {
                s_scan_callback(s_scan_user_ctx, s_scan_results, s_scan_count);
                s_scan_callback = NULL;
                s_scan_user_ctx = NULL;
            }
            break;
        }
        default:
            break;
        }
    } else if (event_base == IP_EVENT) {
        switch (event_id) {
        case IP_EVENT_STA_GOT_IP: {
            ip_event_got_ip_t *ev = (ip_event_got_ip_t *)event_data;
            snprintf(s_sta_ip, sizeof(s_sta_ip), IPSTR, IP2STR(&ev->ip_info.ip));
            s_sta_connected = true;
            ESP_LOGI(TAG, "STA got IP: %s", s_sta_ip);

            /* Save current SSID to NVS on successful connect */
            wifi_ap_record_t ap_info;
            if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
                strncpy(s_sta_ssid, (char *)ap_info.ssid, 32);
                s_sta_ssid[32] = '\0';
            }
            break;
        }
        case IP_EVENT_ASSIGNED_IP_TO_CLIENT:
            ESP_LOGI(TAG, "AP assigned IP to client");
            break;
        default:
            break;
        }
    }
}

/* --------------- Init --------------- */

bool wifi_manager_init(void)
{
    s_boot_time_ms = esp_timer_get_time() / 1000;

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    s_ap_netif  = esp_netif_create_default_wifi_ap();
    s_sta_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                        &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID,
                        &wifi_event_handler, NULL, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

    /* Load saved STA credentials and attempt connect */
    char saved_ssid[33] = {0};
    char saved_pass[65] = {0};
    if (nvs_store_load_wifi(saved_ssid, sizeof(saved_ssid),
                            saved_pass, sizeof(saved_pass))) {
        ESP_LOGI(TAG, "Found saved STA credentials for SSID: %s", saved_ssid);
        wifi_config_t sta_cfg = {0};
        strncpy((char *)sta_cfg.sta.ssid, saved_ssid, 32);
        if (saved_pass[0]) {
            strncpy((char *)sta_cfg.sta.password, saved_pass, 64);
        }
        sta_cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_cfg));
        s_sta_configured = true;
    }

    ESP_ERROR_CHECK(esp_wifi_start());

    /* Start AP immediately */
    wifi_manager_start_ap();

    return true;
}

/* --------------- AP --------------- */

bool wifi_manager_start_ap(void)
{
    wifi_config_t ap_cfg = {
        .ap = {
            .ssid = AP_SSID,
            .password = "",
            .ssid_len = strlen(AP_SSID),
            .channel = 1,
            .authmode = WIFI_AUTH_OPEN,
            .max_connection = AP_MAX_CONN,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_cfg));
    s_ap_enabled = true;
    ESP_LOGI(TAG, "SoftAP started: SSID=%s, IP=%s", AP_SSID, s_ap_ip);
    return true;
}

/* --------------- STA --------------- */

bool wifi_manager_connect_sta(const char *ssid, const char *password)
{
    if (!ssid || !ssid[0]) return false;

    nvs_store_save_wifi(ssid, password);

    esp_wifi_disconnect();

    wifi_config_t sta_cfg = {0};
    strncpy((char *)sta_cfg.sta.ssid, ssid, 32);
    if (password && password[0]) {
        strncpy((char *)sta_cfg.sta.password, password, 64);
    }
    sta_cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    if (esp_wifi_set_config(WIFI_IF_STA, &sta_cfg) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set STA config");
        return false;
    }
    if (esp_wifi_connect() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start STA connect");
        return false;
    }

    ESP_LOGI(TAG, "Connecting to STA SSID: %s", ssid);
    return true;
}

bool wifi_manager_disconnect_sta(void)
{
    esp_err_t ret = esp_wifi_disconnect();
    s_sta_connected = false;
    s_sta_configured = false;
    s_sta_ip[0] = '\0';
    s_sta_ssid[0] = '\0';
    return ret == ESP_OK;
}

bool wifi_manager_forget_sta(void)
{
    esp_wifi_disconnect();
    s_sta_connected = false;
    s_sta_configured = false;
    s_sta_ip[0] = '\0';
    s_sta_ssid[0] = '\0';
    nvs_store_clear_wifi();
    return true;
}

/* --------------- Scan --------------- */

bool wifi_manager_scan(void *user_ctx, wifi_scan_cb_t callback)
{
    if (!callback) return false;

    s_scan_callback = callback;
    s_scan_user_ctx = user_ctx;
    s_scan_count = 0;

    wifi_scan_config_t scan_cfg = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time.active.min = 100,
        .scan_time.active.max = 300,
    };

    esp_err_t ret = esp_wifi_scan_start(&scan_cfg, false);
    if (ret != ESP_OK) {
        s_scan_callback = NULL;
        s_scan_user_ctx = NULL;
        return false;
    }
    return true;
}

void wifi_manager_sta_disconnect_for_scan(void)
{
    s_scan_in_progress = true;
    esp_wifi_disconnect();
    vTaskDelay(pdMS_TO_TICKS(200));
}

void wifi_manager_sta_reconnect_after_scan(void)
{
    s_scan_in_progress = false;
    if (s_sta_configured) {
        esp_wifi_connect();
    }
}

/* --------------- State Getters --------------- */

bool wifi_manager_is_ap_enabled(void)     { return s_ap_enabled; }
bool wifi_manager_is_sta_connected(void)  { return s_sta_connected; }
char* wifi_manager_get_ap_ip(void)        { return s_ap_ip; }
char* wifi_manager_get_sta_ip(void)       { return s_sta_ip; }
char* wifi_manager_get_sta_ssid(void)     { return s_sta_ssid; }

int64_t wifi_manager_get_uptime_ms(void)
{
    return (esp_timer_get_time() / 1000) - s_boot_time_ms;
}

uint32_t wifi_manager_get_free_heap(void)
{
    return esp_get_free_heap_size();
}
