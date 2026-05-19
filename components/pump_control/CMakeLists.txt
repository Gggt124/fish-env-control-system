idf_component_register(
    SRCS "pump_control.c"
    INCLUDE_DIRS "."
    REQUIRES app_config esp_driver_gpio esp_timer freertos
)
