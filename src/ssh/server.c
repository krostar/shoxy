#include <unistd.h>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include "shoxy.h"
#include "client.h"
#include "network.h"
#include "ssh.h"

// int cb(ssh_session session, ssh_message msg, void *data)
// {
// 	log_debugf("coucou message type: %p", msg);
// 	return (1);
// }

int ssh_create_session(client_t *client)
{
	int verbosity = SSH_LOG_FUNCTIONS;

	if ((client->ssh->session = ssh_new()) == NULL)
	{
		log_error("unable to initialize ssh session");
		return (SSH_RETURN_FAILURE);
	}

	if ((client->ssh->bind = ssh_bind_new()) == NULL)
	{
		log_error("unable to initialize ssh session");
		ssh_free(client->ssh->session);
		return (SSH_RETURN_FAILURE);
	}

	ssh_bind_options_set(client->ssh->bind, SSH_BIND_OPTIONS_DSAKEY, "build/keys/ssh_host_dsa_key");
	ssh_bind_options_set(client->ssh->bind, SSH_BIND_OPTIONS_RSAKEY, "build/keys/ssh_host_rsa_key");
	ssh_options_set(client->ssh->session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	// ssh_set_message_callback(client->ssh->session, cb, client);

	// set des callbacks pour tout ?

	if (ssh_bind_accept_fd(client->ssh->bind, client->ssh->session, client->network->socket) != SSH_OK)
	{
		log_errorf("unable to initialize ssh session: %s", ssh_get_error(client->ssh->bind));
		ssh_terminate_session(client);
		return (SSH_RETURN_FAILURE);
	}

	return (SSH_RETURN_SUCCESS);
}

void ssh_terminate_session(client_t *client)
{
	ssh_silent_disconnect(client->ssh->session);
	ssh_free(client->ssh->session);
	ssh_bind_free(client->ssh->bind);
}
