#pragma once
#define DEBUG_PRINTF(...) \
  do { if (DEBUG_PRINT) printf(__VA_ARGS__); } while (0)


#define HALT_TIMEOUT_MS 60000
#define MULTI_CLICK_WAIT_MS 350
#define WIFI_CONNECT_ATTEMPTS 3