cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(Jack2Digital C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

set(ALL_DEVICE_CONFIGS "usb_cdc_sampler")

if(NOT DEFINED DEVICE_CONFIG)
    message(WARNING "DEVICE_CONFIG was not defined by the user.")
    message(STATUS "Defaulting to -DDEVICE_CONFIG=usb_cdc_sampler.")
    message(STATUS "All possible options are (case-sensitive):")
    foreach(config ${ALL_DEVICE_CONFIGS})
        message(STATUS "\t${config}")
    endforeach()

    set(DEVICE_CONFIG "usb_cdc_sampler")
endif()

if(NOT DEVICE_CONFIG IN_LIST ALL_DEVICE_CONFIGS)
    message(STATUS "All possible options of DEVICE_CONFIG are (case-sensitive):")
    foreach(config ${ALL_DEVICE_CONFIGS})
        message(STATUS "\t${config}")
    endforeach()
    message(FATAL_ERROR "${DEVICE_CONFIG} is not a valid DEVICE_CONFIG option!")
endif()

add_executable(${PROJECT_NAME} src/main.cpp src/${DEVICE_CONFIG}/usb_descriptors.c src/${DEVICE_CONFIG}/tusb_config.h)
target_compile_definitions(${PROJECT_NAME} PUBLIC DEVICE_CONFIG_${DEVICE_CONFIG})
target_include_directories(${PROJECT_NAME} PUBLIC src/ src/${DEVICE_CONFIG}/)
target_link_libraries(${PROJECT_NAME} pico_stdlib pico_multicore hardware_adc hardware_timer hardware_clocks hardware_rtc tinyusb_device tinyusb_board)

if(DEVICE_CONFIG STREQUAL "usb_cdc_sampler")
    pico_enable_stdio_usb(${PROJECT_NAME} 1)
endif()
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_add_extra_outputs(${PROJECT_NAME})