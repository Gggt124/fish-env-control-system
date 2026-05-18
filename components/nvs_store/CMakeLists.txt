idf_component_register(
    SRCS "nvs_store.c"
    INCLUDE_DIRS "."
    REQUIRES app_config nvs_flash
)
