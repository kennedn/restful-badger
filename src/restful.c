#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "http.h"
#include "restful.h"

int64_t alarm_callback(alarm_id_t id, void *arg) {
    RESTFUL_REQUEST *request = (RESTFUL_REQUEST *)arg;
    RESTFUL_STATUS_REQUEST *status_request = request->status_request;
    http_request(request->base_url, status_request->request.endpoint, status_request->request.method, 
        status_request->request.json_body, request->callback, request, status_request->key);
}

void restful_callback(char *value, int response_code, void *arg) {
    RESTFUL_REQUEST *request = (RESTFUL_REQUEST *)arg;
    if (response_code != 200 || request->status_request == NULL) {
        request->callback(value, response_code, arg);
    }

    // There has to be a better way to do this, context for original http_request gets lost if we call recursively here and tcp_poll errors ensue
    add_alarm_in_ms(1, alarm_callback, arg, false);
}


void restful_request(RESTFUL_REQUEST *request) {
    RESTFUL_ACTION_REQUEST *action_request = request->action_request;
    http_request(request->base_url, action_request->endpoint, action_request->method, action_request->json_body, restful_callback, request, NULL);
}