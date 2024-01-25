#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "tcp.h"

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return;
    }

    cyw43_arch_enable_sta_mode();

    while(1) {
        // Wifi appears to disconnect after some time, so re-check connection on a timer
        if(!cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
            run_tcp_client_test();
        }
    }

    cyw43_arch_deinit();
    return 0;
}
