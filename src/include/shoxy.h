#ifndef SHOXY_H_
#define SHOXY_H_

#include "network.h"

#define UNUSED __attribute__((__unused__))

typedef struct client_s
{
	network *link;
} client;

void log_error(const char *message);
void log_errorf(const char *format, ...);
void log_info(const char *message);
void log_infof(const char *format, ...);

#endif
