#include "tiles.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "badger.h"
#include "images.h"
#include "restful.h"
#include "pico/time.h"

#define TILES_MAX 255

TILES *tile_array;
HEADING **heading_array;
char heading_count;
char tile_column;
static const char tiles_data[] PROGMEM = {
    TILES_DATA};

void tiles_init(const char *base_url, char size) {
    tile_array = (TILES *)malloc(sizeof(TILES));
    tile_array->base_url = (char *)malloc(strlen(base_url) + 1 * sizeof(char));
    strcpy(tile_array->base_url, base_url);
    tile_array->tiles = malloc(size * sizeof(TILE *));
    tile_array->used = 0;
    tile_array->size = size;
}

void tiles_free() {
    uint64_t sw_time = to_us_since_boot(get_absolute_time());
    if (!tile_array) {
        return;
    }

    for (int i = 0; i < tile_array->used; i++) {
        free(tile_array->tiles[i]->name);
        restful_free_request_data(tile_array->tiles[i]->action_request);
        restful_free_request_data(tile_array->tiles[i]->status_request);
        free(tile_array->tiles[i]);
    }
    free(tile_array->tiles);
    free(tile_array);
    tile_array = NULL;

    if (!heading_array) {
        return;
    }

    for (int i = 0; i < heading_count; i++) {
        free(heading_array[i]->heading);
        free(heading_array[i]);
    }
    free(heading_array);
    heading_array = NULL;
    DEBUG_printf("tiles_free: %ld us\n", (long)(to_us_since_boot(get_absolute_time()) - sw_time));
}

void tiles_add_tile(char *name, char image_idx, void *action_request, void *status_request) {
    TILE *tile = (TILE *)malloc(sizeof(TILE));

    tile->name = name;

    tile->image = (char *)image_tiles[image_idx];
    tile->action_request = action_request;
    tile->status_request = status_request;

    if (!tile_array) {
        free(tile);
        return;
    }

    if (tile_array->used >= TILES_MAX) {
        DEBUG_printf("Hit TILES_MAX(%d), skipping tile\n", TILES_MAX);
        free(tile);
        return;
    }
    if (tile_array->size < TILES_MAX && tile_array->used == tile_array->size) {
        tile_array->size = (tile_array->size * 2 > TILES_MAX) ? TILES_MAX : tile_array->size * 2;
        tile_array->tiles = realloc(tile_array->tiles, tile_array->size * sizeof(TILE *));
    }

    tile_array->tiles[tile_array->used++] = tile;
}

char tiles_max_column() {
    if (tile_array->used % 3 != 0) {
        return (tile_array->used + 3) / 3;
    } else {
        return tile_array->used / 3;
    }
}

void tiles_previous_column(char count) {
    tile_column = (tile_column + tiles_max_column() - count) % tiles_max_column();
}

void tiles_next_column(char count) {
    tile_column = (tile_column + count) % tiles_max_column();
}

char tiles_get_column() {
    return tile_column;
}

HEADING *tiles_get_heading() {
    return heading_array[tile_column];
}

void tiles_set_column(char column) {
    tile_column = 0;
    if (column < tiles_max_column()) {
        tile_column = column;
    }
}

char tiles_get_base_idx() {
    return (tile_column * 3 > tile_array->used) ? tile_array->used : tile_column * 3;
}

char tiles_idx_in_bounds(char idx) {
    return idx < tile_array->used;
}

char tiles_make_str(char **dest, char str_size, const char *src) {
    *dest = (char *)malloc(str_size * sizeof(char) + 1);
    strncpy(*dest, src, str_size);
    (*dest)[str_size] = '\0';  // Null terminate
    return str_size;
}

void tiles_make_tiles() {
    uint64_t sw_time = to_us_since_boot(get_absolute_time());
    tiles_init(API_SERVER, 8);

    RESTFUL_REQUEST_DATA *action_request;
    RESTFUL_REQUEST_DATA *status_request;
    HEADING *heading;
    char *name;
    char *method;
    char *endpoint;
    char *json_body;
    char *key;
    char *on_value;
    char *off_value;
    int ptr = 0;

    heading_count = tiles_data[ptr++];
    heading_array = (HEADING **)malloc(heading_count * sizeof(HEADING *));
    for (char i = 0; i < heading_count; i++) {
        heading = (HEADING *)malloc(sizeof(HEADING));
        char str_size = tiles_data[ptr++];

        if (str_size == 0) {
            // This is a padding tile
            heading->heading = NULL;
            heading->icon = 0;
            heading_array[i] = heading;
            continue;
        }

        ptr += tiles_make_str(&(heading->heading), str_size, (char *)&tiles_data[ptr]);
        heading->icon = (char *)image_icons[tiles_data[ptr++]];
        heading_array[i] = heading;
    }

    char tile_count = tiles_data[ptr++];
    for (char i = 0; i < tile_count; i++) {
        char str_size = tiles_data[ptr++];

        if (str_size == 0) {
            // This is a padding tile
            tiles_add_tile(NULL, 0, NULL, NULL);
            continue;
        }
        ptr += tiles_make_str(&name, str_size, (char *)&tiles_data[ptr]);

        char image_idx = tiles_data[ptr++];
        char request_mask = tiles_data[ptr++];

        // Action request present
        action_request = NULL;
        if (request_mask & 1) {
            ptr += tiles_make_str(&method, tiles_data[ptr++], (char *)&tiles_data[ptr]);
            ptr += tiles_make_str(&endpoint, tiles_data[ptr++], (char *)&tiles_data[ptr]);
            ptr += tiles_make_str(&json_body, tiles_data[ptr++], (char *)&tiles_data[ptr]);

            action_request = restful_make_request_data(
                method,
                endpoint,
                json_body,
                NULL,
                NULL,
                NULL);
        }
        // Status request present
        status_request = NULL;
        if (request_mask & 2) {
            ptr += tiles_make_str(&method, tiles_data[ptr++], (char *)&tiles_data[ptr]);
            ptr += tiles_make_str(&endpoint, tiles_data[ptr++], (char *)&tiles_data[ptr]);
            ptr += tiles_make_str(&json_body, tiles_data[ptr++], (char *)&tiles_data[ptr]);
            ptr += tiles_make_str(&key, tiles_data[ptr++], (char *)&tiles_data[ptr]);
            ptr += tiles_make_str(&on_value, tiles_data[ptr++], (char *)&tiles_data[ptr]);
            ptr += tiles_make_str(&off_value, tiles_data[ptr++], (char *)&tiles_data[ptr]);

            status_request = restful_make_request_data(
                method,
                endpoint,
                json_body,
                key,
                on_value,
                off_value);
        }

        tiles_add_tile(name, image_idx, action_request, status_request);
    }
    DEBUG_printf("tiles_make_tiles: %ld us\n", (long)(to_us_since_boot(get_absolute_time()) - sw_time));
}