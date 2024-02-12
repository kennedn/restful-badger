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
void draw_status_bar(bool sleep);
void draw_tiles(const char *name, const char *indicator_icon);
void deinit();

void ntp_callback(datetime_t *datetime, void *arg) {
    if (datetime == NULL) {
        ntp_time_set = true;
        return;
    }
    DEBUG_printf("Got datetime from NTP\n");
    badger.pcf85063a->set_datetime(datetime);
    rtc_set_datetime(datetime);

    DEBUG_printf("Setting daily NTP time alarm\n");
    // Set daily alarm and enable interrupt
    badger.pcf85063a->set_alarm(ntp_daily_alarm.sec, ntp_daily_alarm.min, ntp_daily_alarm.hour, ntp_daily_alarm.day);
    badger.pcf85063a->enable_alarm_interrupt(true);

    // Clear source of interrupt
    badger.pcf85063a->clear_alarm_flag();

    // Flag that NTP callback has concluded
    ntp_time_set = true;
}

void restful_callback(char *value, int status_code, void *arg) {
    if (arg == NULL) {
        return;
    }

    RESTFUL_REQUEST *request = (RESTFUL_REQUEST *)arg;

    TILE *tile = (TILE *)request->tile;

    if (status_code != 200) {
        draw_tiles(tile->name, image_indicator_cross);
    } else if (status_code == 200 && !value) {
        draw_tiles(tile->name, image_indicator_tick);
    }

    DEBUG_printf("restful_callback: value=%s, status_code=%d, caller=%s\n", value ? value : "NULL", status_code, tile->name);
    if (value && !strcmp(tile->status_on, value)) {
        draw_tiles(tile->name, image_indicator_tick);
    } else if(value && !strcmp(tile->status_off, value)) {
        draw_tiles(tile->name, image_indicator_cross);
    } else {
        draw_tiles(tile->name, image_indicator_question);
    }
    draw_status_bar();
    badger.update();
    restful_free_request(request);
}

int64_t halt_timeout_callback(alarm_id_t id, void *arg) {
    draw_status_bar(true);
    draw_tiles(NULL, NULL);
    badger.update();
    badger.uc8151->busy_wait();
    deinit();
    return 0;
}



void retrieve_time(bool from_ntp) {
    ntp_time_set = false;
    // Check if RTC has been initialised previously, if not or if the RTC int is high get the internet time via NTP
    if (from_ntp) {
        DEBUG_printf("Retrieving time from NTP\n");
        ntp_get_time(ntp_callback, NULL);
        // Block until the callback sets datetime
        while (!ntp_time_set) {
            tight_loop_contents();
        }
    } else {
        DEBUG_printf("Retrieving time from RTC\n");
        // Retrieve stored time from external RTC
        datetime_t datetime = badger.pcf85063a->get_datetime();
        // Store time in internal RTC
        rtc_set_datetime(&datetime);
    }
}

alarm_id_t reset_halt_timeout(alarm_id_t id) {
    if (id != -1) {
        cancel_alarm(id);
    }
    return add_alarm_in_ms(HALT_TIMEOUT_MS, halt_timeout_callback, NULL, false);
}

bool wifi_up() {
    return cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
}

void wifi_wait() {
    while(!wifi_up()) {
        tight_loop_contents();
    }
}

void wait_for_button_press_release() {
    uint32_t mask = (1UL << badger.A) | (1UL << badger.B) | (1UL << badger.C) | (1UL << badger.UP) | (1UL << badger.DOWN);
    // Wait for button press
    while(!(gpio_get_all() & mask)) {
        tight_loop_contents();
    }
    badger.update_button_states();
    badger.led(0);
    // Wait for button release
    while((gpio_get_all() & mask)) {
        tight_loop_contents();
    }
    badger.led(255);
}

void draw_status_bar() {
    draw_status_bar(false);
}

void draw_status_bar(bool sleep) {
    datetime_t datetime;
    float voltage;
    char powerPercent;
    char percentStr[5];
    char clock_str[9];
    char x_offset = 0;
    char x_pad = 4;
    int32_t clock_x_offset;
    uint8_t *wifi_image = (uint8_t *)image_status_wifi_on;
    uint8_t *battery_image = (uint8_t *)image_status_battery_charging;
    if (!power_is_charging()) {
        battery_image = (uint8_t *)image_status_battery_discharging;
        x_offset = 22;

        power_voltage(&voltage);
        powerPercent = power_percent(&voltage);
        sprintf(percentStr, "%d%%", powerPercent);
    }

    if(sleep) {
        wifi_image = (uint8_t *)image_status_sleeping;
        sprintf(clock_str, "Sleeping\n");
    } else {
        if(!wifi_up()) {
            wifi_image = (uint8_t *)image_status_wifi_off;
        }
        rtc_get_datetime(&datetime);
        sprintf(clock_str, "%02d:%02d\n", datetime.hour, datetime.min);
    }
    clock_x_offset = badger.graphics->measure_text(clock_str, 2.0f) / 2;

    badger.graphics->set_pen(0);
    badger.graphics->rectangle(Rect(0, 0, WIDTH, 20));

    badger.graphics->set_pen(15);
    badger.graphics->text("restfulBadger", Point(4, 6), WIDTH, 1.0);
    badger.graphics->text(clock_str, Point(WIDTH / 2 - clock_x_offset, 3), WIDTH, 2.0f);
    badger.image(battery_image, Rect(WIDTH - x_offset - (image_status_size + x_pad), 2, image_status_size, image_status_size));
    badger.image(wifi_image, Rect(WIDTH - x_offset - (image_status_size + x_pad) * 2, 2, image_status_size, image_status_size));
    if (!power_is_charging()) {
        badger.graphics->set_pen(15);
        badger.graphics->text(percentStr, Point(WIDTH - x_offset, 6), WIDTH, 1.0);
        badger.graphics->rectangle(Rect(WIDTH - x_offset - (image_status_size + x_pad) + 2, 7, powerPercent / 10 + 1, 6));
    }
}

void draw_tiles(const char *name, const char *indicator_icon) {
    char tiles_base_idx = tiles_get_idx();
    char tile_pad_x = WIDTH / 3;
    char tile_pad_y = 32;
    char tile_offset = 18;
    char indicator_offset = 44;
    char text_offset = 8;
    for(char i=0; i< 3; i++) {
        TILE *tile = tile_array->tiles[tiles_base_idx + i];
        badger.image((const uint8_t *)tile->image, Rect(tile_offset + (tile_pad_x*i), tile_pad_y, image_tile_size, image_tile_size));
        badger.graphics->set_pen(0);
        int32_t name_x_offset = (tile_pad_x - badger.graphics->measure_text(tile->name, 2.0f)) /2 ;
        badger.graphics->text(tile->name, Point((tile_pad_x*i) + name_x_offset, tile_pad_y + image_tile_size + text_offset), WIDTH, 2);
        if(!strcmp(name, tile->name)) {
            badger.image((const uint8_t *)indicator_icon, 
                Rect(tile_offset + (tile_pad_x*i) + indicator_offset, tile_pad_y + indicator_offset, image_indicator_size, image_indicator_size));
        }
    }

    for (char i=0; i < tiles_max_column(); i++) {
        badger.graphics->set_pen(0);
        char y = (HEIGHT / 2) - (tiles_max_column() * 10 / 2) + (i * 10);
        badger.graphics->rectangle(Rect(WIDTH - 10, y, 8, 8)); 
        if (tiles_get_column() != i) {
            badger.graphics->set_pen(15);
            badger.graphics->rectangle(Rect(WIDTH - 10 + 1, y + 1, 6, 6)); 
        }
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
    // cyw43_arch_deinit();
    badger.led(0);
    badger.halt();
}


int main() {
    init();
    tiles_make_tiles();
    // External RTC has 1 free byte, we can use this to store the current column and retrieve at reboot
    // This limits the max num of columns to 2^8 - 2, since 0 is being used here to signal that the RTC is not initialised
    tiles_set_column((badger.pcf85063a->get_byte() - 1 >= 0) ? badger.pcf85063a->get_byte() - 1: 0);

    // If External RTC is not initialized or we were woken by an external RTC interrupt
    if(!badger.pcf85063a->get_byte() || badger.pressed_to_wake(badger.RTC)) {
        // Must wait for WiFi to be up before triggering an NTP request
        wifi_wait();
        retrieve_time(true);

        // If we were woken by the RTC alarm, just go back to sleep
        if (badger.pressed_to_wake(badger.RTC)) {
            deinit();
            return 0;
        }
        
        // Set the external RTC free byte so we can later determine if it has been initalized
        badger.pcf85063a->set_byte(1);
    } else {
        retrieve_time(false);
    }

    // If the user pressed up or down to wake we can move the column once for free before the main loop
    if(badger.pressed_to_wake(badger.UP) || badger.pressed_to_wake(badger.DOWN)) {
        if (badger.pressed_to_wake(badger.UP)) {
            tiles_previous_column();
        } else {
            tiles_next_column();
        }
        badger.pcf85063a->set_byte(tiles_get_column()+1);
        badger.graphics->set_pen(15);
        badger.graphics->clear();
    } 
    draw_tiles(NULL, NULL);
    draw_status_bar();
    badger.update();

    alarm_id_t halt_timeout_id = -1;
    while(true) {
        // Wait for button press
        wait_for_button_press_release();
        
        halt_timeout_id = reset_halt_timeout(halt_timeout_id);

        char tiles_base_idx = tiles_get_idx();
        TILE *tile;
        if (badger.pressed(badger.A)) {
            tile = tile_array->tiles[tiles_base_idx];
        } else if(badger.pressed(badger.B)) {
            tile = tile_array->tiles[tiles_base_idx + 1];
        } else if(badger.pressed(badger.C)) {
            tile = tile_array->tiles[tiles_base_idx + 2];
        } else if(badger.pressed(badger.UP) || badger.pressed(badger.DOWN)) {
            if (badger.pressed(badger.UP)) {
                tiles_previous_column();
            } else {
                tiles_next_column();
            }
            badger.pcf85063a->set_byte(tiles_get_column()+1);
            badger.graphics->set_pen(15);
            badger.graphics->clear();
            draw_tiles(NULL, NULL);
            draw_status_bar();
            badger.uc8151->busy_wait();
            badger.update();
            continue;
        } else {
            // How did we get here?
            continue;
        }

        // User triggered a HTTP request, so we must wait for wifi if its not up yet
        wifi_wait();

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