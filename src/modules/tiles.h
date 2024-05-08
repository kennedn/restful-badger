#pragma once

#include "restful.h"

#ifndef PROGMEM
#define memcpy_P memcpy
#define PROGMEM
#endif

typedef struct HEADING_ {
    char *heading;
    char *icon;
} HEADING;

typedef struct TILE_ {
    char *name;
    char *image;
    void *action_request;
    void *status_request;
} TILE;

typedef struct TILES_ {
    char *base_url;
    TILE **tiles;
    int used;
    int size;
} TILES;

extern TILES *tile_array;
extern HEADING **heading_array;

void tiles_init(const char *base_url, char size);
void tiles_free();
char tiles_max_column();
void tiles_previous_column(char count);
void tiles_next_column(char count);
char tiles_get_column();
void tiles_set_column(char column);
HEADING *tiles_get_heading();
char tiles_get_base_idx();
char tiles_idx_in_bounds(char idx);
void tiles_make_tiles();