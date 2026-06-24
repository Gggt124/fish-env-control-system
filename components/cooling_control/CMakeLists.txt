idf_component_register(
    SRCS "cooling_control.c"
    INCLUDE_DIRS "."
    REQUIRES app_config hardware_map esp_driver_gpio esp_timer freertos onewire_bus ds18b20 esp_task_wdt
)
