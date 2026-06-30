idf_component_register(
    SRCS "tft_display.c"
    INCLUDE_DIRS "."
    REQUIRES
        app_config
        pump_control
        cooling_control
        wifi_manager
        esp_lcd
        esp_driver_gpio
        esp_driver_ledc
        esp_timer
        freertos
)
