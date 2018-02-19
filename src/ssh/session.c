#include <unistd.h>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/callbacks.h>
#include "shoxy.h"
#include "client.h"
#include "network.h"
#include "ssh_callback.h"

int ssh_create_session(client_t *client)
{
	int verbosity = SSH_LOG_FUNCTIONS;

	client->ssh->channel = NULL;
	if ((client->ssh->session = ssh_new()) == NULL)
	{
		log_error("unable to initialize ssh session");
		return (SSH_RETURN_FAILURE);
	}

	ssh_options_set(client->ssh->session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	ssh_set_auth_methods(client->ssh->session, SSH_AUTH_METHOD_PASSWORD);
	ssh_set_message_callback(client->ssh->session, ssh_callback_request_message, client);

	return (SSH_RETURN_SUCCESS);
}

void ssh_terminate_session(client_t *client)
{
	if (client->ssh->exec_command_buffer != NULL)
		free(client->ssh->exec_command_buffer);
	client->ssh->exec_command_buffer = NULL;
	client->ssh->exec_command_buffer_len = 0;

	if (client->ssh->channel != NULL)
		ssh_channel_free(client->ssh->channel);
	ssh_silent_disconnect(client->ssh->session);
	ssh_free(client->ssh->session);
}
