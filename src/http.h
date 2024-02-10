#pragma once

typedef void (*http_callback_t)(char *value, int response_code, void *arg);
void http_request(const char *url, const char *endpoint, const char *method, const char *json_body, http_callback_t callback, void *arg, const char *key);