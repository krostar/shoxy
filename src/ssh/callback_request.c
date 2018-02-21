#include <string.h>
#include <errno.h>
#include <pty.h>
#include <poll.h>
#include <libssh/libssh.h>
#include <libssh/callbacks.h>
#include "shoxy.h"
#include "client.h"
#include "ssh.h"
#include "ssh_callback.h"
#include "ssh_command.h"

int ssh_callback_request_service_default(client_t *client, ssh_message msg)
{
	log_client_debug(client, "new service request: \"%s\"", ssh_message_service_service(msg));

	if (strcmp(ssh_message_service_service(msg), SSH_SERVICE_USERAUTH) != 0)
		return (SSH_RETURN_FAILURE);

	ssh_message_service_reply_success(msg);
	return (SSH_RETURN_SUCCESS);
}

int ssh_callback_request_auth_default(client_t *client, UNUSED ssh_message msg)
{
	log_client_debug(client, "auth request is unhandled and will be denied");
	return (SSH_RETURN_FAILURE);
}

int ssh_callback_request_auth_password(client_t *client, ssh_message msg)
{
	const char *username = ssh_message_auth_user(msg);
	const char *password = ssh_message_auth_password(msg);

	log_client_debug(client, "new password try: \"%s:%s\"", username, password);

	if (0)
	{
		log_client_error(client, "bad login: \"%s\"", ssh_message_auth_user(msg));
		return (SSH_RETURN_FAILURE);
	}

	log_client_info(client, "successfully authenticated as \"%s\"", ssh_message_auth_user(msg));
	ssh_message_auth_reply_success(msg, 0);
	return (SSH_RETURN_SUCCESS);
}

int ssh_callback_request_channel_pty(client_t *client, UNUSED ssh_message msg)
{
	log_client_debug(client, "new channel pty request");
	ssh_message_channel_request_reply_success(msg);
	return (SSH_RETURN_SUCCESS);
}

int ssh_callback_request_channel_shell(client_t *client, UNUSED ssh_message msg)
{
	log_client_debug(client, "new channel shell request");

	ssh_message_channel_request_reply_success(msg);
	return (SSH_RETURN_SUCCESS);
}

int ssh_callback_request_channel_open(client_t *client, ssh_message msg)
{
	log_client_debug(client, "new channel session request");

	client->ssh->channel = ssh_message_channel_request_open_reply_accept(msg);
	client->ssh->channel_cb.userdata = client;
	client->ssh->channel_cb.channel_data_function = &ssh_callback_channel_on_data;
	ssh_callbacks_init(&client->ssh->channel_cb);
	if (ssh_set_channel_callbacks(client->ssh->channel, &client->ssh->channel_cb) != SSH_OK)
	{
		log_client_error(client, "unable to set channel callbacks: %s", ssh_get_error(msg));
		ssh_channel_free(client->ssh->channel);
		return (SSH_RETURN_FAILURE);
	}
	client->ssh->close_channel = 0;
	client->ssh->exec_command_buffer_len = 0;
	client->ssh->exec_command_buffer = NULL;
	client->ssh->exec_answer_buffer_len = 0;
	client->ssh->exec_answer_buffer = NULL;
	ssh_command_welcome(client);
	return (SSH_RETURN_SUCCESS);
}

int ssh_callback_request_message(UNUSED ssh_session session, ssh_message msg, void *userdata)
{
	message_callback_t fcts[] = {
		{SSH_REQUEST_SERVICE, -1, &ssh_callback_request_service_default},

		{SSH_REQUEST_AUTH, -1, &ssh_callback_request_auth_default},
		{SSH_REQUEST_AUTH, SSH_AUTH_METHOD_PASSWORD, &ssh_callback_request_auth_password},

		{SSH_REQUEST_CHANNEL, SSH_CHANNEL_REQUEST_PTY, &ssh_callback_request_channel_pty},
		{SSH_REQUEST_CHANNEL, SSH_CHANNEL_REQUEST_SHELL, &ssh_callback_request_channel_shell},

		{SSH_REQUEST_CHANNEL_OPEN, SSH_CHANNEL_SESSION, &ssh_callback_request_channel_open},
		{-1, -1, NULL},
	};
	client_t *client = userdata;
	int message_type = ssh_message_type(msg);
	int message_subtype = ssh_message_subtype(msg);

	int founded = -1;
	message_callback_t *default_cb = NULL;
	int rc = SSH_RETURN_FAILURE;

	log_client_debug(client, "new message: types: \"%d;%d\"", message_type, message_subtype);
	for (int i = 0; fcts[i].type != -1; i++)
	{
		if (message_type == fcts[i].type && fcts[i].subtype == -1)
			default_cb = &fcts[i];

		if (message_type == fcts[i].type && message_subtype == fcts[i].subtype)
		{
			rc = fcts[i].fct(client, msg);
			founded = 1;
			break;
		}
	}

	if (founded == -1 && default_cb != NULL)
		rc = (*default_cb).fct(client, msg);

	if (rc != SSH_RETURN_SUCCESS)
	{
		log_client_error(client, "unhandled message or response failure");
		ssh_message_reply_default(msg);
	}

	return (SSH_OK);
}
