#ifndef SHOXY_H_
#define SHOXY_H_

// global variable to enable proper stop
int RUNNING;

#define UNUSED __attribute__((__unused__))

int signal_catch_interruption();
void log_error(const char *message);
void log_errorf(const char *format, ...);
void log_info(const char *message);
void log_infof(const char *format, ...);
void log_debug(const char *message);
void log_debugf(const char *format, ...);

#endif
