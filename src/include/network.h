#ifndef SHOXY_NETWORK_H_
#define SHOXY_NETWORK_H_

#include <stdlib.h>
#include <arpa/inet.h>
#include <poll.h>

// this is actually used in others parts of the
// project so the define is prefixed with the
// namespace 'network' to avoid names conflicts.
#define NETWORK_RETURN_SUCCESS 0
#define NETWORK_RETURN_FAILURE -1

#define TCP_POLL_TIMEOUT -1
#define TCP_READ_TIMEOUT 50  // optional
#define TCP_WRITE_TIMEOUT 50 // optional
#define TCP_LISTEN_MAX 42

// defined in client.h
typedef struct client_s client_t;

typedef struct network_s
{
	int socket;
	char host_ip[INET_ADDRSTRLEN];
	size_t local_port;
} network_t;

int tcp_listen(const char *address, const size_t port);
client_t *tcp_accept(const size_t server_socket);
int tcp_terminate(client_t *client);

int network_listen_and_serve(const char *address, const size_t port);

int poll_set_pollfd(int server_socket, client_t *clients, struct pollfd *to_monitor);
client_t *poll_do(int server_socket, client_t *clients, struct pollfd *to_monitor, int to_monitor_len);

#endif
