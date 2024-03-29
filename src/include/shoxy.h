#ifndef SHOXY_H_
#define SHOXY_H_

// global variable to enable proper stop
int RUNNING;

#define UNUSED __attribute__((__unused__))
#define LOG_VERBOSITY_DEBUG 2
#define LOG_VERBOSITY_INFO 1
#define LOG_VERBOSITY_ERROR 0

typedef struct client_s client_t;

int signal_catch_interruption();
void log_error(const char *format, ...);
void log_info(const char *format, ...);
void log_debug(const char *format, ...);
void log_client_error(client_t *client, const char *format, ...);
void log_client_info(client_t *client, const char *format, ...);
void log_client_debug(client_t *client, const char *format, ...);

#endif
