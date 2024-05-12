#include <stdio.h>
#include <stdlib.h>

#include "pico/cyw43_arch.h"
#include "pico/platform.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/util/datetime.h"

extern "C" {
#include "modules/http.h"
#include "modules/images.h"
#include "modules/ntp.h"
#include "modules/power.h"
#include "modules/restful.h"
#include "modules/tiles.h"
#include "modules/piezo.h"
}

#include "badger.h"
#include "libraries/badger2040w/badger2040w.hpp"

#define WIFI_STATUS_POLL_MS 3000
#define WIDTH 296
#define HEIGHT 128

using namespace pimoroni;

Badger2040W badger;

// 12am daily alarm
datetime_t ntp_daily_alarm = datetime_t{
    .day = -1,
    .hour = 00,
    .min = 00,
    .sec = 00};

int request_buttons[] = {badger.A, badger.B, badger.C};

static volatile bool ntp_time_set = false;
static volatile bool halt_initiated = false;
static volatile bool initialised = false;

void draw_status_bar();
void draw_status_bar(bool sleep);
void draw_tiles(const char *name, const char *indicator_icon);
void deinit(bool update_display=false);

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
        draw_tiles(tile->name, image_indicator_question);
    } else if (status_code == 200 && !value) {
        draw_tiles(tile->name, image_indicator_tick);
    }

    DEBUG_printf("restful_callback: value=%s, status_code=%d, caller=%s\n", value ? value : "NULL", status_code, tile->name);
    if (value && !strcmp(request->status_request->on_value, value)) {
        draw_tiles(tile->name, image_indicator_tick);
        // piezo_play(piezo_notes_pong, piezo_notes_pong_len);
    } else if(value && !strcmp(request->status_request->off_value, value)) {
        draw_tiles(tile->name, image_indicator_cross);
        // piezo_play(piezo_notes_pong_2, piezo_notes_pong_2_len);
    } else if(value) {
        draw_tiles(tile->name, image_indicator_question);
    }
    restful_free_request(request);
    draw_status_bar();
    if (initialised) {
        badger.uc8151->set_update_speed(3);
    }

    badger.update();
    badger.uc8151->busy_wait();
    initialised = true;
    badger.uc8151->set_update_speed(2);
}

int64_t halt_timeout_callback(alarm_id_t id, void *arg) {
    halt_initiated = true;
    return 0;
}


bool datetime_is_sane(datetime_t *datetime) {
    if (datetime->year < 0 || datetime->month > 4095) { return false; }
    if (datetime->month < 1 || datetime->day > 12) { return false; }
    if (datetime->day < 1 || datetime->day > 31) { return false; }
    if (datetime->dotw < 0 || datetime->dotw > 6) { return false; }
    if (datetime->hour < 0 || datetime->hour > 23) { return false; }
    if (datetime->min < 0 || datetime->min > 59) { return false; }
    if (datetime->sec < 0 || datetime->sec > 59) { return false; }
    return true;
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
        if (!datetime_is_sane(&datetime)) {
            DEBUG_printf("RTC time is insane, triggering NTP retrieval\n");
            retrieve_time(true);
            return;
        }
        // Store time in internal RTC
        rtc_set_datetime(&datetime);
    }
}

alarm_id_t rearm_halt_timeout(alarm_id_t id) {
    if (id != -1) {
        cancel_alarm(id);
    }
    return add_alarm_in_ms(HALT_TIMEOUT_MS, halt_timeout_callback, NULL, false);
}


void wifi_connect_async() {
    // Use cyw43_wifi_join so that wifi channel can be specified. This shaves ~700ms of connection time, static IP / disable DNS in lwipopts.h shaves ~800ms too
    cyw43_wifi_join(&cyw43_state, strlen(WIFI_SSID), (const uint8_t *)WIFI_SSID, strlen(WIFI_PASSWORD),
                    (const uint8_t *)WIFI_PASSWORD, CYW43_AUTH_WPA2_MIXED_PSK, WIFI_BSSID, WIFI_CHANNEL);
}

bool wifi_up() {
    return cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
}

void wifi_wait() {
    uint32_t sw_timer = 0;
    while(!wifi_up()) {
        if (halt_initiated) {
            deinit(initialised);
        }
        // Retry wifi connect if link status is in error
        if ((sw_timer == 0 || (to_ms_since_boot(get_absolute_time()) - sw_timer) > WIFI_STATUS_POLL_MS) && cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) <= 0) {
            wifi_connect_async();
            sw_timer = to_ms_since_boot(get_absolute_time());
        }
    }
}

char wait_for_button_press_release() {
    uint32_t mask = (1UL << badger.A) | (1UL << badger.B) | (1UL << badger.C) | (1UL << badger.UP) | (1UL << badger.DOWN);
    uint32_t sw_timer = to_ms_since_boot(get_absolute_time());
    char counter = 0;
    while (true) {
        // Wait for button press
        while(!(gpio_get_all() & mask)) {
            // timer callback has asked for a halt
            if (halt_initiated) {
                deinit(initialised);
            }
            // Allow a grace period before returning to record subsequent clicks
            if (counter > 0 && (to_ms_since_boot(get_absolute_time()) - sw_timer) > MULTI_CLICK_WAIT_MS) {
                return counter;
            }
        }
        sw_timer = to_ms_since_boot(get_absolute_time());
        counter++;
        badger.update_button_states();
        badger.led(0);

        // Wait for button release
        while((gpio_get_all() & mask)) {
            tight_loop_contents();
        }

        badger.led(255);
        sleep_ms(80);    // debounce
    }
}

void draw_status_bar() {
    draw_status_bar(false);
}

void draw_status_bar(bool sleep) {
    datetime_t datetime;
    float voltage;
    char powerPercent;
    char percentStr[5];
    char clock_str[12];
    char *heading_str;
    char status_x_offset = 0;
    char x_pad = 4;
    int32_t clock_x_offset;
    uint8_t *wifi_image = (uint8_t *)image_status_wifi_on;
    uint8_t *battery_image = (uint8_t *)image_status_battery_charging;
    uint8_t *heading_icon = NULL;
    if (!power_is_charging()) {
        battery_image = (uint8_t *)image_status_battery_discharging;
        status_x_offset = 22;

        power_voltage(&voltage);
        powerPercent = power_percent(&voltage);
        sprintf(percentStr, "%d%%", powerPercent);
    }

    if(sleep) {
        wifi_image = (uint8_t *)image_status_sleeping;
        sprintf(clock_str, "Sleeping");
    } else {
        if(!wifi_up()) {
            wifi_image = (uint8_t *)image_status_wifi_off;
        }
        rtc_get_datetime(&datetime);
        sprintf(clock_str, "%02d:%02d\n", datetime.hour, datetime.min);
        heading_icon = (uint8_t *)tiles_get_heading()->icon;
    }
    clock_x_offset = badger.graphics->measure_text(clock_str, 2.0f) / 2;

    badger.graphics->set_pen(0);
    badger.graphics->rectangle(Rect(0, 0, WIDTH, 20));

    badger.graphics->set_pen(15);
    badger.graphics->text(clock_str, Point(WIDTH / 2 - clock_x_offset, 3), WIDTH, 2.0f);

    if (heading_icon) {
        badger.graphics->text(tiles_get_heading()->heading, Point(image_status_size + x_pad * 2, 2), WIDTH, 2.0f);
        badger.image(heading_icon, Rect(x_pad, 2, image_status_size, image_status_size));
    } else {
        badger.graphics->text("restfulBadger", Point(x_pad, 6), WIDTH, 1.0f);
    }
    badger.image(battery_image, Rect(WIDTH - status_x_offset - (image_status_size + x_pad), 2, image_status_size, image_status_size));
    badger.image(wifi_image, Rect(WIDTH - status_x_offset - (image_status_size + x_pad) * 2, 2, image_status_size, image_status_size));
    if (!power_is_charging()) {
        badger.graphics->set_pen(15);
        badger.graphics->text(percentStr, Point(WIDTH - status_x_offset, 6), WIDTH, 1.0);
        badger.graphics->rectangle(Rect(WIDTH - status_x_offset - (image_status_size + x_pad) + 2, 7, powerPercent / 10 + 1, 6));
    }
}

void draw_tiles(const char *name, const char *indicator_icon) {
    char tiles_base_idx = tiles_get_base_idx();
    char tile_pad_x = WIDTH / 3;
    char tile_pad_y = 26;
    char tile_offset = 18;
    char column_pad_y = 10;
    char indicator_offset = 44;
    char text_offset = 10;
    for(char i=0; i< 3; i++) {
        if (!tiles_idx_in_bounds(tiles_base_idx + i)) {
            break;
        } 
        TILE *tile = tile_array->tiles[tiles_base_idx + i];
        if (!tile->image || !tile->name) { // Probably a padding tile
            continue;
        }
        badger.image((const uint8_t *)tile->image, Rect(tile_offset + (tile_pad_x*i), tile_pad_y, image_tile_size, image_tile_size));
        badger.graphics->set_pen(0);


        // Determine the true width of the string if a wrap point exists
        int32_t name_size = badger.graphics->measure_text(tile->name, 2.0f);
        int32_t prev_name_size = name_size;
        if (name_size > tile_pad_x) {
            char *space_ptr = tile->name;
            char has_wrap = false;
            while (true) {
                space_ptr = strchr(space_ptr, ' ');
                if (space_ptr == NULL) {
                    break;
                }
                has_wrap = true;
                *space_ptr = '\0';
                name_size = badger.graphics->measure_text(tile->name, 2.0f);
                *space_ptr = ' ';
                space_ptr++;    // Move past space character for next iteration
                if (name_size > tile_pad_x) {
                    name_size = prev_name_size;
                    break;
                }
                prev_name_size = name_size;
            }

            if (has_wrap) {
                text_offset = 4;    // Correct y offset for multiple lines
            }
        }
        int32_t name_x_offset = (tile_pad_x - name_size) /2 ;

        badger.graphics->text(tile->name, Point((tile_pad_x*i) + name_x_offset, tile_pad_y + image_tile_size + text_offset), tile_pad_x, 2);
        if(!strcmp(name, tile->name)) {
            badger.image((const uint8_t *)indicator_icon, 
                Rect(tile_offset + (tile_pad_x*i) + indicator_offset, tile_pad_y + indicator_offset, image_indicator_size, image_indicator_size));
        }
    }

    DEBUG_printf("current column: %d, max: %d\n", tiles_get_column(), tiles_max_column());
    // Can only fit 10 squares on screen at a time, so if max columns exceeds this we must split the display up into sections of 10
    char current_col = tiles_get_column() % 10;
    char last_10s_col = tiles_max_column() - tiles_max_column() % 10;
    char max_col = (tiles_get_column() < last_10s_col) ? 10 : tiles_max_column() % 10;
    for (char i=0; i < max_col; i++) {
        badger.graphics->set_pen(0);
        char y = column_pad_y + (HEIGHT / 2) - (max_col * 10 / 2) + (i * 10);
        badger.graphics->rectangle(Rect(WIDTH - 10, y, 20, 8)); 
        if (current_col != i) {
            badger.graphics->set_pen(15);
            badger.graphics->rectangle(Rect(WIDTH - 10 + 1, y + 1, 18, 6)); 
        }
    }
}

void init() {
    badger.init();
    badger.led(255);

    adc_init();
    rtc_init();

    if (cyw43_arch_init()) {
        DEBUG_printf("failed to initialise\n");
        deinit(true);
    }
    cyw43_arch_enable_sta_mode();

    wifi_connect_async();

    badger.graphics->set_font("bitmap8");
    badger.uc8151->set_update_speed(2);
    badger.graphics->set_thickness(2);

    badger.graphics->set_pen(15);
    badger.graphics->clear();

    // External RTC has 1 free byte, we can use this to store the current column and retrieve at reboot
    // This limits the max num of columns to 2^8 - 2, since 0 is being used here to signal that the RTC is not initialised
    uint8_t rtc_byte = badger.pcf85063a->get_byte();
    tiles_set_column((rtc_byte - 1 >= 0) ? rtc_byte - 1: 0);

    // If External RTC is not initialized or we were woken by an external RTC interrupt
    if(!rtc_byte || badger.pressed_to_wake(badger.RTC)) {
        // Must wait for WiFi to be up before triggering an NTP request
        wifi_wait();
        retrieve_time(true);

        // If we were woken by the RTC alarm, just go back to sleep
        if (badger.pressed_to_wake(badger.RTC)) {
            deinit(false);
        }
        
        // Set the external RTC free byte so we can later determine if it has been initalized
        badger.pcf85063a->set_byte(1);
    } else {
        retrieve_time(false);
    }
}

void deinit(bool update_display) {
    DEBUG_printf("Going to sleep zZzZ\n");
    if (update_display) {
        draw_status_bar(true);
        draw_tiles(NULL, NULL);
        badger.update();
        badger.uc8151->busy_wait();
    }
    if(initialised) {
        cyw43_arch_deinit();
    }
    tiles_free();
    badger.led(0);
    badger.halt();
}


int main() {
    // Calling the full init here is too slow if we want to catch button presses from wake, so instead
    // Only configure crucial stuff for now e.g Enable_3v3, LED & check RTC

    stdio_init_all();
    gpio_set_function(badger.ENABLE_3V3, GPIO_FUNC_SIO);
    gpio_set_dir(badger.ENABLE_3V3, GPIO_OUT);
    gpio_put(badger.ENABLE_3V3, 1);
    pwm_config cfg = pwm_get_default_config();
    pwm_set_wrap(pwm_gpio_to_slice_num(badger.LED), 65535);
    pwm_init(pwm_gpio_to_slice_num(badger.LED), &cfg, true);
    gpio_set_function(badger.LED, GPIO_FUNC_PWM);

    gpio_set_function(badger.RTC, GPIO_FUNC_SIO);
    gpio_set_dir(badger.RTC, GPIO_IN);
    gpio_set_pulls(badger.RTC, false, true);
    if (gpio_get_all() & 1 << badger.RTC) {
        init();
    }

    alarm_id_t halt_timeout_id = -1;
    while(true) {
        halt_timeout_id = rearm_halt_timeout(halt_timeout_id);
        // Wait for button press
        char click_count = wait_for_button_press_release();

        if (!initialised) {
            piezo_play(piezo_notes_test_4, piezo_notes_test_4_len, false);
            init();
            tiles_make_tiles();
        }

        char tiles_base_idx = tiles_get_base_idx();

        TILE *tile = NULL; 
        if(badger.pressed(badger.UP) || badger.pressed(badger.DOWN)) {
            if (badger.pressed(badger.UP)) {
                tiles_previous_column(click_count);
            } else {
                tiles_next_column(click_count);
            }
            badger.pcf85063a->set_byte(tiles_get_column()+1);
            badger.graphics->set_pen(15);
            badger.graphics->clear();
            draw_tiles(NULL, NULL);
            draw_status_bar();
            badger.update();
            initialised = true;
        } else {
            for(int i=0; i <  count_of(request_buttons); i++) {
                if (!badger.pressed(request_buttons[i])) {
                    continue;
                }
                if (tiles_idx_in_bounds(tiles_base_idx + i)) {
                    tile = tile_array->tiles[tiles_base_idx + i];
                } 
            }
        }

        if (!tile) {
            continue;
        }

        // User triggered a HTTP request, so we must wait for wifi if its not up yet
        if (!wifi_up()) {
            if (!initialised) {
                piezo_play(piezo_notes_test_3, piezo_notes_test_3_len, true);
            }
            wifi_wait();
            piezo_stop();
        }

        restful_request(restful_make_request(
            tile,
            tile_array->base_url,
            (RESTFUL_REQUEST_DATA *)(tile->action_request),
            (RESTFUL_REQUEST_DATA *)(tile->status_request),
            restful_callback)
        );

    }
    return 0;
}
