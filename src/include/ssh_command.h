#ifndef SHOXY_SSH_COMMAND_H_
#define SHOXY_SSH_COMMAND_H_

#include "client.h"

typedef int (*ssh_command_fct_t)(client_t *client, char *remaining, int remaining_len);

typedef struct ssh_command_s
{
	char *name;
	int name_len;
	ssh_command_fct_t fct;
} ssh_command_t;

ssh_command_t *ssh_command_find(char *data, int len);
void ssh_command_welcome(client_t *client);
void ssh_command_answer(client_t *client, void *data, int len);

#endif
