#include "nvs_store.h"
#include "wifi_manager.h"
#include "session.h"
#include "web_server.h"
#include "dns_server.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "mdns.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"

static const char *TAG = "app_main";
static bool s_http_server_retry = false;

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  ESP32 Control System v1.0.0");
    ESP_LOGI(TAG, "  Phase 1 Foundation");
    ESP_LOGI(TAG, "========================================");

    /* 1. Initialize NVS storage */
    if (!nvs_store_init()) {
        ESP_LOGE(TAG, "NVS init failed");
        return;
    }
    ESP_LOGI(TAG, "NVS initialized");

    /* 2. Initialize session system */
    session_init();
    ESP_LOGI(TAG, "Session system initialized");

    /* 3. Initialize Wi-Fi (AP + STA with event handlers) */
    if (!wifi_manager_init()) {
        ESP_LOGE(TAG, "Wi-Fi init failed — continuing in AP-only mode");
    } else {
        ESP_LOGI(TAG, "Wi-Fi initialized");
    }
    ESP_LOGI(TAG, "AP SSID: %s, IP: %s", "ESP32-Control-Setup", wifi_manager_get_ap_ip());

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
        mdns_hostname_set("home1");
        mdns_instance_name_set("ESP32 Dashboard");
        mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
        ESP_LOGI(TAG, "mDNS initialized: http://home1.local");
    }

    /* 7. Start captive-portal DNS server (responds all queries with AP IP) */
    if (!dns_server_start()) {
        ESP_LOGE(TAG, "DNS server start failed");
    } else {
        ESP_LOGI(TAG, "DNS fallback server started (port 53)");
    }

    /* 8. Initialize task watchdog (30s timeout with panic) */
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = 30000,
        .idle_core_mask = 0,
        .trigger_panic = true,
    };
    esp_err_t wdt_ret = esp_task_wdt_init(&wdt_config);
    if (wdt_ret == ESP_OK) {
        esp_task_wdt_add(NULL);
        ESP_LOGI(TAG, "TWDT initialized (30s timeout, panic on trigger)");
    } else if (wdt_ret == ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "TWDT already initialized, adding main task (timeout may differ)");
        esp_task_wdt_add(NULL);
    } else {
        ESP_LOGE(TAG, "TWDT init failed: %s (continuing without watchdog)",
                 esp_err_to_name(wdt_ret));
    }

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  System Ready");
    ESP_LOGI(TAG, "  AP SSID: %s", "ESP32-Control-Setup");
    ESP_LOGI(TAG, "  AP IP: %s", wifi_manager_get_ap_ip());
    ESP_LOGI(TAG, "  URL: http://home1.local (all devices on AP)");
    ESP_LOGI(TAG, "  URL: http://%s (direct IP - always works)", wifi_manager_get_ap_ip());
    if (wifi_manager_is_sta_connected()) {
        ESP_LOGI(TAG, "  STA IP: %s", wifi_manager_get_sta_ip());
        ESP_LOGI(TAG, "  URL: http://home1.local (desktop/iOS on LAN)");
    }
    ESP_LOGI(TAG, "  Login: admin / admin123");
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
        if (loop_counter >= 6) {
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
