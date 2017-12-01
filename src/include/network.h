#ifndef SHOXY_NETWORK_H_
#define SHOXY_NETWORK_H_

#include <stdlib.h>

#define NETWORK_RETURN_SUCCESS 0

int listen_and_serve(const char *address, const size_t port);

#endif
