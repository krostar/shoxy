#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <errno.h>
#include "shoxy.h"
#include "network.h"
#include "client.h"

void network_loop(int server_socket)
{
	struct pollfd to_monitor[1024];
	int pollfd_len;
	client_t *clients = NULL;
	int rc;

	RUNNING = 1; // turned off by interruption signal
	while (RUNNING)
	{
		pollfd_len = poll_set_pollfd(server_socket, clients, to_monitor);
		rc = poll(to_monitor, pollfd_len, TCP_POLL_TIMEOUT);
		if (rc == -1)
		{
			log_errorf("poll failed with error : \"%s\"", strerror(errno));
		}
		else if (rc == 0)
		{
			// there is nothing to do, poll timeout, useful in case RUNNING is now set to 0
		}
		else
		{
			// some on our monitored sockets file descriptor state changed
			clients = poll_do(server_socket, clients, to_monitor, pollfd_len);
		}
		clients_list(clients);
	}

	for (; clients != NULL;)
	{
		client_t *client = clients;
		clients = clients_remove(clients, client);
		tcp_terminate(client);
		client_delete(client);
	}
}

int network_listen_and_serve(const char *address, const size_t port)
{
	int server_socket;

	if ((server_socket = tcp_listen(address, port)) == NETWORK_RETURN_FAILURE)
	{
		log_error("fail to listen on given address and port");
		return (NETWORK_RETURN_FAILURE);
	}
	log_infof("listen on %s:%lu", address, port);

	network_loop(server_socket);

	if (close(server_socket) == -1)
	{
		log_error("fail to close server socket");
		return (NETWORK_RETURN_FAILURE);
	}

	return (NETWORK_RETURN_SUCCESS);
}
