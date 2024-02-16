#pragma once

#include "lwip/err.h"

typedef void (*http_callback_t)(char *value, int response_code, void *arg);
void http_request(const char *url, const char *endpoint, const char *method, const char *json_body, const char *key, http_callback_t callback, void *arg);