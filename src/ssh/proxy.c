#include <libssh/libssh.h>
#include <libssh/callbacks.h>
#include "shoxy.h"
#include "client.h"
#include "ssh_proxy.h"

int ssh_proxify(client_t *client, UNUSED char *user, char *address, int port)
{
	int verbosity = SSH_LOG_FUNCTIONS;

	if ((client->ssh->proxy = malloc(sizeof(ssh_proxy_config_t))) == NULL)
	{
		log_client_error(client, "unable to initiate proxy");
		return (SSH_RETURN_FAILURE);
	}

	if ((client->ssh->proxy->session = ssh_new()) == NULL)
	{
		log_client_error(client, "unable to create proxified session");
		return (SSH_RETURN_FAILURE);
	}
	ssh_options_set(client->ssh->proxy->session, SSH_OPTIONS_HOST, address);
	ssh_options_set(client->ssh->proxy->session, SSH_OPTIONS_PORT, &port);
	ssh_options_set(client->ssh->proxy->session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

	if (ssh_connect(client->ssh->proxy->session) != SSH_OK)
	{
		log_client_error(client, "unable to connect to proxified session: %s", ssh_get_error(client->ssh->proxy->session));
		return (SSH_RETURN_FAILURE);
	}

	if (ssh_userauth_password(client->ssh->proxy->session, "krostar", "mdpendurolol") != SSH_AUTH_SUCCESS)
	{
		log_client_error(client, "unable to authenticate user with password: %s", ssh_get_error(client->ssh->proxy->session));
		return (SSH_RETURN_FAILURE);
	}

	if ((client->ssh->proxy->channel = ssh_channel_new(client->ssh->proxy->session)) == NULL)
	{
		log_client_error(client, "unable to create channel");
		return (SSH_RETURN_FAILURE);
	}

	client->ssh->proxy->channel_cb.channel_data_function = &ssh_proxy_callback_channel_on_data;
	client->ssh->proxy->channel_cb.userdata = client;
	ssh_callbacks_init(&client->ssh->proxy->channel_cb);
	ssh_set_channel_callbacks(client->ssh->proxy->channel, &client->ssh->proxy->channel_cb);

	if (ssh_channel_open_session(client->ssh->proxy->channel) != SSH_OK)
	{
		log_client_error(client, "unable to open session for channel: %s", ssh_get_error(client->ssh->proxy->channel));
		return (SSH_RETURN_FAILURE);
	}

	if (ssh_channel_request_pty(client->ssh->proxy->channel) != SSH_OK)
	{
		log_client_error(client, "unable to request pty: %s", ssh_get_error(client->ssh->proxy->channel));
		return (SSH_RETURN_FAILURE);
	}

	if (ssh_channel_change_pty_size(client->ssh->proxy->channel, 80, 24) != SSH_OK)
	{
		log_client_error(client, "unable change pty size: %s", ssh_get_error(client->ssh->proxy->channel));
		return (SSH_RETURN_FAILURE);
	}

	if (ssh_event_add_session(client->event, client->ssh->proxy->session) != SSH_OK)
	{
		log_client_error(client, "unable to add proxy session to ssh poll event: %s", ssh_get_error(client->event));
		return (SSH_RETURN_FAILURE);
	}

	if (ssh_channel_request_shell(client->ssh->proxy->channel) != SSH_OK)
	{
		log_client_error(client, "unable to request shell: %s", ssh_get_error(client->ssh->proxy->channel));
		return (SSH_RETURN_FAILURE);
	}

	return (SSH_RETURN_SUCCESS);
}

void ssh_unproxify(client_t *client)
{
	if (client->ssh->proxy == NULL)
		return;

	if (client->ssh->proxy->channel != NULL)
	{
		if (ssh_channel_send_eof(client->ssh->proxy->channel) != SSH_OK)
			log_client_error(client, "unable to send eof to channel: %s", ssh_get_error(client->ssh->proxy->channel));
		if (ssh_channel_close(client->ssh->proxy->channel) != SSH_OK)
			log_client_error(client, "unable to close channel: %s", ssh_get_error(client->ssh->proxy->channel));
		ssh_channel_free(client->ssh->proxy->channel);
	}

	ssh_silent_disconnect(client->ssh->proxy->session);
	ssh_free(client->ssh->proxy->session);
	free(client->ssh->proxy);
	client->ssh->proxy = NULL;
}

int ssh_proxy_callback_channel_on_data(UNUSED ssh_session session, ssh_channel channel, void *data, uint32_t len, UNUSED int is_stderr, void *userdata)
{
	client_t *client = userdata;
	log_client_debug(client, "proxified received data %.*s", len, data);
	return (ssh_channel_write(client->ssh->channel, data, len));
}

void ssh_proxy_callback_channel_on_close(UNUSED ssh_session session, UNUSED ssh_channel channel, void *userdata)
{
	client_t *client = userdata;
	log_client_debug(client, "proxified connexion closed");
}