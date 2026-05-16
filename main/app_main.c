#include "nvs_store.h"
#include "wifi_manager.h"
#include "session.h"
#include "web_server.h"
#include "esp_log.h"
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

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  System Ready");
    ESP_LOGI(TAG, "  1. Connect to AP: ESP32-Control-Setup");
    ESP_LOGI(TAG, "  2. Open http://%s", wifi_manager_get_ap_ip());
    ESP_LOGI(TAG, "  3. Login: admin / admin123");
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
