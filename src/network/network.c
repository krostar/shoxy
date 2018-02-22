#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <poll.h>
#include "shoxy.h"
#include "network.h"
#include "client.h"

void network_loop(ssh_bind b)
{
	client_t *clients = NULL;
	ssh_event e = ssh_event_new();
	network_server_data_t callback_infos = {&clients, e, b};

	// add server socket in poll event
	if (ssh_event_add_fd(e, ssh_bind_get_fd(b), POLLIN, &network_poll_on_state_change, &callback_infos) != SSH_OK)
	{
		log_error("unable to add server socket to poll event");
		return;
	}

	RUNNING = 1; // turned off by interruption signal
	while (RUNNING)
	{
		int rc = ssh_event_dopoll(e, TCP_POLL_TIMEOUT);
		if (rc == SSH_AGAIN)
		{
			// this is a ugly workaround
			network_poll_ugly_workaround(&clients);
			// timed out, useful to check if we're still RUNNING
		}
		else if (rc != SSH_OK)
		{
			int client_disconnected = 0;
			// callbacks for exceptions can't be set so we have to check all sessions
			// we may find that someone disconnected
			for (; clients != NULL;)
			{
				client_t *client = clients;
				int client_status = ssh_get_status(client->ssh->session);
				if (client_status & SSH_CLOSED || client_status & SSH_CLOSED_ERROR)
				{
					network_poll_on_client_critical_error(&clients, client->network->socket);
					client_disconnected = 1;
					break;
				}
			}
			if (client_disconnected != 1)
				log_error("ssh event poll failed: \"%s\"", ssh_get_error(b));
		}
	}

	// remove server socket from poll event
	if (ssh_event_remove_fd(e, ssh_bind_get_fd(b)) != SSH_OK)
		log_error("unable to remove server socket from poll event");

	clients_list(clients);

	// clean all clients in poll event
	for (; clients != NULL;)
	{
		client_t *client = clients;
		network_poll_on_client_critical_error(&clients, client->network->socket);
	}
	ssh_event_free(e);
}

int network_listen_and_serve(const char *address, const size_t port)
{
	int server_socket;
	ssh_bind b;

	if ((server_socket = tcp_listen(address, port)) == NETWORK_RETURN_FAILURE)
	{
		log_error("fail to listen on given address and port");
		return (NETWORK_RETURN_FAILURE);
	}
	log_info("listen on %s:%lu", address, port);

	if ((b = ssh_bind_new()) == NULL)
	{
		log_error("unable to initialize ssh server");
		return (NETWORK_RETURN_FAILURE);
	}

	if (ssh_bind_options_set(b, SSH_BIND_OPTIONS_DSAKEY, "build/keys/ssh_host_dsa_key") != SSH_OK)
	{
		log_error("ssh bind set banner option failed: \"%s\"", ssh_get_error(b));
		return (NETWORK_RETURN_FAILURE);
	}
	if (ssh_bind_options_set(b, SSH_BIND_OPTIONS_RSAKEY, "build/keys/ssh_host_rsa_key") != SSH_OK)
	{
		log_error("ssh bind set banner option failed: \"%s\"", ssh_get_error(b));
		return (NETWORK_RETURN_FAILURE);
	}
	if (ssh_bind_options_set(b, SSH_BIND_OPTIONS_BANNER, "SHOXY PROXY SERVER") != SSH_OK)
	{
		log_error("ssh bind set banner option failed: \"%s\"", ssh_get_error(b));
		return (NETWORK_RETURN_FAILURE);
	}
	ssh_bind_set_fd(b, server_socket);

	network_loop(b);

	ssh_bind_free(b); // no need to close, bind_free already do it for us
	return (NETWORK_RETURN_SUCCESS);
}
