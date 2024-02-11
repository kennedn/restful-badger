#pragma once

#include "restful.h"

#define TILES_MAX 254

typedef struct TILE_ {
    char *name;
    char *image;
    char *status_on;
    char *status_off;
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

void tiles_init(const char *base_url, char size);
void tiles_free();
void tiles_add_tile(const char *name, const char *image, const char *status_on, const char *status_off, void *action_request, void *status_request);
char tiles_max_column();
void tiles_previous_column();
void tiles_next_column();
char tiles_get_column();
void tiles_set_column(char column);
char tiles_get_idx();
void tiles_make_tiles();