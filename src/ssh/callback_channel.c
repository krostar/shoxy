#include <libssh/libssh.h>
#include <stdlib.h>
#include <string.h>
#include "shoxy.h"
#include "client.h"
#include "ssh_exec.h"

int ssh_callback_channel_on_data(UNUSED ssh_session session, UNUSED ssh_channel channel, void *data, uint32_t len, UNUSED int is_stderr, void *userdata)
{
	client_t *client = userdata;
	ssh_exec_command_t *command;

	client->ssh->exec_command_buffer = realloc(client->ssh->exec_command_buffer, client->ssh->exec_command_buffer_len + len);
	strncpy(client->ssh->exec_command_buffer + client->ssh->exec_command_buffer_len, data, len);
	client->ssh->exec_command_buffer_len += len;

	if (client->ssh->exec_command_buffer[client->ssh->exec_command_buffer_len - 1] == '\r')
	{
		if ((command = ssh_exec_find_command(client->ssh->exec_command_buffer, client->ssh->exec_command_buffer_len - 1)) == NULL)
			log_client_error(client, "command not found");
		else if (command->fct(client, client->ssh->exec_command_buffer + command->name_len + 1, client->ssh->exec_command_buffer_len - command->name_len - 1) != SSH_RETURN_SUCCESS)
			log_client_error(client, "failed to execute command");
		free(client->ssh->exec_command_buffer);
		client->ssh->exec_command_buffer = NULL;
		client->ssh->exec_command_buffer_len = 0;
		ssh_channel_write(client->ssh->channel, "\r\n", 2);
	}
	else
		ssh_channel_write(client->ssh->channel, data, len);

	return (len);
}

void ssh_callback_channel_on_close(UNUSED ssh_session session, ssh_channel channel, void *userdata)
{
	client_t *client = userdata;

	log_client_debug(client, "closing channel");
	if (client->ssh->exec_command_buffer != NULL)
		free(client->ssh->exec_command_buffer);
	client->ssh->exec_command_buffer = NULL;
	client->ssh->exec_command_buffer_len = 0;
	ssh_channel_free(channel);
}
