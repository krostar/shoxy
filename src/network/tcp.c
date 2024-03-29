#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "shoxy.h"
#include "client.h"
#include "network.h"

int tcp_listen(const char *address, const size_t port)
{
	int yes = 1;
	int server_socket;
	struct sockaddr_in server_info;

	// create a socket
	if ((server_socket = socket(AF_INET, SOCK_STREAM, getprotobyname("TCP")->p_proto)) == -1)
	{
		log_error("unable to create socket: %s", strerror(errno));
		return (NETWORK_RETURN_FAILURE);
	}

	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
	{
		log_error("unable to set reusable address option on server socket: %s", strerror(errno));
		close(server_socket);
		return (NETWORK_RETURN_FAILURE);
	};

	// set server attributes and bind newly-created socket
	server_info.sin_family = AF_INET;
	server_info.sin_port = htons(port);
	server_info.sin_addr.s_addr = inet_addr(address);
	if (bind(server_socket, (const struct sockaddr *)&server_info, sizeof(server_info)) == -1)
	{
		log_error("unable to bind socket: %s", strerror(errno));
		close(server_socket);
		return (NETWORK_RETURN_FAILURE);
	}
	if (listen(server_socket, TCP_LISTEN_MAX) == -1)
	{
		log_error("unable to listen on socket: %s", strerror(errno));
		close(server_socket);
		return (NETWORK_RETURN_FAILURE);
	}

	return (server_socket);
}

client_t *tcp_accept(const size_t server_socket)
{
	client_t *client;
	int client_socket;
	UNUSED struct timeval timeout; // if read or write timeout are undefined drop the warning
	struct sockaddr_in client_info;
	socklen_t size = (socklen_t)sizeof(client_info);

	if ((client_socket = accept(server_socket, (struct sockaddr *)&client_info, &size)) == -1)
	{
		log_error("unable to accept from server socket: %s", strerror(errno));
		return (NULL);
	}

// set read and write timeout options on socket
#ifdef TCP_READ_TIMEOUT
	timeout.tv_usec = 0;
	timeout.tv_sec = TCP_READ_TIMEOUT;
	if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
	{
		log_error("unable to set read timeout option on client socket: %s", strerror(errno));
		close(client_socket);
		return (NULL);
	}
#endif
#ifdef TCP_WRITE_TIMEOUT
	timeout.tv_usec = 0;
	timeout.tv_sec = TCP_WRITE_TIMEOUT;
	if (setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == -1)
	{
		log_error("unable to set write timeout option on client socket: %s", strerror(errno));
		close(client_socket);
		return (NULL);
	}
#endif

	// create client and fill attributes
	if ((client = client_create()) == NULL)
	{
		log_error("unable to create empty client");
		tcp_terminate(client);
		return (NULL);
	}
	client->network->socket = client_socket;
	client->network->local_port = ntohs(client_info.sin_port);
	if (inet_ntop(AF_INET, &(client_info.sin_addr), client->network->host_ip, INET_ADDRSTRLEN) == NULL)
	{
		log_client_error(client, "unable to retrieve client IP: %s", strerror(errno));
		client_delete(client);
		tcp_terminate(client);
		return (NULL);
	}

	log_client_info(client, "connected");
	return (client);
}

int tcp_terminate(client_t *client)
{
	if (close(client->network->socket) == -1)
	{
		log_client_error(client, "unable to close client socket: %s", strerror(errno));
		return (NETWORK_RETURN_FAILURE);
	}

	log_client_info(client, "disconnected");
	return (NETWORK_RETURN_SUCCESS);
}
