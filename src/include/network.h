#ifndef SHOXY_NETWORK_H_
#define SHOXY_NETWORK_H_

#include <stdlib.h>

#define NETWORK_RETURN_SUCCESS 0
#define NETWORK_RETURN_FAILURE -1

#define SELECT_FD_MAX __FD_SETSIZE
#define TCP_LISTEN_MAX 42

typedef struct network_s
{
	int socket;
} network;

int tcp_listen(const char *address, const size_t port);
int listen_and_serve(const char *address, const size_t port);

#endif
