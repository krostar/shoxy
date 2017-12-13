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
		to_monitor[i + 1].fd = client->network->socket;
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
			// we have a socket error, socket has not been closed yet but shutdown has already been called
			// we have nothing to do other than close it and delete the client
			clients = poll_handle_error(clients, to_monitor[i].fd);
		else if (to_monitor[i].revents & POLLIN)
			// we have something to read
			clients = poll_handle_pollin(server_socket, clients, to_monitor[i].fd);
		else if (to_monitor[i].revents & POLLOUT)
			// we have something to write
			clients = poll_handle_pollout(clients, to_monitor[i].fd);
		else
			log_debugf("unhandled event on socket %lu: %d", to_monitor[i].fd, to_monitor[i].revents);
	}
	return (clients);
}

client_t *poll_handle_error(client_t *clients, int socket)
{
	client_t *client;

	log_errorf("an error ocurred on socket %d, terminate it", socket);
	if ((client = client_find_by_socket(clients, socket)) == NULL)
	{
		log_errorf("client corresponding to socket %d is null", socket);
		return (clients);
	}
	clients = clients_remove(clients, client);
	ssh_terminate_session(client);
	tcp_terminate(client);
	client_delete(client);
	return (clients);
}

client_t *poll_handle_pollin(int server_socket, client_t *clients, int socket)
{
	client_t *client;

	// server has a new client
	if (socket == server_socket)
	{
		client = tcp_accept(server_socket);
		if (ssh_create_session(client) == SSH_RETURN_SUCCESS)
			clients = clients_add(clients, client);
		else
		{
			log_error("unable to create ssh session with client");
			tcp_terminate(client);
			client_delete(client);
		}
	}
	else // one of the ssh client is talking
	{
		log_debugf("socket %d has something to say", socket);
		if ((client = client_find_by_socket(clients, socket)) == NULL)
		{
			log_errorf("client corresponding to socket %d is null", socket);
			return (clients);
		}
		// do something about it
		log_debugf("session state = %d", client->ssh->session);
	}
	return (clients);
}

client_t *poll_handle_pollout(client_t *clients, int socket)
{
	client_t *client;

	log_debugf("socket %d has something to tell", socket);
	if ((client = client_find_by_socket(clients, socket)) == NULL)
	{
		log_errorf("client corresponding to socket %d is null", socket);
		return (clients);
	}
	// do something about it
	return (clients);
}
