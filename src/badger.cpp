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

void restful_callback(char *value, int status_code, void *arg) {
    if (arg == NULL) {
        return;
    }
    RESTFUL_REQUEST *request = (RESTFUL_REQUEST *)arg;
    DEBUG_printf("restful_callback: value=%s, status_code=%d, caller=%d\n", value, status_code, request->caller);
    if (!strcmp("1", value)) {
        badger.image((const uint8_t *)image_bulb_64x64, Rect(10, 30, 64, 64));
    } else {
        badger.image((const uint8_t *)image_bulb_off_64x64, Rect(10, 30, 64, 64));
    }
    draw_status_bar();
    badger.update();
}

void retrieve_time(datetime_t *datetime) {
    ntp_time_set = false;
    badger.update_button_states();
    // Check if RTC has been initialised previously, if not or if the RTC int is high get the internet time via NTP
    if (!badger.pcf85063a->get_byte() || badger.pressed(badger.RTC)) {
        DEBUG_printf("Retrieving time from NTP\n");
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

bool wifi_up() {
    return cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
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

int main() {
    init();
    while(!wifi_up()) {
        sleep_ms(10);
    }

    draw_status_bar();
    badger.update();
    RESTFUL_ACTION_REQUEST action_request = (RESTFUL_ACTION_REQUEST){
        .method = "POST",
        .endpoint = "/v2/meross",
        .json_body = "{\"hosts\": \"office,hall_up,bedroom,sad,thermometer\", \"code\": \"toggle\"}"
    };
    RESTFUL_STATUS_REQUEST status_request = (RESTFUL_STATUS_REQUEST){
        .request = (RESTFUL_ACTION_REQUEST){
            .method = "POST",
            .endpoint = "/v2/meross",
            .json_body = "{\"hosts\": \"office,hall_up,bedroom,sad,thermometer\", \"code\": \"status\"}"
        },
        .key = "onoff",
    };
    RESTFUL_REQUEST request = (RESTFUL_REQUEST){
        .caller = 1,
        .base_url = API_SERVER,
        .action_request = &action_request,
        .status_request = &status_request,
        .callback = restful_callback
    };
    while(true) {
        badger.wait_for_press();
        if (badger.pressed(badger.A)) {
            // http_request(API_SERVER, "/v2/meross", "POST", "{\"hosts\": \"office,hall_up,bedroom,sad,thermometer\", \"code\": \"status\"}", http_callback, NULL, "onoff");
            restful_request(&request);
        }
    }
    // } else if (badger.pressed_to_wake(badger.B)) {
    //     http_request(API_SERVER, "/v2/meross/office", "POST", "{\"code\": \"toggle\"}", http_callback, NULL, "message");
    // } else if (badger.pressed_to_wake(badger.C)) {
    //     http_request(API_SERVER, "/v2/wol/pc", "POST", "{\"code\": \"status\"}", http_callback, NULL, "data");
    // }
    deinit();
    return 0;
}