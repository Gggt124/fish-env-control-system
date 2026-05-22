idf_component_register(
    SRCS "hardware_map.c"
    INCLUDE_DIRS "."
    REQUIRES app_config esp_driver_gpio
)
