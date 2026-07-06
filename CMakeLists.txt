cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS "${CMAKE_SOURCE_DIR}/components")

# Use separate sdkconfig file for each target to avoid race conditions
set(SDKCONFIG "${CMAKE_SOURCE_DIR}/sdkconfig.${IDF_TARGET}")

project(fish_pump_relay_timer_control VERSION 0.2.3)

# ESP-IDF v5+ auto-loads sdkconfig.defaults.<IDF_TARGET> after sdkconfig.defaults.
# The unconditional set (no if/EXISTS guard) ensures SDKCONFIG_DEFAULTS is always set.
set(SDKCONFIG_DEFAULTS "${CMAKE_SOURCE_DIR}/sdkconfig.defaults")
