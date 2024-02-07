typedef void (*ntp_callback_t)(datetime_t *datetime, void *arg);

void ntp_get_time(ntp_callback_t callback, void *arg);