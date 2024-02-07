#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/platform.h"
#include "pico/util/datetime.h"
#include "pico/cyw43_arch.h"


extern "C" {
#include "http_tcp.h"
#include "power.h"
#include "ntp.h"
}

#include "libraries/badger2040w/badger2040w.hpp"

#ifndef PROGMEM
#define memcpy_P memcpy
#define PROGMEM
#endif

using namespace pimoroni;


Badger2040W badger;
int32_t WIDTH = 296, HEIGHT= 128;

datetime_t ntp_daily_alarm = datetime_t{
    .day    = -1,
    .hour   = 00,
    .min    = 00,
    .sec    = 00
};


const uint8_t wifi_icon[] PROGMEM = {
    0b00000000, 0b00000000, //
    0b00000111, 0b11100000, //      ######
    0b00011111, 0b11111000, //    ##########
    0b00111111, 0b11111100, //   ############
    0b01110000, 0b00001110, //  ###        ###
    0b01100111, 0b11100110, //  ##  ######  ##
    0b00001111, 0b11110000, //     ########
    0b00011000, 0b00011000, //    ##      ##
    0b00000011, 0b11000000, //       ####
    0b00000111, 0b11100000, //      ######
    0b00000100, 0b00100000, //      #    #
    0b00000001, 0b10000000, //        ##
    0b00000001, 0b10000000, //        ##
    0b00000000, 0b00000000, //
    0b00000000, 0b00000000, //
    0b00000000, 0b00000000, //
};

const uint8_t no_wifi_icon[] PROGMEM = {
    0b00000000, 0b00000000, 
    0b00000000, 0b00000000,       
    0b00000000, 0b00000000,     
    0b00000000, 0b00000000,    
    0b00000000, 0b00000000,           
    0b00000000, 0b00000000,       
    0b00000000, 0b00000000,      
    0b00000000, 0b00000000,           
    0b00000000, 0b00000000,        
    0b00000000, 0b00000000,       
    0b00000000, 0b00000000,           
    0b00000001, 0b10000000, //        ##
    0b00000001, 0b10000000, //        ##
    0b00000000, 0b00000000, //
    0b00000000, 0b00000000, //
    0b00000000, 0b00000000, //
};

static const uint8_t battery_charging[32] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x80, 0x02, 0xbf, 0xfa, 0xbd, 0xfb, 0xb8, 0x9b, 0xb2, 0x3b, 0xbf, 0x7b, 0xbf, 0xfa, 0x80, 0x02, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t battery_icon[32] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x80, 0x02, 0x80, 0x02, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x02, 0x80, 0x02, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t padlock_64x64[512] PROGMEM =  {
    255, 255, 254, 0, 0, 127, 255, 255, 255, 255, 252, 0, 0, 63, 255, 255, 255, 255, 248, 0, 0, 63, 255, 255, 255, 255, 240, 0, 0, 31, 255, 255, 255, 255, 240, 0, 0, 15, 255, 255, 255, 255, 224, 123, 236, 7, 255, 255, 255, 255, 192, 127, 254, 3, 255, 255, 255, 255, 128, 255, 255, 1, 255, 255, 255, 255, 1, 255, 255, 129, 255, 255, 255, 254, 3, 255, 255, 192, 127, 255, 255, 254, 7, 255, 255, 192, 127, 255, 255, 252, 15, 255, 255, 240, 63, 255, 255, 248, 15, 255, 255, 240, 127, 255, 255, 248, 63, 255, 255, 248, 255, 255, 255, 248, 63, 255, 255, 253, 255, 255, 255, 248, 63, 255, 255, 255, 255, 255, 255, 248, 63, 255, 255, 255, 255, 255, 255, 248, 63, 255, 255, 255, 255, 255, 255, 248, 56, 0, 0, 31, 255, 255, 255, 248, 48, 0, 0, 15, 255, 255, 255, 248, 32, 0, 0, 7, 255, 255, 255, 248, 0, 0, 0, 3, 255, 255, 255, 248, 0, 0, 0, 1, 255, 255, 255, 248, 0, 0, 0, 1, 255, 255, 255, 248, 0, 0, 0, 0, 127, 255, 255, 248, 0, 255, 255, 0, 63, 255, 255, 248, 1, 255, 255, 128, 31, 255, 255, 240, 3, 255, 255, 192, 15, 255, 255, 224, 7, 255, 255, 224, 7, 255, 255, 192, 15, 255, 255, 240, 3, 255, 255, 192, 31, 255, 255, 248, 3, 255, 255, 128, 63, 255, 255, 252, 1, 255, 255, 128, 127, 253, 127, 254, 1, 255, 255, 128, 255, 248, 31, 255, 1, 255, 255, 128, 255, 224, 7, 255, 1, 255, 255, 128, 255, 192, 7, 255, 1, 255, 255, 128, 255, 193, 3, 255, 1, 255, 255, 128, 255, 131, 195, 255, 1, 255, 255, 128, 255, 199, 195, 255, 1, 255, 255, 128, 255, 131, 131, 255, 1, 255, 255, 128, 255, 193, 131, 255, 1, 255, 255, 128, 255, 193, 7, 255, 1, 255, 255, 128, 255, 225, 143, 255, 1, 255, 255, 128, 255, 225, 7, 255, 1, 255, 255, 128, 255, 225, 15, 255, 1, 255, 255, 128, 255, 225, 15, 255, 1, 255, 255, 128, 255, 224, 15, 255, 1, 255, 255, 128, 255, 240, 15, 255, 1, 255, 255, 128, 255, 240, 31, 255, 1, 255, 255, 128, 127, 252, 127, 254, 1, 255, 255, 128, 63, 255, 255, 252, 1, 255, 255, 192, 31, 255, 255, 248, 3, 255, 255, 192, 15, 255, 255, 240, 3, 255, 255, 240, 7, 255, 255, 224, 15, 255, 255, 240, 3, 255, 255, 192, 15, 255, 255, 252, 1, 255, 255, 128, 31, 255, 255, 252, 0, 255, 255, 0, 127, 255, 255, 255, 0, 0, 0, 0, 127, 255, 255, 255, 0, 0, 0, 0, 255, 255, 255, 255, 192, 0, 0, 3, 255, 255, 255, 255, 192, 0, 0, 3, 255, 255, 255, 255, 240, 0, 0, 7, 255, 255, 255, 255, 240, 0, 0, 31, 255, 255, 255, 255, 252, 0, 0, 63, 255, 255
};


static const uint8_t bulb_64x64[512] PROGMEM = {
    0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xfe, 0x00, 0xff, 0xff, 0xff, 0xfe, 0x01, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xfc, 0x03, 0xff, 0xff, 0x80, 0x7f, 0xff, 0xff, 0xf8, 0x07, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xf8, 0x07, 0xff, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xfc, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xfc, 0x07, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xf8, 0x07, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xe0, 0x0f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xfc, 0x01, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xff, 0xfe, 0x00, 0xff, 0xfe, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x7f, 0xfe, 0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 0x7f, 0xfc, 0x03, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x1f, 0xf8, 0x07, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x1f, 0xf8, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x1f, 0xf8, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x1f, 0xf8, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x1f, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x1f, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x1f, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x1f, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x15, 0x50, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd2, 0x4f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static const uint8_t bulb_off_64x64[512] PROGMEM = {
    0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

bool got_time = false;

void recv_time(datetime_t *datetime, void *arg) {
    printf("Got datetime from NTP\n");
    badger.pcf85063a->set_datetime(datetime);
    rtc_set_datetime(datetime);
    got_time = true;
}

void recv_http(uint8_t *buffer, void *arg) {
    printf("recv_http: %.2048s\n", buffer);
}

int main() {
    badger.init();
    badger.led(255);
    
    stdio_init_all();
    adc_init();
    rtc_init();

    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();
    // Use cyw43_wifi_join so that wifi channel can be specified. This shaves ~700ms of connection time, static IP / disable DNS in lwipopts.h shaves ~800ms too
    cyw43_wifi_join(&cyw43_state, strlen(WIFI_SSID), (const uint8_t *)WIFI_SSID, 
        strlen(WIFI_PASSWORD), (const uint8_t *)WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 
        (const uint8_t[]){0x9C, 0x53, 0x22, 0x90, 0xD3, 0xC8}, 11);

    badger.graphics->set_font("bitmap8");
    badger.uc8151->set_update_speed(2);
    badger.graphics->set_thickness(2);

    badger.graphics->set_pen(15);
    badger.graphics->clear();

    badger.graphics->set_pen(0);
    badger.graphics->rectangle(Rect(0, 0, WIDTH, 20));
    badger.graphics->set_pen(15);
    badger.graphics->text("restfulBadger", Point(4, 6), WIDTH, 1.0);
    // badger.image((wifi_icon, Rect(WIDTH - 16 - 2, 2, 16, 16));


    if (power_is_charging()) {
        badger.image(wifi_icon, Rect(WIDTH - 32 - 8, 2, 16, 16));
        badger.image(battery_charging, Rect(WIDTH - 16 - 4, 2, 16, 16));
        // badger.image(bulb_off_64x64, Rect(20, 40, 64, 64));
    } else {
        // badger.image(bulb_64x64, Rect(20, 40, 64, 64));
        badger.image(wifi_icon, Rect(WIDTH - 52 - 10, 2, 16, 16));
        badger.image(battery_icon, Rect(WIDTH - 36 - 6, 2, 16, 16));
        badger.graphics->set_pen(15);
        char percentStr[5];
        float voltage;
        power_voltage(&voltage);
        int powerPercent = power_percent(&voltage);
        sprintf(percentStr, "%d%%", powerPercent);
        badger.graphics->text(percentStr, Point(WIDTH - 20 - 2, 6), WIDTH, 1.0);
        badger.graphics->rectangle(Rect(WIDTH - 36 - 4, 7, powerPercent / 10 + 1, 6));
    }
    power_print(); 
    while(cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_UP) {
        sleep_ms(10);
    }
    printf("connected\n");

    std::string button = "";
    if(badger.pressed_to_wake(badger.A))        { button += "A"; http_request(API_SERVER , "/v2/meross/sad", "POST", "{\"code\": \"toggle\"}", recv_http, NULL);}
    else if(badger.pressed_to_wake(badger.B))   { button += "B"; http_request(API_SERVER, "/v2/meross/office", "POST", "{\"code\": \"toggle\"}", recv_http, NULL); }
    else if(badger.pressed_to_wake(badger.C))   { button += "C"; http_request(API_SERVER, "/v2/wol/pc", "POST", "{\"code\": \"status\"}", recv_http, NULL); }

    // if (button != "") {
    //     badger.graphics->text("Button " + button + " pressed", Point(10, 100), 200, 2);
    // } else {
    //     badger.graphics->text("Button not detected", Point(10, 100), 200, 2);
    // }

    datetime_t datetime;
    badger.update_button_states();
    // Check if RTC has been initialised previously, if not or if the RTC int is high get the internet time via NTP
    if (!badger.pcf85063a->get_byte() || badger.pressed(badger.RTC)) {
        printf("Retrieving time from NTP\n");
        ntp_get_time(recv_time, NULL);
        while(!got_time) {
            sleep_ms(10);
        }
        rtc_get_datetime(&datetime);
        badger.update_button_states();
        if (!badger.pressed(badger.RTC)) {
            printf("Setting daily NTP time alarm\n");
            // Set daily alarm and enable interrupt
            badger.pcf85063a->set_alarm(ntp_daily_alarm.sec, ntp_daily_alarm.min, ntp_daily_alarm.hour, ntp_daily_alarm.day);
            badger.pcf85063a->enable_alarm_interrupt(true);
            // Set byte on external RTC to indicate that it has been initialised
            badger.pcf85063a->set_byte(1);
        }

        // Clear source of interrupt
        badger.pcf85063a->clear_alarm_flag();

    } else {
        printf("Retrieving time from RTC\n");
        // Retrieve stored time from external RTC
        datetime = badger.pcf85063a->get_datetime();
        // Store time in internal RTC
        rtc_set_datetime(&datetime);
    }
    char datetime_str[10];
    sprintf(datetime_str, "%02d:%02d\n", datetime.hour, datetime.min);
    printf("%s\n", datetime_str);
    badger.graphics->set_pen(15);
    badger.graphics->text(datetime_str, Point(WIDTH / 2 - 20, 3), WIDTH, 2);
    badger.update();
    cyw43_arch_deinit();

    badger.led(0);
    badger.halt();
    return 0;
}