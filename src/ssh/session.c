#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/callbacks.h>
#include "shoxy.h"
#include "client.h"
#include "network.h"
#include "ssh_callback.h"

int ssh_create_session(client_t *client, ssh_bind b)
{
	int verbosity = SSH_LOG_FUNCTIONS;

	if ((client->ssh->session = ssh_new()) == NULL)
	{
		log_client_error(client, "unable to initialize ssh session");
		return (SSH_RETURN_FAILURE);
	}

	client->ssh->channel = NULL;
	client->ssh->exec_command_buffer = NULL;
	client->ssh->exec_command_buffer_len = 0;
	client->ssh->exec_answer_buffer = NULL;
	client->ssh->exec_answer_buffer_len = 0;

	ssh_options_set(client->ssh->session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	ssh_set_auth_methods(client->ssh->session, SSH_AUTH_METHOD_PASSWORD);
	ssh_set_message_callback(client->ssh->session, ssh_callback_request_message, client);

	if (ssh_bind_accept_fd(b, client->ssh->session, client->network->socket) != SSH_OK)
	{
		log_client_error(client, "unable to initialize ssh session: %s", ssh_get_error(b));
		ssh_free(client->ssh->session);
		return (NETWORK_RETURN_FAILURE);
	}

	return (SSH_RETURN_SUCCESS);
}

void ssh_terminate_session(client_t *client)
{
	if (client->ssh->exec_command_buffer != NULL)
		free(client->ssh->exec_command_buffer);
	client->ssh->exec_command_buffer = NULL;
	client->ssh->exec_command_buffer_len = 0;
	if (client->ssh->exec_answer_buffer != NULL)
		free(client->ssh->exec_answer_buffer);
	client->ssh->exec_answer_buffer = NULL;
	client->ssh->exec_answer_buffer_len = 0;

	if (client->ssh->channel != NULL)
	{
		ssh_channel_close(client->ssh->channel);
		ssh_channel_free(client->ssh->channel);
	}
	ssh_silent_disconnect(client->ssh->session);
	ssh_free(client->ssh->session);
}
