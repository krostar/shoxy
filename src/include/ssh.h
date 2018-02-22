#ifndef SHOXY_SSH_H_
#define SHOXY_SSH_H_

#include <libssh/server.h>
#include <libssh/callbacks.h>
#include "ssh_command.h"

// this is actually used in others parts of the
// project so the define is prefixed with the
// namespace 'SSH' to avoid names conflicts.
#define SSH_RETURN_SUCCESS 0
#define SSH_RETURN_FAILURE -1
#define SSH_SERVICE_USERAUTH "ssh-userauth"

// defined in client.h
typedef struct client_s client_t;

typedef struct ssh_config_s
{
	ssh_session session;

	ssh_channel channel;
	struct ssh_channel_callbacks_struct channel_cb;

	char *exec_command_buffer;
	int exec_command_buffer_len;

	ssh_command_t *exec_command;

	char *exec_answer_buffer;
	int exec_answer_buffer_len;
} ssh_config_t;

int ssh_create_session(client_t *client, ssh_bind b);
void ssh_terminate_channel(client_t *client);
void ssh_terminate_session(client_t *client);

typedef struct message_callback_s
{
	int type;
	int subtype;
	int (*fct)(client_t *client, ssh_message msg);
} message_callback_t;

#endif
