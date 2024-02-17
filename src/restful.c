#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "restful.h"
#include "tiles.h"
#include "badger.h"

void restful_callback(char *value, int response_code, void *arg) {
    RESTFUL_REQUEST *request = (RESTFUL_REQUEST *)arg;
    if (response_code != 200 || request->status_request == NULL) {
        request->callback(value, response_code, arg);
        return;
    }
    RESTFUL_REQUEST_DATA *status_request = request->status_request;
    http_request(request->base_url, status_request->endpoint, status_request->method, 
        status_request->json_body, status_request->key, request->callback, request);
}

void restful_request(RESTFUL_REQUEST *request) {
    RESTFUL_REQUEST_DATA *sub_request = request->action_request;
    void *callback = restful_callback;
    char *key = NULL;

    if (sub_request == NULL && request->status_request != NULL) {
        sub_request = request->status_request;
        callback = request->callback;
        key = sub_request->key;
    } else if(sub_request == NULL) {
        DEBUG_printf("No requests present, nothing to do\n");
        return;
    }

    http_request(request->base_url, sub_request->endpoint, sub_request->method, sub_request->json_body, key, callback, request);
}

RESTFUL_REQUEST *restful_make_request(void *tile, const char *base_url, RESTFUL_REQUEST_DATA *action_request, RESTFUL_REQUEST_DATA *status_request, restful_callback_t callback) {
    if (!base_url || !callback || (!action_request && !status_request)) {
        DEBUG_printf("Required data in request was NULL\n");
        return NULL;
    }
    RESTFUL_REQUEST *request = (RESTFUL_REQUEST *)malloc(sizeof(RESTFUL_REQUEST));
    request->tile = tile;
    request->base_url = (char*) malloc(strlen(base_url) + 1 * sizeof(char));
    strcpy(request->base_url, base_url);
    request->action_request = action_request;
    request->status_request = status_request;
    request->callback = callback;

}

void restful_free_request(RESTFUL_REQUEST *request) {
    free(request->base_url);
    free(request);
}

RESTFUL_REQUEST_DATA *restful_make_request_data(char *method, char *endpoint, char *json_body, char *key, char *on_value, char *off_value) {
    if (!method || !endpoint || !json_body) {
        DEBUG_printf("Required data in request data was NULL\n");
        return NULL;
    }
    if (key && (!on_value || !off_value)) {
        DEBUG_printf("On and Off values missing despite key being specified\n");
        return NULL;
    }
    RESTFUL_REQUEST_DATA *request_data = (RESTFUL_REQUEST_DATA *)malloc(sizeof(RESTFUL_REQUEST_DATA));
    request_data->method = method;
    request_data->endpoint = endpoint;
    request_data->json_body = json_body;
    request_data->key = NULL;
    if(key) {
        request_data->key = key;
        request_data->on_value = on_value;
        request_data->off_value = off_value;
    } 

    return request_data;
 }

void restful_free_request_data(RESTFUL_REQUEST_DATA *request) {
    free(request->method);
    free(request->endpoint);
    free(request->json_body);
    if (request->key) {
        free(request->key);
        free(request->on_value);
        free(request->off_value);
    }
}