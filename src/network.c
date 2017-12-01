#include <stdio.h>
#include <network.h>

int listen_and_serve(const char *address, const size_t port)
{
	printf("listen on %s:%lu\n", address, port);
	return (NETWORK_RETURN_SUCCESS);
}
