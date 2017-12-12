#include <unistd.h>
#include <poll.h>
#include "shoxy.h"
#include "client.h"

int poll_set_pollfd(int server_socket, client_t *clients, struct pollfd *to_monitor)
{
	int i = 0;

	to_monitor[0].fd = server_socket;
	to_monitor[0].events = POLLIN;

	for (client_t *client = clients; client != NULL; client = client->next)
	{
		to_monitor[i + 1].fd = client->link->socket;
		to_monitor[i + 1].events = POLLIN;
		i++;
	}
	return (i + 1);
}

client_t *poll_do(int server_socket, client_t *clients, struct pollfd *to_monitor, int to_monitor_len)
{
	for (int i = 0; i < to_monitor_len; i++)
	{
		if (to_monitor[i].revents == 0)
			continue;
		else if (to_monitor[i].revents & POLLHUP || to_monitor[i].revents & POLLERR)
		{
			// we have a socket error, socket has not been closed yet but shutdown has already been called
			// we have nothing to do other than close it and delete the client
			client_t *client = client_find_by_socket(clients, to_monitor[i].fd);
			clients = clients_remove(clients, client);
			tcp_terminate(client);
			client_delete(client);
		}
		else if (to_monitor[i].revents & POLLIN)
		{
			if (to_monitor[i].fd == server_socket)
				clients = clients_add(clients, tcp_accept(server_socket));
			else
			{
				client_t *client = client_find_by_socket(clients, to_monitor[i].fd);
				// client has something to read !!
			}
		}
		else if (to_monitor[i].revents & POLLOUT)
		{
			client_t *client = client_find_by_socket(clients, to_monitor[i].fd);
			// client has something to write !!
		}
		else
			log_debugf("unhandled event on socket %lu: %d", to_monitor[i].fd, to_monitor[i].revents);
	}
	return (clients);
}
