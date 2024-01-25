#pragma once
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"

typedef enum HTTP_METHOD_T_ {
    HTTP_METHOD_GET,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT
} HTTP_METHOD_T;

typedef enum HTTP_VERSION_T_ {
    HTTP_VERSION_1,
    HTTP_VERSION_1_1,
    HTTP_VERSION_2,
    HTTP_VERSION_3,
} HTTP_VERSION_T;

typedef enum HTTP_CODE_LOOKUP_T_ {
    HTTP_CODE_LOOKUP_STATUS = 0,
    HTTP_CODE_LOOKUP_UNKNOWN_VALUE = 1,
    HTTP_CODE_LOOKUP_NO_VALUE = 2
} HTTP_CODE_LOOKUP_T;

typedef struct HTTP_MESSAGE_BODY_T_ {
    HTTP_METHOD_T method;
    HTTP_VERSION_T version;
    char url[20];
    uint32_t code;
    bool input_change_flag;
} HTTP_MESSAGE_BODY_T;

/*!
  * \brief Extract parameters, react and then respond to a HTTP request.
  * \param arg TCP server state struct
  */
void http_process_recv_data(void *arg);
   
