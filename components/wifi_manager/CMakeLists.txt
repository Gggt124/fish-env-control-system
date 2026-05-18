idf_component_register(
    SRCS "wifi_manager.c"
    INCLUDE_DIRS "."
    REQUIRES app_config nvs_store esp_wifi esp_event esp_netif esp_timer freertos
)
