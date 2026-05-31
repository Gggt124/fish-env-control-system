#include "wifi_manager.h"
#include "nvs_store.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "lwip/ip4_addr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "wifi_mgr";

#define STA_MAX_RETRY 1
#define STA_CONNECT_DELAY_MS 400

static bool s_ap_enabled = false;
static bool s_sta_connected = false;
static bool s_sta_configured = false;
static bool s_sta_connecting = false;
static bool s_sta_manual_connect_pending = false;
static bool s_scan_in_progress = false;
static char s_sta_ssid[33] = {0};
static char s_ap_ip[16] = "192.168.4.1";
static char s_sta_ip[16] = {0};
static int64_t s_boot_time_ms = 0;
static int s_sta_retry_count = 0;

static uint32_t s_ap_stop_timeout_ms = AP_STOP_TMO_DEFAULT_MS;
static bool s_ap_auto_stop = AP_AUTO_STOP_DEFAULT;

static esp_netif_t *s_ap_netif = NULL;
static esp_netif_t *s_sta_netif = NULL;

static wifi_scan_cb_t s_scan_callback = NULL;
static void *s_scan_user_ctx = NULL;
static wifi_scan_entry_t s_scan_results[WIFI_SCAN_MAX];
static int s_scan_count = 0;

static esp_timer_handle_t s_ap_stop_timer = NULL;
static esp_timer_handle_t s_sta_connect_timer = NULL;

static SemaphoreHandle_t s_wifi_mutex;

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

static void restore_sta_dhcp(void)
{
    if (s_sta_netif) {
        esp_err_t err = esp_netif_dhcpc_start(s_sta_netif);
        if (err != ESP_OK) {
            ESP_LOGD(TAG, "DHCP client start returned %d", err);
        }
    }
}

static bool apply_static_ip(const wifi_sta_ip_config_t *cfg)
{
    if (!cfg || !cfg->ip[0]) return false;

    if (!cfg->gateway[0]) {
        ESP_LOGW(TAG, "Static IP ignored: gateway is required");
        return false;
    }

    if (!s_sta_netif) {
        ESP_LOGE(TAG, "STA netif not available for static IP");
        return false;
    }

    ip4_addr_t ip;
    ip4_addr_t gw;
    ip4_addr_t mask;
    ip4_addr_t dns_addr;
    const char *netmask = cfg->netmask[0] ? cfg->netmask : "255.255.255.0";
    const char *dns = cfg->dns[0] ? cfg->dns : cfg->gateway;

    if (!ip4addr_aton(cfg->ip, &ip) ||
        !ip4addr_aton(cfg->gateway, &gw) ||
        !ip4addr_aton(netmask, &mask) ||
        !ip4addr_aton(dns, &dns_addr)) {
        ESP_LOGW(TAG, "Static IP ignored: invalid IP/gateway/netmask/DNS value");
        return false;
    }

    esp_netif_ip_info_t ip_info;
    memset(&ip_info, 0, sizeof(ip_info));
    ip_info.ip.addr = ip.addr;
    ip_info.gw.addr = gw.addr;
    ip_info.netmask.addr = mask.addr;

    esp_netif_dhcpc_stop(s_sta_netif);
    esp_err_t err = esp_netif_set_ip_info(s_sta_netif, &ip_info);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set static IP info: %d", err);
        restore_sta_dhcp();
        return false;
    }

    esp_netif_dns_info_t dns_info;
    memset(&dns_info, 0, sizeof(dns_info));
    dns_info.ip.u_addr.ip4.addr = dns_addr.addr;
    dns_info.ip.type = ESP_IPADDR_TYPE_V4;
    esp_netif_set_dns_info(s_sta_netif, ESP_NETIF_DNS_MAIN, &dns_info);

    ESP_LOGI(TAG, "Static IP: %s / %s gw=%s dns=%s", cfg->ip, netmask, cfg->gateway, dns);
    return true;
}

static void schedule_sta_connect(void);

static void sta_connect_timer_cb(void *arg)
{
    xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
    bool should_connect = s_sta_configured &&
                          !s_sta_connected &&
                          !s_sta_connecting &&
                          !s_scan_in_progress;
    if (should_connect) {
        s_sta_connecting = true;
    }
    xSemaphoreGive(s_wifi_mutex);

    if (!should_connect) {
        return;
    }

    esp_err_t err = esp_wifi_connect();
    if (err == ESP_OK) {
        return;
    }

    xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
    s_sta_connecting = false;
    bool should_recover = s_sta_configured && !s_scan_in_progress;
    xSemaphoreGive(s_wifi_mutex);

    ESP_LOGW(TAG, "Deferred STA connect start failed: %s", esp_err_to_name(err));
    if (should_recover && err == ESP_ERR_WIFI_CONN) {
        ESP_LOGI(TAG, "Resetting busy STA state before retry");
        esp_wifi_disconnect();
        schedule_sta_connect();
    }
}

static void schedule_sta_connect(void)
{
    if (!s_sta_connect_timer) {
        ESP_LOGW(TAG, "STA connect timer unavailable");
        return;
    }

    esp_timer_stop(s_sta_connect_timer);
    esp_err_t err = esp_timer_start_once(s_sta_connect_timer, STA_CONNECT_DELAY_MS * 1000);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to schedule STA connect: %s", esp_err_to_name(err));
    }
}

static void configure_radio_stability(void)
{
    esp_err_t sta_bw_err = esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW20);
    if (sta_bw_err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to set STA bandwidth HT20: %s", esp_err_to_name(sta_bw_err));
    }

    esp_err_t ap_bw_err = esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW20);
    if (ap_bw_err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to set AP bandwidth HT20: %s", esp_err_to_name(ap_bw_err));
    }

    esp_err_t power_err = esp_wifi_set_max_tx_power(APP_TEMPLATE_WIFI_MAX_TX_POWER_QDBM);
    if (power_err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to set max TX power: %s", esp_err_to_name(power_err));
        return;
    }

    int8_t tx_power = 0;
    if (esp_wifi_get_max_tx_power(&tx_power) == ESP_OK) {
        ESP_LOGI(TAG, "Radio stability: APSTA HT20, max TX power=%d qdBm", tx_power);
    }
}

/* --------------- AP Auto-Stop Timer --------------- */

static void ap_stop_timer_cb(void *arg)
{
    ESP_LOGI(TAG, "AP auto-stop timer expired, stopping AP");
    xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
    esp_wifi_set_mode(WIFI_MODE_STA);
    s_ap_enabled = false;
    xSemaphoreGive(s_wifi_mutex);
}

/* --------------- Event Handler --------------- */

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
        case WIFI_EVENT_STA_START:
            xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
            bool sta_configured = s_sta_configured;
            xSemaphoreGive(s_wifi_mutex);
            if (sta_configured) {
                schedule_sta_connect();
            }
            break;
        case WIFI_EVENT_STA_DISCONNECTED: {
            wifi_event_sta_disconnected_t *ev = (wifi_event_sta_disconnected_t *)event_data;
            ESP_LOGW(TAG, "STA disconnected, reason=%d", ev->reason);
            xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
            s_sta_connected = false;
            s_sta_connecting = false;
            s_sta_ip[0] = '\0';
            s_sta_ssid[0] = '\0';

            if (s_ap_stop_timer) {
                esp_timer_stop(s_ap_stop_timer);
            }

            bool manual_connect_pending = s_sta_manual_connect_pending;
            if (manual_connect_pending) {
                s_sta_manual_connect_pending = false;
                s_sta_retry_count = 0;
            } else if (s_sta_configured && !s_scan_in_progress) {
                s_sta_retry_count++;
            }
            bool retry_limit_reached = s_sta_retry_count > STA_MAX_RETRY;
            bool sta_configured = s_sta_configured;
            bool scan_in_progress = s_scan_in_progress;
            bool ap_enabled = s_ap_enabled;
            xSemaphoreGive(s_wifi_mutex);

            if (manual_connect_pending && sta_configured && !scan_in_progress) {
                ESP_LOGI(TAG, "Scheduling requested STA connect after disconnect");
                schedule_sta_connect();
            } else if (retry_limit_reached) {
                ESP_LOGI(TAG, "STA retry limit (%d) reached, restoring AP as fallback", STA_MAX_RETRY);
                if (!ap_enabled) {
                    wifi_manager_start_ap();
                }
            } else if (sta_configured && !scan_in_progress) {
                schedule_sta_connect();
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
                xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
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
                wifi_scan_cb_t cb = s_scan_callback;
                void *ctx = s_scan_user_ctx;
                s_scan_callback = NULL;
                s_scan_user_ctx = NULL;
                xSemaphoreGive(s_wifi_mutex);
                free(ap_info);
                if (cb) {
                    cb(ctx, s_scan_results, s_scan_count);
                }
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
            xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
            snprintf(s_sta_ip, sizeof(s_sta_ip), IPSTR, IP2STR(&ev->ip_info.ip));
            s_sta_connected = true;
            s_sta_connecting = false;
            s_sta_manual_connect_pending = false;
            s_sta_retry_count = 0;
            ESP_LOGI(TAG, "STA got IP: %s", s_sta_ip);

            wifi_ap_record_t ap_info;
            if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
                strncpy(s_sta_ssid, (char *)ap_info.ssid, 32);
                s_sta_ssid[32] = '\0';
            }

            bool ap_enabled = s_ap_enabled;
            bool ap_auto_stop = s_ap_auto_stop;
            uint32_t ap_stop_timeout_ms = s_ap_stop_timeout_ms;
            xSemaphoreGive(s_wifi_mutex);

            if (ap_enabled && ap_auto_stop && s_ap_stop_timer) {
                ESP_LOGI(TAG, "Starting AP auto-stop timer (%lu ms)", (unsigned long)ap_stop_timeout_ms);
                esp_timer_start_once(s_ap_stop_timer, ap_stop_timeout_ms * 1000);
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

    s_wifi_mutex = xSemaphoreCreateMutex();
    if (!s_wifi_mutex) {
        ESP_LOGE(TAG, "Failed to create Wi-Fi mutex");
        return false;
    }

    esp_timer_create_args_t sta_connect_timer_args = {
        .callback = &sta_connect_timer_cb,
        .arg = NULL,
        .name = "sta_connect_timer"
    };
    if (esp_timer_create(&sta_connect_timer_args, &s_sta_connect_timer) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create STA connect timer");
        return false;
    }

    /* Load AP config from NVS */
    nvs_store_load_ap_config(&s_ap_stop_timeout_ms, &s_ap_auto_stop);
    ESP_LOGI(TAG, "AP config: auto_stop=%s, timeout=%lu ms",
             s_ap_auto_stop ? "ON" : "OFF", (unsigned long)s_ap_stop_timeout_ms);

    /* Load saved STA credentials and attempt connect */
    char saved_ssid[33] = {0};
    char saved_pass[65] = {0};
    if (nvs_store_load_wifi(saved_ssid, sizeof(saved_ssid),
                            saved_pass, sizeof(saved_pass))) {
        ESP_LOGI(TAG, "Found saved STA credentials for SSID: %s", saved_ssid);

        /* Load optional static IP config and apply before connecting */
        wifi_sta_ip_config_t saved_ip = {0};
        if (nvs_store_load_sta_ip(saved_ip.ip, sizeof(saved_ip.ip),
                                  saved_ip.gateway, sizeof(saved_ip.gateway),
                                  saved_ip.netmask, sizeof(saved_ip.netmask),
                                  saved_ip.dns, sizeof(saved_ip.dns))) {
            if (!apply_static_ip(&saved_ip)) {
                ESP_LOGW(TAG, "Clearing invalid saved static IP config; using DHCP");
                nvs_store_clear_sta_ip();
                restore_sta_dhcp();
            }
        }

        wifi_config_t sta_cfg = {0};
        strncpy((char *)sta_cfg.sta.ssid, saved_ssid, 32);
        if (saved_pass[0]) {
            strncpy((char *)sta_cfg.sta.password, saved_pass, 64);
        }
        sta_cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        if (esp_wifi_set_config(WIFI_IF_STA, &sta_cfg) != ESP_OK) {
            ESP_LOGW(TAG, "Failed to set saved STA config, will retry on connect");
        }
        s_sta_configured = true;
    }

    ESP_ERROR_CHECK(esp_wifi_start());
    configure_radio_stability();

    /* Create AP auto-stop timer */
    esp_timer_create_args_t timer_args = {
        .callback = &ap_stop_timer_cb,
        .arg = NULL,
        .name = "ap_stop_timer"
    };
    if (esp_timer_create(&timer_args, &s_ap_stop_timer) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to create AP auto-stop timer");
        s_ap_stop_timer = NULL;
    }

    /* Start AP immediately */
    wifi_manager_start_ap();

    return true;
}

/* --------------- AP --------------- */

bool wifi_manager_start_ap(void)
{
    xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
    esp_wifi_set_mode(WIFI_MODE_APSTA);

    wifi_config_t ap_cfg = {
        .ap = {
            .ssid = AP_SSID,
            .password = "",
            .ssid_len = strlen(AP_SSID),
            .channel = APP_TEMPLATE_AP_CHANNEL,
            .authmode = WIFI_AUTH_OPEN,
            .max_connection = AP_MAX_CONN,
        },
    };

    if (esp_wifi_set_config(WIFI_IF_AP, &ap_cfg) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set AP config");
        xSemaphoreGive(s_wifi_mutex);
        return false;
    }
    s_ap_enabled = true;
    ESP_LOGI(TAG, "SoftAP started: SSID=%s, IP=%s", AP_SSID, s_ap_ip);
    xSemaphoreGive(s_wifi_mutex);
    return true;
}

bool wifi_manager_stop_ap(void)
{
    xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
    if (s_ap_stop_timer) {
        esp_timer_stop(s_ap_stop_timer);
    }
    esp_err_t err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set STA mode: %d", err);
    }
    s_ap_enabled = false;
    ESP_LOGI(TAG, "SoftAP stopped");
    xSemaphoreGive(s_wifi_mutex);
    return true;
}

/* --------------- STA --------------- */

bool wifi_manager_connect_sta(const char *ssid, const char *password, const wifi_sta_ip_config_t *ip_config)
{
    if (!ssid || !ssid[0]) return false;
    if (ip_config && ip_config->ip[0] && !ip_config->gateway[0]) {
        ESP_LOGW(TAG, "Static IP connect rejected: gateway is required");
        return false;
    }

    nvs_store_save_wifi(ssid, password);

    /* Save and apply static IP config if provided, otherwise clear it */
    if (ip_config && ip_config->ip[0]) {
        if (!apply_static_ip(ip_config)) {
            return false;
        }
        nvs_store_save_sta_ip(ip_config->ip, ip_config->gateway,
                               ip_config->netmask, ip_config->dns);
    } else {
        nvs_store_clear_sta_ip();
        restore_sta_dhcp();
    }

    xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
    s_sta_retry_count = 0;
    s_sta_connected = false;
    s_sta_configured = false;
    s_sta_connecting = false;
    s_sta_manual_connect_pending = true;
    s_sta_ip[0] = '\0';
    s_sta_ssid[0] = '\0';
    xSemaphoreGive(s_wifi_mutex);

    esp_err_t disconnect_err = esp_wifi_disconnect();
    if (disconnect_err != ESP_OK && disconnect_err != ESP_ERR_WIFI_NOT_CONNECT) {
        ESP_LOGW(TAG, "STA disconnect before connect returned: %s", esp_err_to_name(disconnect_err));
    }

    wifi_config_t sta_cfg = {0};
    strncpy((char *)sta_cfg.sta.ssid, ssid, 32);
    if (password && password[0]) {
        strncpy((char *)sta_cfg.sta.password, password, 64);
    }
    sta_cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    if (esp_wifi_set_config(WIFI_IF_STA, &sta_cfg) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set STA config");
        xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
        s_sta_manual_connect_pending = false;
        xSemaphoreGive(s_wifi_mutex);
        return false;
    }

    xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
    s_sta_configured = true;
    xSemaphoreGive(s_wifi_mutex);

    schedule_sta_connect();
    ESP_LOGI(TAG, "Queued STA connect for SSID: %s", ssid);
    return true;
}

bool wifi_manager_disconnect_sta(void)
{
    xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
    if (s_ap_stop_timer) {
        esp_timer_stop(s_ap_stop_timer);
    }
    if (s_sta_connect_timer) {
        esp_timer_stop(s_sta_connect_timer);
    }
    s_sta_connected = false;
    s_sta_configured = false;
    s_sta_connecting = false;
    s_sta_manual_connect_pending = false;
    s_sta_ip[0] = '\0';
    s_sta_ssid[0] = '\0';
    s_sta_retry_count = 0;
    bool ap_enabled = s_ap_enabled;
    xSemaphoreGive(s_wifi_mutex);

    esp_err_t ret = esp_wifi_disconnect();
    if (!ap_enabled) {
        ESP_LOGI(TAG, "Restoring AP after STA disconnect");
        wifi_manager_start_ap();
    }

    return ret == ESP_OK;
}

bool wifi_manager_forget_sta(void)
{
    xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
    if (s_ap_stop_timer) {
        esp_timer_stop(s_ap_stop_timer);
    }
    if (s_sta_connect_timer) {
        esp_timer_stop(s_sta_connect_timer);
    }
    s_sta_connected = false;
    s_sta_configured = false;
    s_sta_connecting = false;
    s_sta_manual_connect_pending = false;
    s_sta_ip[0] = '\0';
    s_sta_ssid[0] = '\0';
    s_sta_retry_count = 0;
    bool ap_enabled = s_ap_enabled;
    xSemaphoreGive(s_wifi_mutex);

    esp_wifi_disconnect();
    nvs_store_clear_wifi();
    nvs_store_clear_sta_ip();
    restore_sta_dhcp();
    if (!ap_enabled) {
        wifi_manager_start_ap();
    }
    return true;
}

/* --------------- Scan --------------- */

bool wifi_manager_scan(void *user_ctx, wifi_scan_cb_t callback)
{
    if (!callback) return false;

    xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
    s_scan_callback = callback;
    s_scan_user_ctx = user_ctx;
    s_scan_count = 0;
    xSemaphoreGive(s_wifi_mutex);

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
        xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
        s_scan_callback = NULL;
        s_scan_user_ctx = NULL;
        xSemaphoreGive(s_wifi_mutex);
        return false;
    }
    return true;
}

void wifi_manager_sta_disconnect_for_scan(void)
{
    xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
    s_scan_in_progress = true;
    s_sta_connecting = false;
    if (s_sta_connect_timer) {
        esp_timer_stop(s_sta_connect_timer);
    }
    xSemaphoreGive(s_wifi_mutex);
    esp_wifi_disconnect();
    vTaskDelay(pdMS_TO_TICKS(200));
}

void wifi_manager_sta_reconnect_after_scan(void)
{
    xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
    s_scan_in_progress = false;
    s_sta_retry_count = 0;
    bool ap_enabled = s_ap_enabled;
    bool ap_auto_stop = s_ap_auto_stop;
    bool sta_configured = s_sta_configured;
    xSemaphoreGive(s_wifi_mutex);

    if (ap_enabled && ap_auto_stop && s_ap_stop_timer) {
        esp_timer_stop(s_ap_stop_timer);
    }
    if (sta_configured) {
        schedule_sta_connect();
    }
}

/* --------------- State Getters --------------- */

bool wifi_manager_is_ap_enabled(void)
{
    xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
    bool v = s_ap_enabled;
    xSemaphoreGive(s_wifi_mutex);
    return v;
}

bool wifi_manager_is_sta_connected(void)
{
    xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
    bool v = s_sta_connected;
    xSemaphoreGive(s_wifi_mutex);
    return v;
}

char* wifi_manager_get_ap_ip(void)
{
    return s_ap_ip;
}

char* wifi_manager_get_sta_ip(void)
{
    xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
    xSemaphoreGive(s_wifi_mutex);
    return s_sta_ip;
}

char* wifi_manager_get_sta_ssid(void)
{
    xSemaphoreTake(s_wifi_mutex, portMAX_DELAY);
    xSemaphoreGive(s_wifi_mutex);
    return s_sta_ssid;
}

int64_t wifi_manager_get_uptime_ms(void)
{
    return (esp_timer_get_time() / 1000) - s_boot_time_ms;
}

uint32_t wifi_manager_get_free_heap(void)
{
    return esp_get_free_heap_size();
}
