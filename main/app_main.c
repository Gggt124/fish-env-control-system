#include "app_config.h"
#include "nvs_store.h"
#include "pump_control.h"
#include "wifi_manager.h"
#include "session.h"
#include "web_server.h"
#include "dns_server.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_err.h"
#include "mdns.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"

static const char *TAG = "app_main";
static bool s_http_server_retry = false;

static const char *pump_settings_load_status_name(nvs_store_pump_settings_load_status_t status)
{
    switch (status) {
    case NVS_STORE_PUMP_SETTINGS_LOADED:
        return "saved";
    case NVS_STORE_PUMP_SETTINGS_DEFAULTS_MISSING:
        return "defaults-missing";
    case NVS_STORE_PUMP_SETTINGS_DEFAULTS_INVALID:
        return "defaults-invalid";
    case NVS_STORE_PUMP_SETTINGS_DEFAULTS_ERROR:
    default:
        return "defaults-error";
    }
}

static void apply_pump_settings_to_config(pump_control_config_t *config,
                                          const nvs_store_pump_settings_t *settings)
{
    if (!config || !settings) {
        return;
    }

    config->timer1.on_sec = settings->timer1_on_sec;
    config->timer1.off_sec = settings->timer1_off_sec;
    config->timer2.on_sec = settings->timer2_on_sec;
    config->timer2.off_sec = settings->timer2_off_sec;
    config->relay_polarity = settings->relay_active_low
        ? PUMP_CONTROL_RELAY_ACTIVE_LOW
        : PUMP_CONTROL_RELAY_ACTIVE_HIGH;
}

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  %s %s", APP_TEMPLATE_NAME, APP_TEMPLATE_FIRMWARE_VERSION);
    ESP_LOGI(TAG, "  %s", APP_TEMPLATE_PHASE_LABEL);
    ESP_LOGI(TAG, "========================================");

    /* 1. Initialize NVS storage */
    if (!nvs_store_init()) {
        ESP_LOGE(TAG, "NVS init failed");
        return;
    }
    ESP_LOGI(TAG, "NVS initialized");

    nvs_store_pump_settings_t pump_settings;
    nvs_store_pump_settings_load_status_t pump_settings_status =
        nvs_store_load_pump_settings(&pump_settings);
    pump_control_config_t pump_config = pump_control_default_config();
    apply_pump_settings_to_config(&pump_config, &pump_settings);

    bool allow_auto_start = pump_settings.auto_start;
    if (pump_settings_status == NVS_STORE_PUMP_SETTINGS_LOADED) {
        ESP_LOGI(TAG, "Pump settings loaded from NVS");
    } else if (pump_settings_status == NVS_STORE_PUMP_SETTINGS_DEFAULTS_MISSING) {
        ESP_LOGI(TAG, "Pump settings missing; using product defaults");
    } else {
        allow_auto_start = false;
        ESP_LOGW(TAG, "Pump settings %s; using defaults and suppressing auto-start for this boot",
                 pump_settings_load_status_name(pump_settings_status));
    }

    if (pump_control_init(&pump_config)) {
        ESP_LOGI(TAG, "Pump control initialized: relay GPIO=%d, float GPIO=%d, auto_start=%s",
                 pump_config.relay_gpio, pump_config.float_gpio,
                 allow_auto_start ? "enabled" : "disabled");
        if (allow_auto_start) {
            if (pump_control_start()) {
                ESP_LOGI(TAG, "Pump control auto-started");
            } else {
                pump_control_stop();
                ESP_LOGE(TAG, "Pump control auto-start failed; continuing with Wi-Fi/web setup");
            }
        } else {
            ESP_LOGI(TAG, "Pump control initialized stopped with relay inactive");
        }
    } else {
        ESP_LOGE(TAG, "Pump control init failed; continuing with Wi-Fi/web setup");
    }

    /* 2. Initialize session system */
    session_init();
    ESP_LOGI(TAG, "Session system initialized");

    /* 3. Initialize Wi-Fi (AP + STA with event handlers) */
    if (!wifi_manager_init()) {
        ESP_LOGE(TAG, "Wi-Fi init failed — continuing in AP-only mode");
    } else {
        ESP_LOGI(TAG, "Wi-Fi initialized");
    }
    ESP_LOGI(TAG, "AP SSID: %s, IP: %s", APP_TEMPLATE_AP_SSID, wifi_manager_get_ap_ip());

    /* 4. Start HTTP server */
    if (!web_server_start()) {
        ESP_LOGE(TAG, "HTTP server start failed — will retry");
        s_http_server_retry = true;
    } else {
        ESP_LOGI(TAG, "HTTP server started");
    }

    /* 5. Disable Wi-Fi power save (prevents mMC multicast loss) */
    esp_wifi_set_ps(WIFI_PS_NONE);

    /* 6. Initialize mDNS */
    esp_err_t mdns_err = mdns_init();
    if (mdns_err != ESP_OK) {
        ESP_LOGE(TAG, "mDNS init failed: %d", mdns_err);
    } else {
        mdns_hostname_set(APP_TEMPLATE_MDNS_HOSTNAME);
        mdns_instance_name_set(APP_TEMPLATE_MDNS_INSTANCE_NAME);
        mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
        ESP_LOGI(TAG, "mDNS initialized: http://%s.local", APP_TEMPLATE_MDNS_HOSTNAME);
    }

    /* 7. Start captive-portal DNS server (responds all queries with AP IP) */
    if (!dns_server_start()) {
        ESP_LOGE(TAG, "DNS server start failed");
    } else {
        ESP_LOGI(TAG, "DNS fallback server started (port 53)");
    }

    /* 8. Initialize task watchdog (30s timeout with panic) */
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = APP_TEMPLATE_MAIN_WDT_TIMEOUT_MS,
        .idle_core_mask = 0,
        .trigger_panic = true,
    };
    esp_err_t wdt_ret = esp_task_wdt_init(&wdt_config);
    if (wdt_ret == ESP_OK) {
        esp_task_wdt_add(NULL);
        ESP_LOGI(TAG, "TWDT initialized (%d ms timeout, panic on trigger)",
                 APP_TEMPLATE_MAIN_WDT_TIMEOUT_MS);
    } else if (wdt_ret == ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "TWDT already initialized, adding main task (timeout may differ)");
        esp_task_wdt_add(NULL);
    } else {
        ESP_LOGE(TAG, "TWDT init failed: %s (continuing without watchdog)",
                 esp_err_to_name(wdt_ret));
    }

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  System Ready");
    ESP_LOGI(TAG, "  AP SSID: %s", APP_TEMPLATE_AP_SSID);
    ESP_LOGI(TAG, "  AP IP: %s", wifi_manager_get_ap_ip());
    ESP_LOGI(TAG, "  URL: http://%s.local (mDNS)", APP_TEMPLATE_MDNS_HOSTNAME);
    ESP_LOGI(TAG, "  URL: http://%s (direct IP - always works)", wifi_manager_get_ap_ip());
    if (wifi_manager_is_sta_connected()) {
        ESP_LOGI(TAG, "  STA IP: %s", wifi_manager_get_sta_ip());
        ESP_LOGI(TAG, "  URL: http://%s.local (desktop/iOS on LAN)", APP_TEMPLATE_MDNS_HOSTNAME);
    }
    ESP_LOGI(TAG, "  Login user: %s", APP_TEMPLATE_DEFAULT_USERNAME);
    ESP_LOGI(TAG, "  WARNING: Change default credentials!");
    ESP_LOGI(TAG, "========================================");

    /* Main loop — reset WDT every 5s, retry HTTP server, log status every 30s */
    uint32_t loop_counter = 0;
    while (1) {
        esp_task_wdt_reset();

        if (s_http_server_retry) {
            if (web_server_start()) {
                ESP_LOGI(TAG, "HTTP server started on retry");
                s_http_server_retry = false;
            }
        }

        loop_counter++;
        if (loop_counter >= APP_TEMPLATE_STATUS_LOG_INTERVALS) {
            ESP_LOGI(TAG, "[STATUS] AP=%s, IP=%s, STA=%s, Heap=%lu",
                wifi_manager_is_ap_enabled() ? "ON" : "OFF",
                wifi_manager_get_ap_ip(),
                wifi_manager_is_sta_connected() ? wifi_manager_get_sta_ip() : "disconnected",
                (unsigned long)wifi_manager_get_free_heap());
            loop_counter = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
