#include "http.h"

#include <string.h>
#include <time.h>

#include "badger.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"

#define TCP_PORT 443
#define BUF_SIZE 2048

#define POLL_TIME_S 5

#if 0
static void dump_bytes(const uint8_t *bptr, uint32_t len) {
    unsigned int i = 0;

   DEBUG_printf("dump_bytes %d", len);
    for (i = 0; i < len;) {
        if ((i & 0x0f) == 0) {
           DEBUG_printf("\n");
        } else if ((i & 0x07) == 0) {
           DEBUG_printf(" ");
        }
       DEBUG_printf("%02x ", bptr[i++]);
    }
   DEBUG_printf("\n");
    for (i = 0; i < len;) {
       DEBUG_printf("%c", bptr[i++]);
    }
   DEBUG_printf("\n");
}
#define DUMP_BYTES dump_bytes
#else
#define DUMP_BYTES(A, B)
#endif

typedef struct TLS_CLIENT_T_ {
    struct altcp_pcb *pcb;
    ip_addr_t remote_addr;
    uint8_t buffer[BUF_SIZE];
    int buffer_len;
    int sent_len;
    char base_url[20];
    const char *method;
    const char *endpoint;
    const char *json_body;
    const char *key;
    http_callback_t callback;
    void *arg;
} TLS_CLIENT_T;

static struct altcp_tls_config *tls_config = NULL;

/*!
 * \brief Extract HTTP response code and optionally scrape the JSON body for a key/value pair
 * \param arg TCP server state struct
 */
static void http_process_buffer(void *arg) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    if (state->buffer == NULL) {
        state->callback(NULL, 0, state->arg);
        return;
    }

    // Process HTTP response body, example: "HTTP/1.1 200 OK\r\n"
    char *message_body = strstr((char *)state->buffer, " ");
    if (message_body == NULL) {
        state->callback(NULL, 0, state->arg);
        return;
    }

    // Move past deliminater and convert to number
    int response_code = atoi(++message_body);

    // Key wasn't provided by user, so exit early
    if (!state->key) {
        state->callback(NULL, response_code, state->arg);
        return;
    }

    // Return if no header present to indicate JSON content
    if (strstr(message_body, "Content-Type: application/json") == NULL) {
        state->callback(NULL, response_code, state->arg);
        return;
    }

    // Seek to last occurrence of newline, which should contain JSON string
    message_body = strrchr(message_body, '\n');
    if (message_body == NULL) {
        state->callback(NULL, response_code, state->arg);
        return;
    }
    message_body++;
    DEBUG_printf("http_message_body_parse message_body: %s\n", message_body);

    // Simple JSON key/value extractor, essentially just grep the json string for our value based on the user provided key and do a bit of cleanup

    char key[20];
    char *token;
    char value[20];
    char *value_ptr = value;
    // Build substring search and get a pointer to the first occurrence of the search
    sprintf(key, "\"%s\":", state->key);
    token = strstr(message_body, key);
    // Key not found in JSON, exit early
    if (token == NULL) {
        state->callback(NULL, response_code, state->arg);
        return;
    }
    // Seek past the found string
    token += strlen(key);

    // Cleanup and extract value
    for (int i = 0; token[i]; i++) {
        if (token[i] == '"') {
            continue;
        }
        if (token[i] == ',' || token[i] == '}') {
            break;
        }
        *value_ptr++ = token[i];
    }

    // Null terminate
    *value_ptr = '\0';

    DEBUG_printf("http_message_body_parse value: %s\n", value);

    state->callback(value, response_code, state->arg);
}

static err_t tls_client_close(void *arg) {
    DEBUG_printf("tls_client_close\n");
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    err_t err = ERR_OK;
    if (state == NULL) {
        return err;
    }
    
    if (state->pcb != NULL) {
        altcp_arg(state->pcb, NULL);
        altcp_poll(state->pcb, NULL, 0);
        altcp_sent(state->pcb, NULL);
        altcp_recv(state->pcb, NULL);
        altcp_err(state->pcb, NULL);
        err = altcp_close(state->pcb);
        if (err != ERR_OK) {
            DEBUG_printf("close failed %d, calling abort\n", err);
            altcp_abort(state->pcb);
            err = ERR_ABRT;
        }
        state->pcb = NULL;
    }
    return err;
}

// Called with results of operation
static err_t tls_result(void *arg, int status) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    if (status == 0) {
        DEBUG_printf("success\n");
    } else {
        DEBUG_printf("failed %d\n", status);
    }

    err_t err = tls_client_close(arg);
    http_process_buffer(arg);
    free(state);
    DEBUG_printf("state freed\n");
    return err;
}

static err_t tls_client_sent(void *arg, struct altcp_pcb *tpcb, u16_t len) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    DEBUG_printf("tls_client_sent %u\n", len);
    state->sent_len += len;

    if (state->sent_len >= BUF_SIZE) {
        state->buffer_len = 0;
        state->sent_len = 0;
        DEBUG_printf("Waiting for buffer from server\n");
    }

    return ERR_OK;
}

static err_t tls_client_connected(void *arg, struct altcp_pcb *tpcb, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    if (err != ERR_OK) {
        DEBUG_printf("connect failed %d\n", err);
        return tls_result(arg, err);
    }
    DEBUG_printf("Connected, sending HTTP payload\n");

    state->buffer_len = sprintf((char *)state->buffer,
                                "%s %s HTTP/1.1\r\n"
                                "Host: %s\r\n"
                                "Content-Type: application/json\r\n"
                                "Content-Length: %d\r\n\r\n"
                                "%s",
                                state->method, state->endpoint,
                                state->base_url,
                                strlen(state->json_body),
                                state->json_body);

    DEBUG_printf("Writing %d bytes to server\n", state->buffer_len);
    err = altcp_write(tpcb, state->buffer, state->buffer_len, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        DEBUG_printf("Failed to enqueue data %d\n", err);
        return tls_result(arg, -1);
    }

    err = altcp_output(tpcb);
    if (err != ERR_OK) {
        DEBUG_printf("Failed to send data %d\n", err);
        return tls_result(arg, -1);
    }

    state->buffer_len = 0;

    return ERR_OK;
}

static err_t tls_client_poll(void *arg, struct altcp_pcb *tpcb) {
    DEBUG_printf("tls_client_poll\n");
    return tls_result(arg, -1);  // no response is an error?
}

static void tls_client_err(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        DEBUG_printf("tls_client_err %d\n", err);
        tls_result(arg, err);
    }
}

err_t tls_client_recv(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    if (!p) {
        return tls_result(arg, -1);
    }
    if (p->tot_len > 0) {
        DEBUG_printf("recv %d err %d\n", p->tot_len, err);
        for (struct pbuf *q = p; q != NULL; q = q->next) {
            DUMP_BYTES(q->payload, q->len);
        }
        // Receive the buffer
        const uint16_t buffer_left = BUF_SIZE - state->buffer_len;
        state->buffer_len += pbuf_copy_partial(p, state->buffer + state->buffer_len,
                                               p->tot_len > buffer_left ? buffer_left : p->tot_len, 0);
        altcp_recved(tpcb, p->tot_len);
    }
    pbuf_free(p);

    // If we have received the whole buffer, we are done
    if (state->buffer_len == p->tot_len) {
        return tls_result(arg, 0);
    }
    return ERR_OK;
}

static bool tls_client_open(void *arg) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    DEBUG_printf("Connecting to %s port %u\n", ip4addr_ntoa(&state->remote_addr), TCP_PORT);
    state->pcb = altcp_tls_new(tls_config, IP_GET_TYPE(&state->remote_addr));
    if (!state->pcb) {
        DEBUG_printf("failed to create pcb\n");
        return false;
    }

    altcp_arg(state->pcb, state);
    altcp_poll(state->pcb, tls_client_poll, POLL_TIME_S * 2);
    altcp_sent(state->pcb, tls_client_sent);
    altcp_recv(state->pcb, tls_client_recv);
    altcp_err(state->pcb, tls_client_err);

    state->buffer_len = 0;

    cyw43_arch_lwip_begin();
    err_t err =  altcp_connect(state->pcb, &state->remote_addr, TCP_PORT, tls_client_connected);
    cyw43_arch_lwip_end();

    return err == ERR_OK;
}

// Call back with a DNS result
static void tls_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    if (ipaddr) {
        state->remote_addr = *ipaddr;
        DEBUG_printf("tls address %s\n", ipaddr_ntoa(ipaddr));
        if (!tls_client_open(state)) {
            tls_result(state, -1);
            return;
        }
    } else {
        DEBUG_printf("tls dns request failed\n");
        tls_result(state, -1);
    }
}

// Perform initialisation
static TLS_CLIENT_T *tls_client_init(const char *url, const char *endpoint, const char *method, const char *json_body, const char *key, http_callback_t callback, void *arg) {
    TLS_CLIENT_T *state = calloc(1, sizeof(TLS_CLIENT_T));
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return NULL;
    }
    strncpy(state->base_url, url, count_of(state->base_url) - 1);
    state->endpoint = endpoint;
    state->method = method;
    state->json_body = json_body;
    state->key = key;

    state->callback = callback;
    state->arg = arg;
    return state;
}

void http_request(const char *url, const char *endpoint, const char *method, const char *json_body, const char *key, http_callback_t callback, void *arg) {
    tls_config = altcp_tls_create_config_client(NULL, 0);
    if(!tls_config) {
        return;
    }
    TLS_CLIENT_T *state = tls_client_init(url, endpoint, method, json_body, key, callback, arg);
    if (!state) {
        return;
    }

    mbedtls_ssl_set_hostname(altcp_tls_context(state->pcb), url);
    cyw43_arch_lwip_begin();
    int err = dns_gethostbyname(url, &state->remote_addr, tls_dns_found, state);
    cyw43_arch_lwip_end();

    if (err == ERR_OK) {  // Cached result
        if (!tls_client_open(state)) {
            tls_result(state, -1);
            altcp_tls_free_config(tls_config);
            return;
        }
    } else if (err != ERR_INPROGRESS) {  // ERR_INPROGRESS means the callback will set the remote address
        DEBUG_printf("dns request failed\n");
        tls_result(state, -1);
        altcp_tls_free_config(tls_config);
    }
}