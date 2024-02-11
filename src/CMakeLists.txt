add_executable(badger)

# Replace ":" with ", 0x" to form the array initializer
string(REPLACE ":" ", 0x" BSSID_HEX "${WIFI_BSSID}")

# Construct the array initializer string
set(BSSID "(const uint8_t[]){ 0x${BSSID_HEX} }")

target_compile_definitions(badger PRIVATE 
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    WIFI_BSSID=${BSSID}
    WIFI_CHANNEL=${WIFI_CHANNEL}
    API_SERVER=\"${API_SERVER}\"
    NTP_SERVER=\"pool.ntp.org\"
)

target_sources(badger PRIVATE 
    badger.cpp 
    power.c 
    ntp.c
    http.c 
    restful.c
    tiles.c
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
)

pico_enable_stdio_usb(badger 1)

pico_add_extra_outputs(badger)
