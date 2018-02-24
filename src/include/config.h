#ifndef SHOXY_CONFIG_H_
#define SHOXY_CONFIG_H_

#include <unistd.h>
#include "shoxy.h"

#define CONFIG_RETURN_SUCCESS 0
#define CONFIG_RETURN_FAILURE -1

#define CONFIG_DEFAULT_BIND_ADDR "127.0.0.1"
#define CONFIG_DEFAULT_BIND_PORT 2222
#define CONFIG_DEFAULT_VERBOSITY LOG_VERBOSITY_DEBUG
#define CONFIG_DEFAULT_SSH_KEY_RSA "/etc/shoxy/keys/ssh_host_rsa_key"
#define CONFIG_DEFAULT_SSH_KEY_DSA "/etc/shoxy/keys/ssh_host_dsa_key"

typedef struct config_connect_remote_s
{
	char *user;
	char *password;
	char *addr;
	int port;
} config_connect_remote_t;

typedef struct config_host_user_s
{
	char *user;
	char *password;
} config_host_user_t;

typedef struct config_host_s
{
	char *addr;
	size_t port;
	config_host_user_t **users;
} config_host_t;

typedef struct config_right_s
{
	char *user;
	config_host_t **hosts;
} config_right_t;

typedef struct config_s
{
	char *config_file; // not reloadable

	int verbosity;			 // reloadable via SIGHUP
	char *bind_addr;		 // not reloadable
	size_t bind_port;		 // not reloadable
	char *ssh_key_rsa;		 // not reloadable
	char *ssh_key_dsa;		 // not reloadable
	config_right_t **rights; // reloadable via SIGHUP
} config_t;

int config_parse_cli(int ac, char **av);
int config_parse_file(char *filename);
void config_free();
void config_reload();
int config_get_verbosity();
char *config_get_bind_addr();
size_t config_get_bind_port();
char *config_get_ssh_key_rsa();
char *config_get_ssh_key_dsa();
config_right_t **config_get_rights();
void config_rights_list(char *output, char *user_logged);
int config_rights_check(config_connect_remote_t *remote, char *user_logged, char *hostname, char *user_remote);

#endif
