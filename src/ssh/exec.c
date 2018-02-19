#include "shoxy.h"
#include "ssh.h"
#include "ssh_exec.h"

int ssh_exec_help(client_t *client, UNUSED char *remaining, UNUSED int remaining_len)
{
	log_client_debug(client, "exec help function call");
	return (SSH_RETURN_SUCCESS);
}

int ssh_exec_connect(client_t *client, UNUSED char *remaining, int remaining_len)
{
	log_client_debug(client, "exec connect function call, remaining = %d", remaining_len);
	return (SSH_RETURN_SUCCESS);
}

int ssh_exec_list(client_t *client, UNUSED char *remaining, UNUSED int remaining_len)
{
	log_client_debug(client, "exec list function call");
	return (SSH_RETURN_SUCCESS);
}

int ssh_exec_exit(client_t *client, UNUSED char *remaining, UNUSED int remaining_len)
{
	log_client_debug(client, "exec exit function call");
	return (SSH_RETURN_SUCCESS);
}

static ssh_exec_command_t commands[] = {
	{"help", 4, &ssh_exec_help},
	{"connect", 7, &ssh_exec_connect},
	{"list", 4, &ssh_exec_list},
	{"exit", 4, &ssh_exec_exit},
	{NULL, 0, NULL},
};

ssh_exec_command_t *ssh_exec_find_command(char *data, int len)
{
	for (int i = 0; commands[i].name != NULL; i++)
		if (len >= commands[i].name_len && strncmp(data, commands[i].name, commands[i].name_len) == 0)
			return (&commands[i]);
	return (NULL);
}
