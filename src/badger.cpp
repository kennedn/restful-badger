#include <stdio.h>
#include <stdlib.h>

#include "pico/cyw43_arch.h"
#include "pico/platform.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"

extern "C" {
#include "http.h"
#include "images.h"
#include "ntp.h"
#include "power.h"
#include "restful.h"
#include "tiles.h"
}

#include "badger.h"
#include "libraries/badger2040w/badger2040w.hpp"


using namespace pimoroni;

Badger2040W badger;
int32_t WIDTH = 296, HEIGHT = 128;

// 12am daily alarm
datetime_t ntp_daily_alarm = datetime_t{
    .day = -1,
    .hour = 00,
    .min = 00,
    .sec = 00};

static volatile bool ntp_time_set = false;
void draw_status_bar();

void ntp_callback(datetime_t *datetime, void *arg) {
    datetime_t *new_datetime = (datetime_t *)arg;
    if (datetime == NULL) {
        ntp_time_set = true;
        return;
    }
    DEBUG_printf("Got datetime from NTP\n");
    badger.pcf85063a->set_datetime(datetime);
    rtc_set_datetime(datetime);

    badger.update_button_states();
    if (!badger.pressed(badger.RTC)) {
        DEBUG_printf("Setting daily NTP time alarm\n");
        // Set daily alarm and enable interrupt
        badger.pcf85063a->set_alarm(ntp_daily_alarm.sec, ntp_daily_alarm.min, ntp_daily_alarm.hour, ntp_daily_alarm.day);
        badger.pcf85063a->enable_alarm_interrupt(true);
        // Set byte on external RTC to indicate that it has been initialised
        badger.pcf85063a->set_byte(1);
    }

    // Clear source of interrupt
    badger.pcf85063a->clear_alarm_flag();

    // Copy datetime data to user provided pointer
    *new_datetime = *datetime;
    // Flag that NTP callback has concluded
    ntp_time_set = true;
}

bool wifi_up() {
    return cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
}

void retrieve_time(datetime_t *datetime) {
    ntp_time_set = false;
    badger.update_button_states();
    // Check if RTC has been initialised previously, if not or if the RTC int is high get the internet time via NTP
    if (!badger.pcf85063a->get_byte() || badger.pressed(badger.RTC)) {
        DEBUG_printf("Retrieving time from NTP\n");
        // Wifi must be up for the NTP request to succeed
        while (!wifi_up()) {
            sleep_ms(10);
        }
        ntp_get_time(ntp_callback, datetime);
        // Block until the callback sets datetime
        while (!ntp_time_set) {
            sleep_ms(10);
        }
    } else {
        DEBUG_printf("Retrieving time from RTC\n");
        // Retrieve stored time from external RTC
        *datetime = badger.pcf85063a->get_datetime();
        // Store time in internal RTC
        rtc_set_datetime(datetime);
    }
}

void init() {
    badger.init();
    badger.led(255);

    stdio_init_all();
    adc_init();
    rtc_init();

    if (cyw43_arch_init()) {
        DEBUG_printf("failed to initialise\n");
        return;
    }
    cyw43_arch_enable_sta_mode();
    // Use cyw43_wifi_join so that wifi channel can be specified. This shaves ~700ms of connection time, static IP / disable DNS in lwipopts.h shaves ~800ms too
    cyw43_wifi_join(&cyw43_state, strlen(WIFI_SSID), (const uint8_t *)WIFI_SSID, strlen(WIFI_PASSWORD),
                    (const uint8_t *)WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, WIFI_BSSID, WIFI_CHANNEL);

    badger.graphics->set_font("bitmap8");
    badger.uc8151->set_update_speed(2);
    badger.graphics->set_thickness(2);

    badger.graphics->set_pen(15);
    badger.graphics->clear();
}

void deinit() {
    cyw43_arch_deinit();
    badger.led(0);
    badger.halt();
}

void draw_status_bar() {
    datetime_t datetime;
    char datetime_str[6];
    char percentStr[5];
    float voltage;
    int powerPercent;
    int x_offset = 0;
    int x_pad = 4;
    int icon_size = 16;
    int clock_x_offset = 20;
    uint8_t *wifi_image = (uint8_t *)image_wifi_icon;
    uint8_t *battery_image = (uint8_t *)image_battery_charging;
    if (!power_is_charging()) {
        battery_image = (uint8_t *)image_battery;
        x_offset = 22;

        power_voltage(&voltage);
        powerPercent = power_percent(&voltage);
        sprintf(percentStr, "%d%%", powerPercent);
    }

    if (!wifi_up()) {
        wifi_image = (uint8_t *)image_no_wifi_icon;
    }

    retrieve_time(&datetime);
    sprintf(datetime_str, "%02d:%02d\n", datetime.hour, datetime.min);

    badger.graphics->set_pen(0);
    badger.graphics->rectangle(Rect(0, 0, WIDTH, 20));

    badger.graphics->set_pen(15);
    badger.graphics->text("restfulBadger", Point(4, 6), WIDTH, 1.0);
    badger.graphics->text(datetime_str, Point(WIDTH / 2 - clock_x_offset, 3), WIDTH, 2);
    badger.image(battery_image, Rect(WIDTH - x_offset - (icon_size + x_pad), 2, icon_size, icon_size));
    badger.image(wifi_image, Rect(WIDTH - x_offset - (icon_size + x_pad) * 2, 2, icon_size, icon_size));
    if (!power_is_charging()) {
        badger.graphics->set_pen(15);
        badger.graphics->text(percentStr, Point(WIDTH - x_offset, 6), WIDTH, 1.0);
        badger.graphics->rectangle(Rect(WIDTH - x_offset - (icon_size + x_pad) + 2, 7, powerPercent / 10 + 1, 6));
    }
}

void draw_tiles(const char *name, const char *status_icon) {
    char tiles_base_idx = tiles_get_idx();
    for(char i=0; i< 3; i++) {
        TILE *tile = tile_array->tiles[tiles_base_idx + i];
        badger.image((const uint8_t *)tile->image, Rect(18 + (99*i), 32, 64, 64));
        badger.graphics->set_pen(0);
        badger.graphics->text(tile->name, Point(18 + (99*i), 104), WIDTH, 2);
        if(!strcmp(name, tile->name)) {
            badger.image((const uint8_t *)status_icon, Rect(18 + (99*i) + 44, 32 + 44, 16, 16));
        }
    }

    for (char i=0; i < tiles_max_column(); i++) {
        badger.graphics->set_pen(0);
        char y = (128 / 2) - (tiles_max_column() * 10 / 2) + (i * 10);
        badger.graphics->rectangle(Rect(286, y, 8, 8)); 
        if (tiles_get_column() != i) {
            badger.graphics->set_pen(15);
            badger.graphics->rectangle(Rect(286 + 1, y + 1, 6, 6)); 
        }
    }
}

void restful_callback(char *value, int status_code, void *arg) {
    if (arg == NULL) {
        return;
    }

    RESTFUL_REQUEST *request = (RESTFUL_REQUEST *)arg;

    TILE *tile = (TILE *)request->tile;

    if (status_code != 200) {
        DEBUG_printf("restful_callback: value=%s, status_code=%d, caller=%s\n", value, status_code, tile->name);
        return;
    }

    DEBUG_printf("restful_callback: value=%s, status_code=%d, caller=%s\n", value, status_code, tile->name);
    if (!strcmp(tile->status_on, value)) {
        draw_tiles(tile->name, tick);
    } else if(!strcmp(tile->status_off, value)) {
        draw_tiles(tile->name, cross);
    }
    draw_status_bar();
    badger.update();
    restful_free_request(request);
}

int main() {
    init();
    tiles_make_tiles();
    tiles_set_column((badger.pcf85063a->get_byte() - 1 >= 0) ? badger.pcf85063a->get_byte() - 1: 0);

    draw_tiles(NULL, NULL);
    draw_status_bar();
    badger.update();
    while(true) {
        badger.wait_for_press();

        if(!wifi_up()) {
            continue;
        }

        char tiles_base_idx = tiles_get_idx();
        TILE *tile;
        if (badger.pressed(badger.A)) {
            tile = tile_array->tiles[tiles_base_idx];
        } else if(badger.pressed(badger.B)) {
            tile = tile_array->tiles[tiles_base_idx + 1];
        } else if(badger.pressed(badger.C)) {
            tile = tile_array->tiles[tiles_base_idx + 2];
        } else if(badger.pressed(badger.UP)) {
            tiles_previous_column();
            badger.pcf85063a->set_byte(tiles_get_column()+1);
            badger.graphics->set_pen(15);
            badger.graphics->clear();
            draw_tiles(NULL, NULL);
            draw_status_bar();
            badger.update();
            continue;
        } else if(badger.pressed(badger.DOWN)) {
            tiles_next_column();
            badger.pcf85063a->set_byte(tiles_get_column()+1);
            badger.graphics->set_pen(15);
            badger.graphics->clear();
            draw_tiles(NULL, NULL);
            draw_status_bar();
            badger.update();
            continue;
        }

        RESTFUL_REQUEST *request = restful_make_request(
            tile,
            tile_array->base_url,
            (RESTFUL_REQUEST_DATA *)(tile->action_request),
            (RESTFUL_REQUEST_DATA *)(tile->status_request),
            restful_callback
        );
        restful_request(request);
    }
    deinit();
    return 0;
}