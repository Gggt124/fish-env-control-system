idf_component_register(
    SRCS "pump_control.c"
    INCLUDE_DIRS "."
    REQUIRES app_config driver esp_timer freertos
)
