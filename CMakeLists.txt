cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS "${CMAKE_SOURCE_DIR}/components")

project(main_dashboard_mcu)

# Use sdkconfig.defaults if present
if(EXISTS ${CMAKE_SOURCE_DIR}/sdkconfig.defaults)
    set(SDKCONFIG_DEFAULTS ${CMAKE_SOURCE_DIR}/sdkconfig.defaults)
endif()
