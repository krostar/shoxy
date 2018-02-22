#include <libssh/libssh.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include "shoxy.h"
#include "client.h"
#include "ssh_command.h"

int ssh_callback_channel_on_data(UNUSED ssh_session session, UNUSED ssh_channel channel, void *data, uint32_t len, UNUSED int is_stderr, void *userdata)
{
	client_t *client = userdata;

	if (client->ssh->proxy != NULL)
	{
		ssh_command_answer(client, data, len);
		return (len);
	}
	// return ssh_channel_write(client->ssh->proxy->channel, data, len);

	client->ssh->exec_command_buffer = realloc(client->ssh->exec_command_buffer, client->ssh->exec_command_buffer_len + len);
	strncpy(client->ssh->exec_command_buffer + client->ssh->exec_command_buffer_len, data, len);
	client->ssh->exec_command_buffer_len += len;

	if (client->ssh->exec_command_buffer[client->ssh->exec_command_buffer_len - 1] == '\r')
	{
		ssh_command_answer(client, "\n\r", 2);
		if ((client->ssh->exec_command = ssh_command_find(client->ssh->exec_command_buffer, client->ssh->exec_command_buffer_len - 1)) == NULL)
		{
			log_client_error(client, "command not found");
			ssh_command_answer(client, "command not found\n\r", 19);
			ssh_command_prompt(client);
			free(client->ssh->exec_command_buffer);
			client->ssh->exec_command_buffer = NULL;
			client->ssh->exec_command_buffer_len = 0;
		}
	}
	else
		ssh_command_answer(client, data, len);

	return (len);
}
