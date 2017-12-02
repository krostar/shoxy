#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "shoxy.h"
#include "network.h"

int tcp_listen(const char *address, const size_t port)
{
	int server_socket;
	struct sockaddr_in sai;

	if ((server_socket = socket(AF_INET, SOCK_STREAM, getprotobyname("TCP")->p_proto)) == -1)
	{
		log_error("unable to create socket");
		return (NETWORK_RETURN_FAILURE);
	}

	sai.sin_family = AF_INET;
	sai.sin_port = htons(port);
	sai.sin_addr.s_addr = inet_addr(address);
	if (bind(server_socket, (const struct sockaddr *)&sai, sizeof(sai)) == -1)
	{
		log_error("unable to bind socket");
		return (NETWORK_RETURN_FAILURE);
	}
	if (listen(server_socket, TCP_LISTEN_MAX) == -1)
	{
		log_error("unable to listen on socket");
		return (NETWORK_RETURN_FAILURE);
	}

	return server_socket;
}
