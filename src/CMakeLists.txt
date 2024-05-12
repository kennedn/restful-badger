add_executable(badger)

# Default definitions
set(NTP_SERVER "pool.ntp.org" CACHE STRING "Specify the NTP server")
set(IP_ADDRESS "192.168.1.204" CACHE STRING "Specify IP Address")
set(IP_GATEWAY "192.168.1.1" CACHE STRING "Specify IP Gateway")
set(IP_DNS "192.168.1.1" CACHE STRING "Specify IP DNS")

# BSSID format
string(REPLACE ":" ", 0x" BSSID_HEX "${WIFI_BSSID}")
set(BSSID_FMT "(const uint8_t[]){ 0x${BSSID_HEX} }")

# IP format
string(REPLACE "." ", " IP_ADDRESS_FMT "${IP_ADDRESS}")
string(REPLACE "." ", " IP_GATEWAY_FMT "${IP_GATEWAY}")
string(REPLACE "." ", " IP_DNS_FMT "${IP_DNS}")
set(IP_ADDRESS_FMT "LWIP_MAKEU32(${IP_ADDRESS_FMT})")
set(IP_GATEWAY_FMT "LWIP_MAKEU32(${IP_GATEWAY_FMT})")
set(IP_DNS_FMT "LWIP_MAKEU32(${IP_DNS_FMT})")

set(JSON_FILEPATH "${PROJECT_SOURCE_DIR}/config/tiles.json" CACHE STRING "Location of tiles json")

execute_process(COMMAND "${PROJECT_SOURCE_DIR}/tools/json_to_c_array.py" "-f" "${JSON_FILEPATH}" OUTPUT_VARIABLE "TILES_DATA")

target_compile_definitions(badger PRIVATE 
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    WIFI_BSSID=${BSSID_FMT}
    WIFI_CHANNEL=${WIFI_CHANNEL}
    API_SERVER=\"${API_SERVER}\"

    NTP_SERVER=\"${NTP_SERVER}\"
    IP_ADDRESS=${IP_ADDRESS_FMT}
    IP_GATEWAY=${IP_GATEWAY_FMT}
    IP_DNS=${IP_DNS_FMT}
    TILES_DATA=${TILES_DATA}
)
add_subdirectory(modules)

target_sources(badger PRIVATE 
    badger.cpp 
)

target_include_directories(badger PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

include(${PIMORONI_PICO_PATH}/libraries/badger2040w/badger2040w.cmake)
include(${PIMORONI_PICO_PATH}/common/pimoroni_bus.cmake)

target_link_libraries(badger
    hardware_adc
    badger2040w
    pimoroni_bus
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mbedtls
    pico_mbedtls
)

pico_enable_stdio_usb(badger 1)

pico_add_extra_outputs(badger)
