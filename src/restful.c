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
        status_request->json_body, request->callback, request, status_request->key);
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

    http_request(request->base_url, sub_request->endpoint, sub_request->method, sub_request->json_body, callback, request, key);
}

RESTFUL_REQUEST *restful_make_request(void *tile, const char *base_url, RESTFUL_REQUEST_DATA *action_request, RESTFUL_REQUEST_DATA *status_request, restful_callback_t callback) {
    if (!base_url || !callback || (!action_request && !status_request)) {
        DEBUG_printf("Required data in request was NULL");
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
    // if (request->action_request) {
    //     restful_free_request_data(request->action_request);
    // }
    // if (request->status_request) {
    //     restful_free_request_data(request->status_request);
    // }
    free(request);
}

 RESTFUL_REQUEST_DATA *restful_make_request_data(const char *method, const char *endpoint, const char *json_body, const char *key) {
    if (!method || !endpoint || !json_body) {
        DEBUG_printf("Required data in request data was NULL");
        return NULL;
    }
    RESTFUL_REQUEST_DATA *request_data = (RESTFUL_REQUEST_DATA *)malloc(sizeof(RESTFUL_REQUEST_DATA));
    request_data->method = (char*) malloc(strlen(method) + 1 * sizeof(char));
    strcpy(request_data->method, method);
    request_data->endpoint = (char*) malloc(strlen(endpoint) + 1 * sizeof(char));
    strcpy(request_data->endpoint, endpoint);
    request_data->json_body = (char*) malloc(strlen(json_body) + 1 * sizeof(char));
    strcpy(request_data->json_body, json_body);
    if (key != NULL) {
        request_data->key = (char*) malloc(strlen(key) + 1 * sizeof(char));
        strcpy(request_data->key, key);
    } else {
        request_data->key = NULL;
    }
    return request_data;
 }

void restful_free_request_data(RESTFUL_REQUEST_DATA *request) {
    free(request->method);
    free(request->endpoint);
    free(request->json_body);
    if (request->key) {
        free(request->key);
    }
}