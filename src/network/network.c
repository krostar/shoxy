#include <stdio.h>
#include <unistd.h>
#include "shoxy.h"
#include "network.h"

int listen_and_serve(const char *address, const size_t port)
{
	int server_socket = -1;

	if ((server_socket = tcp_listen(address, port)) == NETWORK_RETURN_FAILURE)
	{
		log_error("fail to listen on given address and port");
		return (NETWORK_RETURN_FAILURE);
	}
	log_infof("listen on %s:%lu", address, port);

	/*
		poll(read => socket server, write, event, timeout)

		client = ssh_connect_client(socket)
		client->socket = socket
		client


		On fait une socket server
		~~~Je sais pas comment on gere l'authentification du gars~~~
		On monitore le read/write
		Quand on read un certain message on forward a la fonction qui traite les messages de ssh
		Si le message c'est genre connect server 3
			On cree une nouvelle socket ~~~quid de l'acception du client ?~~~ et une nouvelle session
			libssh2_session_handshake

	*/

	if (close(server_socket) == -1)
	{
		log_error("fail to close server socket");
		return (NETWORK_RETURN_FAILURE);
	}

	return (NETWORK_RETURN_SUCCESS);
}
