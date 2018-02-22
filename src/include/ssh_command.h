#ifndef SHOXY_SSH_COMMAND_H_
#define SHOXY_SSH_COMMAND_H_

// defined in client.h
typedef struct client_s client_t;

typedef int (*ssh_command_fct_t)(client_t *client, char *remaining, int remaining_len);

typedef struct ssh_command_s
{
	char *name;
	int name_len;
	ssh_command_fct_t fct;
	char *help;
} ssh_command_t;

ssh_command_t *ssh_command_find(char *data, int len);
void ssh_command_prompt(client_t *client);
void ssh_command_welcome(client_t *client);
void ssh_command_answer(client_t *client, void *data, int len);
void ssh_command_exec_if_needed(client_t *client);
int ssh_command_help(client_t *client, char *remaining, int remaining_len);
int ssh_command_connect(client_t *client, char *remaining, int remaining_len);
int ssh_command_list(client_t *client, char *remaining, int remaining_len);
int ssh_command_exit(client_t *client, char *remaining, int remaining_len);

#endif
