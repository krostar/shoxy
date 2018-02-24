#include <unistd.h>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <poll.h>
#include "shoxy.h"
#include "client.h"

// something went wrong or client disconnected, kick him
int network_poll_on_client_critical_error(client_t **clients, int socket)
{
	client_t *client;

	if ((client = client_find_by_socket(*clients, socket)) == NULL)
	{
		log_error("client corresponding to socket %d is null", socket);
		return (NETWORK_RETURN_FAILURE);
	}
	log_client_error(client, "a critical error occured or client disconnected");

	// we don't want to handle any more event from him
	ssh_event_remove_session(client->event, client->ssh->session);

	*clients = clients_remove(*clients, client);
	ssh_terminate_session(client);
	client_delete(client);

	return (NETWORK_RETURN_SUCCESS);
}

// call every time a new client know on server socket door
int network_poll_on_new_client(client_t **clients, network_server_data_t *data)
{
	client_t *client;

	if ((client = tcp_accept(ssh_bind_get_fd(data->b))) == NULL)
	{
		log_error("unable to accept client");
		return (NETWORK_RETURN_FAILURE);
	}

	client->event = data->e;
	if (ssh_create_session(client, data->b) != SSH_RETURN_SUCCESS)
	{
		log_client_error(client, "unable to create ssh session with client");
		tcp_terminate(client);
		client_delete(client);
		return (NETWORK_RETURN_FAILURE);
	}

	*clients = clients_add(*clients, client);

	if (ssh_handle_key_exchange(client->ssh->session) != SSH_OK)
	{
		log_client_error(client, "unable to exchange ssh keys: %s", ssh_get_error(client->ssh->session));
		network_poll_on_client_critical_error(clients, client->network->socket);
		return (NETWORK_RETURN_FAILURE);
	}

	// monitor this new client session
	if (ssh_event_add_session(data->e, client->ssh->session) != SSH_OK)
	{
		log_client_error(client, "unable to add session to ssh poll event: %s", ssh_get_error(data->e));
		network_poll_on_client_critical_error(clients, client->network->socket);
		return (NETWORK_RETURN_FAILURE);
	}

	return (NETWORK_RETURN_SUCCESS);
}

int network_poll_on_state_change(socket_t fd, int revents, void *userdata)
{
	network_server_data_t *cb_data = userdata;
	client_t **clients = cb_data->clients;

	if (ssh_bind_get_fd(cb_data->b) == fd)
		if (revents & POLLIN)
			if (network_poll_on_new_client(clients, cb_data) == NETWORK_RETURN_SUCCESS)
				return (SSH_OK);

	return (SSH_ERROR);
}

void network_poll_ugly_workaround(client_t **clients)
{
	// this function is here because of some bugs when modifying session in ssh callbacks
	// this is an ugly workarround, it would have been so much easier if we could have done this
	// directly in any callbacks instead of doing it here
	for (client_t *client = *clients; client != NULL; client = client->next)
	{
		if (client->ssh->channel != NULL)
		{
			// did the proxy channel closes recently ?
			if (client->ssh->proxy != NULL && client->ssh->proxy->closed == 1)
			{
				ssh_free(client->ssh->proxy->session);
				free(client->ssh->proxy);
				client->ssh->proxy = NULL;
			}

			// do we have something to say to the remote end ?
			if (client->ssh->exec_answer_buffer_len > 0)
			{
				int sent;
				if (client->ssh->proxy != NULL && client->ssh->proxy->channel != NULL)
					sent = ssh_channel_write(client->ssh->proxy->channel, client->ssh->exec_answer_buffer, client->ssh->exec_answer_buffer_len);
				else
					sent = ssh_channel_write(client->ssh->channel, client->ssh->exec_answer_buffer, client->ssh->exec_answer_buffer_len);
				client->ssh->exec_answer_buffer_len -= sent;
				memcpy(client->ssh->exec_answer_buffer, client->ssh->exec_answer_buffer + sent, client->ssh->exec_answer_buffer_len);
				client->ssh->exec_answer_buffer = realloc(client->ssh->exec_answer_buffer, client->ssh->exec_answer_buffer_len);
			}
			ssh_command_exec_if_needed(client);
		}

		if (!ssh_is_connected(client->ssh->session))
		{
			network_poll_on_client_critical_error(clients, client->network->socket);
			break;
		}
	}
}
