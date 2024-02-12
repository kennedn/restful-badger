#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tiles.h"
#include "restful.h"
#include "images.h"
#include "badger.h"

TILES *tile_array;
char tile_column;

void tiles_init(const char *base_url, char size) {
    tile_array = (TILES *)malloc(sizeof(TILES));
    tile_array->base_url = (char *)malloc(strlen(base_url) + 1 * sizeof(char));
    strcpy(tile_array->base_url, base_url);
    tile_array->tiles = malloc(size * sizeof(TILE*));
    tile_array->used = 0;
    tile_array->size = size;
}

void tiles_free() {
    if(!tile_array) {
        return;
    }

    for(int i=0; i <tile_array->used; i++) {
        free(tile_array->tiles[i]->name);
        free(tile_array->tiles[i]->status_on);
        free(tile_array->tiles[i]->status_off);
        restful_free_request_data(tile_array->tiles[i]->action_request);
        restful_free_request_data(tile_array->tiles[i]->status_request);
        free(tile_array->tiles[i]);
    }
    free(tile_array->tiles);
    free(tile_array);
    tile_array = NULL;
}

void tiles_add_tile(const char *name, const char *image, const char *status_on, const char *status_off, void *action_request, void *status_request) {
    TILE *tile = (TILE *)malloc(sizeof(TILE));

    tile->name = (char *)malloc(strlen(name) + 1 * sizeof(char));
    strcpy(tile->name, name);
    tile->status_on = (char *)malloc(strlen(status_on) + 1 * sizeof(char));
    strcpy(tile->status_on, status_on);
    tile->status_off = (char *)malloc(strlen(status_off) + 1 * sizeof(char));
    strcpy(tile->status_off, status_off);

    tile->image = (char *)image;
    tile->action_request = action_request;
    tile->status_request = status_request;

    if(!tile_array) {
        free(tile);
        return;
    }

    if(tile_array->used >= TILES_MAX) {
        DEBUG_printf("Hit TILES_MAX(%d), skipping tile", TILES_MAX);
        free(tile);
        return;
    }
    if (tile_array->size < TILES_MAX && tile_array->used == tile_array->size) {
        tile_array->size = (tile_array->size *2 > TILES_MAX) ? TILES_MAX : tile_array->size *2;
        tile_array->tiles = realloc(tile_array->tiles, tile_array->size * sizeof(TILE*));
    }
  
    tile_array->tiles[tile_array->used++] = tile;

}

char tiles_max_column() {
    return tile_array->used / 3;
}

void tiles_previous_column() {
    tile_column = (tile_column + tiles_max_column() - 1) % tiles_max_column();
}

void tiles_next_column() {
    tile_column = (tile_column + 1) % tiles_max_column();
}

char tiles_get_column() {
    return tile_column;
}

void tiles_set_column(char column) {
    tile_column = column;
}

char tiles_get_idx() {
    return (tile_column * 3 > tile_array->used) ? tile_array->used : tile_column * 3;
}

void tiles_make_tiles() {
    tiles_init(API_SERVER, 8);
    RESTFUL_REQUEST_DATA *action_request;
    RESTFUL_REQUEST_DATA *status_request;

    action_request = restful_make_request_data(
        "POST",
        "/v2/meross/office",
        "{\"code\": \"toggle\"}",
        NULL
    );
    status_request = restful_make_request_data(
        "POST",
        "/v2/meross/office",
        "{\"code\": \"status\"}",
        "onoff"
    );
    tiles_add_tile(
        "office",
        image_tile_bulb,
        "1",
        "0",
        action_request,
        status_request
    );

    action_request = restful_make_request_data(
        "POST",
        "/v2/meross/bedroom",
        "{\"code\": \"toggle\"}",
        NULL
    );
    status_request = restful_make_request_data(
        "POST",
        "/v2/meross/bedroom",
        "{\"code\": \"status\"}",
        "onoff"
    );
    tiles_add_tile(
        "bedroom",
        image_tile_bulb,
        "1",
        "0",
        action_request,
        status_request
    );

    action_request = restful_make_request_data(
        "POST",
        "/v2/meross/hall_up",
        "{\"code\": \"toggle\"}",
        NULL
    );
    status_request = restful_make_request_data(
        "POST",
        "/v2/meross/hall_up",
        "{\"code\": \"status\"}",
        "onoff"
    );
    tiles_add_tile(
        "hall up",
        image_tile_bulb,
        "1",
        "0",
        action_request,
        status_request
    );

    action_request = restful_make_request_data(
        "POST",
        "/v2/meross/thermometer",
        "{\"code\": \"toggle\"}",
        NULL
    );
    status_request = restful_make_request_data(
        "POST",
        "/v2/meross/thermometer",
        "{\"code\": \"status\"}",
        "onoff"
    );
    tiles_add_tile(
        "thermo",
        image_tile_bulb,
        "1",
        "0",
        action_request,
        status_request
    );

    action_request = restful_make_request_data(
        "POST",
        "/v2/meross/sad",
        "{\"code\": \"toggle\"}",
        NULL
    );
    status_request = restful_make_request_data(
        "POST",
        "/v2/meross/sad",
        "{\"code\": \"status\"}",
        "onoff"
    );
    tiles_add_tile(
        "sad",
        image_tile_bulb,
        "1",
        "0",
        action_request,
        status_request
    );

    action_request = restful_make_request_data(
        "POST",
        "/v2/meross/tree2",
        "{\"code\": \"toggle\"}",
        NULL
    );
    tiles_add_tile(
        "tree",
        image_tile_flame,
        "1",
        "0",
        action_request,
        NULL
    );
}