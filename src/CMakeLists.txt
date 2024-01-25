add_executable(badger)

target_compile_definitions(badger PRIVATE 
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    TEST_TCP_SERVER_IP=\"${TEST_TCP_SERVER_IP}\"
)
target_sources(badger PRIVATE badger.c tcp.c)

target_include_directories(badger PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(badger PRIVATE 
    pico_stdlib 
    pico_cyw43_arch_lwip_threadsafe_background
)
pico_add_extra_outputs(badger)
