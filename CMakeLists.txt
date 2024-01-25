cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(snowdon_ii_wifi C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# if (PICO_CYW43_SUPPORTED)
    if (NOT TARGET pico_cyw43_arch)
        # error("BOARD must equal pico_w")
    else()
        set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "WiFi SSID")
        set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password")


        if ("${WIFI_SSID}" STREQUAL "")
            error("WIFI_SSID is not defined")
        elseif ("${WIFI_PASSWORD}" STREQUAL "")
            error("WIFI_PASSWORD is not defined")
        else()
            add_subdirectory(src)
        endif()
    endif()
# endif()

