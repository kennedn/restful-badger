#include "http.h"

#include <string.h>
#include <time.h>

#include "badger.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#define TCP_PORT 80
#define BUF_SIZE 2048

#define TEST_ITERATIONS 10
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

typedef struct TCP_CLIENT_T_ {
    struct tcp_pcb *tcp_pcb;
    ip_addr_t remote_addr;
    uint8_t buffer[BUF_SIZE];
    int buffer_len;
    int sent_len;
    char base_url[20];
    char method[8];
    char endpoint[40];
    char json_body[128];
    http_callback_t callback;
    void *arg;
    char key[10];
} TCP_CLIENT_T;

/*!
 * \brief Extract HTTP response code and optionally scrape the JSON body for a key/value pair
 * \param arg TCP server state struct
 */
static void http_process_buffer(void *arg) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
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

    if (state->key[0] == '\0') {
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
    if (value == NULL) {
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

static err_t tcp_client_close(void *arg) {
    DEBUG_printf("tcp_client_close\n");
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    err_t err = ERR_OK;
    if (state == NULL) {
        return err;
    }
    
    if (state->tcp_pcb != NULL) {
        tcp_arg(state->tcp_pcb, NULL);
        tcp_poll(state->tcp_pcb, NULL, 0);
        tcp_sent(state->tcp_pcb, NULL);
        tcp_recv(state->tcp_pcb, NULL);
        tcp_err(state->tcp_pcb, NULL);
        err = tcp_close(state->tcp_pcb);
        if (err != ERR_OK) {
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(state->tcp_pcb);
            err = ERR_ABRT;
        }
        state->tcp_pcb = NULL;
    }
    return err;
}

// Called with results of operation
static err_t tcp_result(void *arg, int status) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    if (status == 0) {
        DEBUG_printf("success\n");
    } else {
        DEBUG_printf("failed %d\n", status);
    }

    err_t err = tcp_client_close(arg);
    http_process_buffer(arg);
    free(state);
    DEBUG_printf("state freed\n");
    return err;
}

static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    DEBUG_printf("tcp_client_sent %u\n", len);
    state->sent_len += len;

    if (state->sent_len >= BUF_SIZE) {
        state->buffer_len = 0;
        state->sent_len = 0;
        DEBUG_printf("Waiting for buffer from server\n");
    }

    return ERR_OK;
}

static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    if (err != ERR_OK) {
        DEBUG_printf("connect failed %d\n", err);
        return tcp_result(arg, err);
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
    err = tcp_write(tpcb, state->buffer, state->buffer_len, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        DEBUG_printf("Failed to enqueue data %d\n", err);
        return tcp_result(arg, -1);
    }

    err = tcp_output(tpcb);
    if (err != ERR_OK) {
        DEBUG_printf("Failed to send data %d\n", err);
        return tcp_result(arg, -1);
    }

    state->buffer_len = 0;

    return ERR_OK;
}

static err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb) {
    DEBUG_printf("tcp_client_poll\n");
    return tcp_result(arg, -1);  // no response is an error?
}

static void tcp_client_err(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        DEBUG_printf("tcp_client_err %d\n", err);
        tcp_result(arg, err);
    }
}

err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    if (!p) {
        return tcp_result(arg, -1);
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
        tcp_recved(tpcb, p->tot_len);
    }
    pbuf_free(p);

    // If we have received the whole buffer, we are done
    if (state->buffer_len == p->tot_len) {
        return tcp_result(arg, 0);
    }
    return ERR_OK;
}

static bool tcp_client_open(void *arg) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    DEBUG_printf("Connecting to %s port %u\n", ip4addr_ntoa(&state->remote_addr), TCP_PORT);
    state->tcp_pcb = tcp_new_ip_type(IP_GET_TYPE(&state->remote_addr));
    if (!state->tcp_pcb) {
        DEBUG_printf("failed to create pcb\n");
        return false;
    }

    tcp_arg(state->tcp_pcb, state);
    tcp_poll(state->tcp_pcb, tcp_client_poll, POLL_TIME_S * 2);
    tcp_sent(state->tcp_pcb, tcp_client_sent);
    tcp_recv(state->tcp_pcb, tcp_client_recv);
    tcp_err(state->tcp_pcb, tcp_client_err);

    state->buffer_len = 0;

    cyw43_arch_lwip_begin();
    err_t err = tcp_connect(state->tcp_pcb, &state->remote_addr, TCP_PORT, tcp_client_connected);
    cyw43_arch_lwip_end();

    return err == ERR_OK;
}

// Call back with a DNS result
static void tcp_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    if (ipaddr) {
        state->remote_addr = *ipaddr;
        DEBUG_printf("tcp address %s\n", ipaddr_ntoa(ipaddr));
        if (!tcp_client_open(state)) {
            tcp_result(state, -1);
            return;
        }
    } else {
        DEBUG_printf("tcp dns request failed\n");
        tcp_result(state, -1);
    }
}

// Perform initialisation
static TCP_CLIENT_T *tcp_client_init(const char *url, const char *endpoint, const char *method, const char *json_body, http_callback_t callback, void *arg, const char *key) {
    TCP_CLIENT_T *state = calloc(1, sizeof(TCP_CLIENT_T));
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return NULL;
    }
    strncpy(state->base_url, url, count_of(state->base_url) - 1);
    strncpy(state->endpoint, endpoint, count_of(state->endpoint) - 1);
    strncpy(state->method, method, count_of(state->method) - 1);
    strncpy(state->json_body, json_body, count_of(state->json_body) - 1);

    state->key[0] = '\0';
    if (key != NULL) {
        strncpy(state->key, key, count_of(state->key) - 1);
    }
    state->callback = callback;
    state->arg = arg;
    return state;
}

void http_request(const char *url, const char *endpoint, const char *method, const char *json_body, http_callback_t callback, void *arg, const char *key) {
    TCP_CLIENT_T *state = tcp_client_init(url, endpoint, method, json_body, callback, arg, key);
    if (!state) {
        return;
    }

    cyw43_arch_lwip_begin();
    int err = dns_gethostbyname(url, &state->remote_addr, tcp_dns_found, state);
    cyw43_arch_lwip_end();

    if (err == ERR_OK) {  // Cached result
        if (!tcp_client_open(state)) {
            tcp_result(state, -1);
            return;
        }
    } else if (err != ERR_INPROGRESS) {  // ERR_INPROGRESS means the callback will set the remote address
        DEBUG_printf("dns request failed\n");
        tcp_result(state, -1);
    }
}

// void http_request_with_status