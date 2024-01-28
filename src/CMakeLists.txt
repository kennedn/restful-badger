add_executable(badger)

target_compile_definitions(badger PRIVATE 
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    TCP_DESTINATION_IP=\"${TCP_DESTINATION_IP}\"
    TCP_DESTINATION_HOST=\"${TCP_DESTINATION_HOST}\"
)
target_sources(badger PRIVATE badger.cpp http_tcp.c)

target_include_directories(badger PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

include(${PIMORONI_PICO_PATH}/libraries/badger2040w/badger2040w.cmake)
include(${PIMORONI_PICO_PATH}/common/pimoroni_bus.cmake)
# include(${PIMORONI_PICO_PATH}/common/pimoroni_i2c.cmake)

target_link_libraries(badger
    badger2040w
    pimoroni_bus
    pico_stdlib 
    pico_cyw43_arch_lwip_threadsafe_background
)

pico_enable_stdio_usb(badger 1)

pico_add_extra_outputs(badger)
