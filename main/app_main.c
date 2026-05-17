#include "nvs_store.h"
#include "wifi_manager.h"
#include "session.h"
#include "web_server.h"
#include "dns_server.h"
#include "netif_helper.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "mdns.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "app_main";

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
        ESP_LOGE(TAG, "Wi-Fi init failed");
        return;
    }
    ESP_LOGI(TAG, "Wi-Fi initialized");
    ESP_LOGI(TAG, "AP SSID: %s, IP: %s", "ESP32-Control-Setup", wifi_manager_get_ap_ip());

    /* 4. Start HTTP server */
    if (!web_server_start()) {
        ESP_LOGE(TAG, "HTTP server start failed");
        return;
    }
    ESP_LOGI(TAG, "HTTP server started");

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

    /* 8. Add fixed secondary IP on STA interface for easy access from LAN */
    esp_netif_t *sta_if = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (sta_if && netif_add_alias_ip(sta_if, "10.87.1.1") == ESP_OK) {
        ESP_LOGI(TAG, "  URL: http://10.87.1.1 (fixed IP - all devices on LAN)");
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

    /* Main loop - log status periodically */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(30000));
        ESP_LOGI(TAG, "[STATUS] AP=%s, IP=%s, STA=%s, Heap=%lu",
            wifi_manager_is_ap_enabled() ? "ON" : "OFF",
            wifi_manager_get_ap_ip(),
            wifi_manager_is_sta_connected() ? wifi_manager_get_sta_ip() : "disconnected",
            (unsigned long)wifi_manager_get_free_heap());
    }
}
