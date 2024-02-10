#pragma once

#ifndef PROGMEM
#define memcpy_P memcpy
#define PROGMEM
#endif

const char image_wifi_icon[32] PROGMEM = {
    0x00, 0x00, 0x07, 0xe0, 0x1f, 0xf8, 0x3f, 0xfc, 0x70, 0x0e, 0x67, 0xe6, 0x0f, 0xf0, 0x18, 0x18, 0x03, 0xc0, 0x07, 0xe0, 0x04, 0x20, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const char image_no_wifi_icon[32] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static const char image_battery_charging[32] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x80, 0x02, 0xbf, 0xfa, 0xbd, 0xfb, 0xb8, 0x9b, 0xb2, 0x3b, 0xbf, 0x7b, 0xbf, 0xfa, 0x80, 0x02, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static const char image_battery[32] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x80, 0x02, 0x80, 0x02, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x02, 0x80, 0x02, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static const char image_bulb_64x64[512] PROGMEM = {
    0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xfe, 0x00, 0xff, 0xff, 0xff, 0xfe, 0x01, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xfc, 0x03, 0xff, 0xff, 0x80, 0x7f, 0xff, 0xff, 0xf8, 0x07, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xf8, 0x07, 0xff, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xfc, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xfc, 0x07, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xf8, 0x07, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xe0, 0x0f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xfc, 0x01, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xff, 0xfe, 0x00, 0xff, 0xfe, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x7f, 0xfe, 0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 0x7f, 0xfc, 0x03, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x1f, 0xf8, 0x07, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x1f, 0xf8, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x1f, 0xf8, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x1f, 0xf8, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x1f, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x1f, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x1f, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x1f, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x15, 0x50, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd2, 0x4f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static const char image_bulb_off_64x64[512] PROGMEM = {
    0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};