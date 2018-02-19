#ifndef SHOXY_SSH_EXEC_H_
#define SHOXY_SSH_EXEC_H_

#include "client.h"

typedef int (*ssh_exec_command_fct_t)(client_t *client, char *remaining, int remaining_len);

typedef struct exec_command_s
{
	char *name;
	int name_len;
	ssh_exec_command_fct_t fct;
} ssh_exec_command_t;

ssh_exec_command_t *ssh_exec_find_command(char *data, int len);

#endif
