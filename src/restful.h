#pragma once

#include "tiles.h"

typedef void (*restful_callback_t)(char *value, int response_code, void *arg);

typedef struct RESTFUL_REQUEST_DATA_ {
    char *method;
    char *endpoint;
    char *json_body;
    char *key;
} RESTFUL_REQUEST_DATA;

typedef struct RESTFUL_REQUEST_ {
    void *tile;
    char *base_url;
    RESTFUL_REQUEST_DATA *action_request;
    RESTFUL_REQUEST_DATA *status_request;
    restful_callback_t callback;
} RESTFUL_REQUEST;

void restful_request(RESTFUL_REQUEST *request);
RESTFUL_REQUEST *restful_make_request(void *tile, const char *base_url, RESTFUL_REQUEST_DATA *action_request, RESTFUL_REQUEST_DATA *status_request, restful_callback_t callback);
RESTFUL_REQUEST_DATA *restful_make_request_data(const char *method, const char *endpoint, const char *json_body, const char *key);
void restful_free_request(RESTFUL_REQUEST *request);
void restful_free_request_data(RESTFUL_REQUEST_DATA *request);