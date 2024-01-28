#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/platform.h"
#include "pico/cyw43_arch.h"
// #include "hardware/vreg.h"
// #include "hardware/clocks.h"
// #include "hardware/pll.h"


extern "C" {
#include "http_tcp.h"
}

#include "libraries/badger2040w/badger2040w.hpp"

using namespace pimoroni;

Badger2040W badger;

int main() {
    // // overclock to 250 MHZ to get fa
    // vreg_set_voltage(VREG_VOLTAGE_1_20);
    // set_sys_clock_khz(250000, true);
    badger.init();
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 0;
    }

    cyw43_arch_enable_sta_mode();

    badger.uc8151->set_update_speed(2);
    badger.graphics->set_thickness(2);

    badger.graphics->set_pen(15);
    badger.graphics->clear();
    badger.graphics->set_pen(0);
    badger.graphics->text("Connecting to Wifi", Point(10, 20), 200, 3);
    badger.update();

    while(1) {
        // Wifi appears to disconnect after some time, so re-check connection on a timer
        if(!cyw43_arch_wifi_connect_bssid_timeout_ms(WIFI_SSID, (const uint8_t[]){0x9C, 0x53, 0x22, 0x90, 0xD3, 0xC8}, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
            break;
        }
    }

    badger.graphics->set_pen(15);
    badger.graphics->clear();
    badger.graphics->set_pen(0);
    badger.graphics->text("Connected", Point(10, 20), 200, 3);
    badger.update();



    // std::string button = "";
    // if(badger.pressed_to_wake(badger.A))      { button += "A"; http_request("POST", "/v2/meross/sad", "{\"code\": \"toggle\"}");}
    // else if(badger.pressed_to_wake(badger.B)) { button += "B"; http_request("POST", "/v2/meross/office", "{\"code\": \"toggle\"}"); }
    // else if(badger.pressed_to_wake(badger.C)) { button += "C"; http_request("POST", "/v2/wol/pc", "{\"code\": \"status\"}"); }
    // cyw43_arch_deinit();

    // if (button != "") {
    //     badger.graphics->text("Button " + button + " pressed to wake, going to sleep", Point(10, 80), 200, 2);
    // } else {
    //     badger.graphics->text("Wake button not detected, going to sleep", Point(10, 80), 200, 2);
    // }
    while(true) {
        badger.wait_for_press();
        std::string button = "";
        if(badger.pressed(badger.A))      { button += "A"; http_request("POST", "/v2/meross/sad", "{\"code\": \"toggle\"}");}
        else if(badger.pressed(badger.B)) { button += "B"; http_request("POST", "/v2/meross/office", "{\"code\": \"toggle\"}"); }
        else if(badger.pressed(badger.C)) { button += "C"; http_request("POST", "/v2/wol/pc", "{\"code\": \"status\"}"); }

        badger.graphics->set_pen(15);
        badger.graphics->clear();
        badger.graphics->set_pen(0);
        badger.graphics->text("Connected", Point(10, 20), 200, 3);
        if (button != "") {
            badger.graphics->text("Button " + button + " pressed", Point(10, 80), 200, 2);
        } else {
            badger.graphics->text("Button not detected", Point(10, 80), 200, 2);
        }
        badger.update();
    }

    cyw43_arch_deinit();
    badger.halt();
    return 0;
}