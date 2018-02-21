#include <stdio.h>
#include <libssh/server.h>
#include "shoxy.h"
#include "ssh.h"
#include "ssh_command.h"
#include "ssh_proxify.h"

int ssh_command_help(client_t *client, UNUSED char *remaining, UNUSED int remaining_len)
{
	log_client_debug(client, "exec help function call");
	return (SSH_RETURN_SUCCESS);
}

int ssh_command_connect(client_t *client, UNUSED char *remaining, int remaining_len)
{
	log_client_debug(client, "exec connect function call, remaining = %d", remaining_len);
	return (ssh_proxify(client, "127.0.0.1", 2022));
}

int ssh_command_list(client_t *client, UNUSED char *remaining, UNUSED int remaining_len)
{
	log_client_debug(client, "exec list function call");
	return (SSH_RETURN_SUCCESS);
}

int ssh_command_exit(client_t *client, UNUSED char *remaining, UNUSED int remaining_len)
{
	log_client_debug(client, "exec exit function call");
	client->ssh->close_channel = 1;
	ssh_command_answer(client, "exiting...\n\r", 12);
	return (SSH_RETURN_SUCCESS);
}

static ssh_command_t commands[] = {
	{"help", 4, &ssh_command_help},
	{"connect", 7, &ssh_command_connect},
	{"list", 4, &ssh_command_list},
	{"exit", 4, &ssh_command_exit},
	{NULL, 0, NULL},
};

ssh_command_t *ssh_command_find(char *data, int len)
{
	for (int i = 0; commands[i].name != NULL; i++)
		if (len >= commands[i].name_len && strncmp(data, commands[i].name, commands[i].name_len) == 0)
			return (&commands[i]);
	return (NULL);
}

void ssh_command_welcome(client_t *client)
{
	char to_send[512];

	sprintf(to_send, "You successfully connected to shoxy as %s, type help if you need something\n\r$>", "anon");
	ssh_command_answer(client, to_send, strlen(to_send));
}

void ssh_command_answer(client_t *client, void *data, int len)
{
	client->ssh->exec_answer_buffer = realloc(client->ssh->exec_answer_buffer, client->ssh->exec_answer_buffer_len + len);
	memcpy(client->ssh->exec_answer_buffer + client->ssh->exec_answer_buffer_len, data, len);
	client->ssh->exec_answer_buffer_len += len;
}
