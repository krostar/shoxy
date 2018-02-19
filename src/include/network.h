#ifndef SHOXY_NETWORK_H_
#define SHOXY_NETWORK_H_

#include <stdlib.h>
#include <arpa/inet.h>
#include <libssh/server.h>

// this is actually used in others parts of the
// project so the define is prefixed with the
// namespace 'network' to avoid names conflicts.
#define NETWORK_RETURN_SUCCESS 0
#define NETWORK_RETURN_FAILURE -1

#define TCP_POLL_TIMEOUT 1000
#define TCP_READ_TIMEOUT 50  // optional
#define TCP_WRITE_TIMEOUT 50 // optional
#define TCP_LISTEN_MAX 42

// defined in client.h
typedef struct client_s client_t;

typedef struct network_client_data_s
{
	int socket;
	char host_ip[INET_ADDRSTRLEN];
	size_t local_port;
} network_client_data_t;

typedef struct network_server_data_s
{
	client_t **clients;
	ssh_event e;
	ssh_bind b;
} network_server_data_t;

int tcp_listen(const char *address, const size_t port);
client_t *tcp_accept(const size_t server_socket);
int tcp_terminate(client_t *client);

int network_listen_and_serve(const char *address, const size_t port);
int network_poll_on_client_critical_error(client_t **clients, int socket);
int network_poll_on_state_change(socket_t fd, int revents, void *userdata);

#endif
