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
        restful_free_request_data(tile_array->tiles[i]->action_request);
        restful_free_request_data(tile_array->tiles[i]->status_request);
        free(tile_array->tiles[i]);
    }
    free(tile_array->tiles);
    free(tile_array);
    tile_array = NULL;
}

void tiles_add_tile(char *name, char image_idx, void *action_request, void *status_request) {
    if (!name) {
        DEBUG_printf("tiles_add_tile: invalid parameters");
        return;
    }
    TILE *tile = (TILE *)malloc(sizeof(TILE));

    tile->name = name;

    tile->image = (char *)image_tiles[image_idx];
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

void tiles_make_str(char **dest, const char *src, char str_size) {
    *dest = (char *)malloc(str_size * sizeof(char) + 1);
    strncpy(*dest, src, str_size);
    (*dest)[str_size] = '\0'; // Null terminate
}

void tiles_make_tiles() {
    tiles_init(API_SERVER, 8);

    RESTFUL_REQUEST_DATA *action_request;
    RESTFUL_REQUEST_DATA *status_request;
    char *name;
    char *method;
    char *endpoint;
    char *json_body;
    char *key;
    char *on_value;
    char *off_value;
    char str_size = 0;
    char i = 0;
    int ptr = 0;
    char tile_count = tiles_data[ptr++];

    while(i < tile_count) {
        char image_idx = tiles_data[ptr++];

        str_size = tiles_data[ptr++];
        tiles_make_str(&name, (char *)&tiles_data[ptr], str_size);
        ptr += str_size;

        char request_mask = tiles_data[ptr++];
        // Action request present
        if (request_mask & 1) { 
            str_size = tiles_data[ptr++];
            tiles_make_str(&method, (char *)&tiles_data[ptr], str_size);
            ptr +=str_size;

            str_size = tiles_data[ptr++];
            tiles_make_str(&endpoint, (char *)&tiles_data[ptr], str_size);
            ptr +=str_size;

            str_size = tiles_data[ptr++];
            tiles_make_str(&json_body, (char *)&tiles_data[ptr], str_size);
            ptr +=str_size;
            
            action_request = restful_make_request_data(
                method,
                endpoint,
                json_body,
                NULL,
                NULL,
                NULL
            );
        }
        // Status request present
        if (request_mask & 2) { 
            str_size = tiles_data[ptr++];
            tiles_make_str(&method, (char *)&tiles_data[ptr], str_size);
            ptr +=str_size;

            str_size = tiles_data[ptr++];
            tiles_make_str(&endpoint, (char *)&tiles_data[ptr], str_size);
            ptr +=str_size;

            str_size = tiles_data[ptr++];
            tiles_make_str(&json_body, (char *)&tiles_data[ptr], str_size);
            ptr +=str_size;

            str_size = tiles_data[ptr++];
            tiles_make_str(&key, (char *)&tiles_data[ptr], str_size);
            ptr +=str_size;

            str_size = tiles_data[ptr++];
            tiles_make_str(&on_value, (char *)&tiles_data[ptr], str_size);
            ptr +=str_size;

            str_size = tiles_data[ptr++];
            tiles_make_str(&off_value, (char *)&tiles_data[ptr], str_size);
            ptr +=str_size;

            status_request = restful_make_request_data(
                method,
                endpoint,
                json_body,
                key,
                on_value,
                off_value 
            );
        }

        tiles_add_tile(name, image_idx, action_request, status_request);

        i++;
    }
}