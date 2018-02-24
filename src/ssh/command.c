#include <stdio.h>
#include <libssh/server.h>
#include "shoxy.h"
#include "client.h"
#include "ssh.h"
#include "config.h"
#include "ssh_command.h"
#include "ssh_proxy.h"

static ssh_command_t commands[] = {
	{"help", 4, &ssh_command_help, "display this help"},
	{"connect", 7, &ssh_command_connect, "param: user@host, connect to host 'host' as 'user'"},
	{"list", 4, &ssh_command_list, "list the differents endpoint and user"},
	{"exit", 4, &ssh_command_exit, "quit shoxy"},
	{NULL, 0, NULL, NULL},
};

// if the buffer contains a known command, return it
ssh_command_t *ssh_command_find(char *data, int len)
{
	for (int i = 0; commands[i].name != NULL; i++)
		if (len >= commands[i].name_len && strncmp(data, commands[i].name, commands[i].name_len) == 0)
			return (&commands[i]);
	return (NULL);
}

void ssh_command_prompt(client_t *client)
{
	ssh_command_answer(client, "$>", 2);
}

void ssh_command_welcome(client_t *client)
{
	char to_send[512];

	sprintf(to_send, "You successfully connected to shoxy as %s, type help if you need something\n\r", "anon");
	ssh_command_answer(client, to_send, strlen(to_send));
	ssh_command_prompt(client);
}

void ssh_command_answer(client_t *client, void *data, int len)
{
	client->ssh->exec_answer_buffer = realloc(client->ssh->exec_answer_buffer, client->ssh->exec_answer_buffer_len + len);
	memcpy(client->ssh->exec_answer_buffer + client->ssh->exec_answer_buffer_len, data, len);
	client->ssh->exec_answer_buffer_len += len;
}

void ssh_command_exec_if_needed(client_t *client)
{
	if (client->ssh->exec_command == NULL)
		return;
	if (client->ssh->exec_command->fct(client, client->ssh->exec_command_buffer + client->ssh->exec_command->name_len, client->ssh->exec_command_buffer_len - client->ssh->exec_command->name_len - 1))
	{
		log_client_error(client, "failed to execute command");
		ssh_command_answer(client, "failed to execute command\n\r", 27);
		ssh_command_prompt(client);
	}
	client->ssh->exec_command = NULL;
	free(client->ssh->exec_command_buffer);
	client->ssh->exec_command_buffer = NULL;
	client->ssh->exec_command_buffer_len = 0;
}

int ssh_command_check(char **remaining, int *remaining_len)
{
	// is there so remaining caracteres and is it normal ?
	if (*remaining_len > 0)
	{
		if (**remaining != ' ')
			return (SSH_RETURN_FAILURE);
		else
		{
			(*remaining_len)--;
			(*remaining)++;
		}
	}
	return (SSH_RETURN_SUCCESS);
}

int ssh_command_help(client_t *client, char *remaining, int remaining_len)
{
	char raw_help[1024];
	log_client_debug(client, "exec help function call, remaining %d = %.*s", remaining_len, remaining_len, remaining);
	if (ssh_command_check(&remaining, &remaining_len) != SSH_RETURN_SUCCESS)
		return (SSH_RETURN_FAILURE);
	for (int i = 0; commands[i].name != NULL; i++)
	{
		sprintf(raw_help, "%s\n\r\t%s\n\r", commands[i].name, commands[i].help);
		ssh_command_answer(client, raw_help, strlen(raw_help));
	}
	ssh_command_prompt(client);
	return (SSH_RETURN_SUCCESS);
}

int ssh_command_connect(client_t *client, char *remaining, int remaining_len)
{
	config_connect_remote_t remote;
	char user[512];
	char hostname[512];
	log_client_debug(client, "exec connect function call, remaining %d = %.*s", remaining_len, remaining_len, remaining);
	if (ssh_command_check(&remaining, &remaining_len) != SSH_RETURN_SUCCESS)
		return (SSH_RETURN_FAILURE);

	memset(user, '\0', 512);
	memset(hostname, '\0', 512);
	for (int i = 0; i < remaining_len; i++)
	{
		if (remaining[i] == '@' && (i < remaining_len - 1))
		{
			strncpy(user, remaining, i);
			strncpy(hostname, remaining + i + 1, remaining_len - i - 1);
			break;
		}
	}

	if (user[0] == '\0' || hostname[0] == '\0')
		return (SSH_RETURN_FAILURE);

	log_client_debug(client, "trying to connect to host '%s' as '%s'", hostname, user);
	if (config_rights_check(&remote, client->ssh->username, hostname, user) != 0)
	{
		log_client_error(client, "not authorized to log as %s on %s", user, hostname);
		return (SSH_RETURN_FAILURE);
	}

	if (ssh_proxify(client, remote.user, remote.password, remote.addr, remote.port) == SSH_RETURN_SUCCESS)
		return (SSH_RETURN_SUCCESS);
	ssh_unproxify(client);
	return (SSH_RETURN_FAILURE);
}

int ssh_command_list(client_t *client, char *remaining, int remaining_len)
{
	char list[4096];

	log_client_debug(client, "exec list function call, remaining %d = %.*s", remaining_len, remaining_len, remaining);
	if (ssh_command_check(&remaining, &remaining_len) != SSH_RETURN_SUCCESS)
		return (SSH_RETURN_FAILURE);
	config_rights_list(list, client->ssh->username);
	log_client_debug(client, "rights list: %s", list);
	ssh_command_answer(client, list, strlen(list));
	ssh_command_prompt(client);
	return (SSH_RETURN_SUCCESS);
}

int ssh_command_exit(client_t *client, char *remaining, int remaining_len)
{
	log_client_debug(client, "exec exit function call, remaining %d = %.*s", remaining_len, remaining_len, remaining);
	if (ssh_command_check(&remaining, &remaining_len) != SSH_RETURN_SUCCESS)
		return (SSH_RETURN_FAILURE);
	ssh_command_answer(client, "exiting...\n\r", 12);
	ssh_terminate_channel(client);
	return (SSH_RETURN_SUCCESS);
}
