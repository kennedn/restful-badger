#pragma once

typedef void (*restful_callback_t)(char *value, int response_code, void *arg);

typedef struct RESTFUL_ACTION_REQUEST_ {
    char method[8];
    char endpoint[40];
    char json_body[128];
} RESTFUL_ACTION_REQUEST;

typedef struct RESTFUL_STATUS_REQUEST_ {
    RESTFUL_ACTION_REQUEST request;
    char key[20];
} RESTFUL_STATUS_REQUEST;

typedef struct RESTFUL_REQUEST_ {
    int caller;
    char base_url[20];
    RESTFUL_ACTION_REQUEST *action_request;
    RESTFUL_STATUS_REQUEST *status_request;
    restful_callback_t callback;
} RESTFUL_REQUEST;

void restful_request(RESTFUL_REQUEST *request);
