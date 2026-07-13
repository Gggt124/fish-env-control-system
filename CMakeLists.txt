cmake_minimum_required(VERSION 3.22)

# Project-wide ESP-IDF inputs must be set before project.cmake/project().
set(EXTRA_COMPONENT_DIRS "${CMAKE_SOURCE_DIR}/components")
set(SDKCONFIG_DEFAULTS "${CMAKE_SOURCE_DIR}/sdkconfig.defaults")

# The build wrapper passes an explicit SDKCONFIG path for each target. Keep a
# target-specific fallback for direct CMake/idf.py use when IDF_TARGET is set.
if(DEFINED IDF_TARGET AND NOT DEFINED SDKCONFIG)
    set(SDKCONFIG "${CMAKE_SOURCE_DIR}/sdkconfig.${IDF_TARGET}")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(fish_pump_relay_timer_control VERSION 0.2.5)

add_compile_definitions(APP_TEMPLATE_FIRMWARE_VERSION="v${PROJECT_VERSION}")
