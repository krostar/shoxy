#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shoxy.h"
#include "config.h"

static config_t *CONFIG = NULL;

void config_usage_cli(char *binary_name)
{
	log_info("usage: %s [config_file]\nexample: %s /etc/shoxy/shoxy.cfg", binary_name, binary_name);
}

void config_usage_file(char *file_name)
{
	log_info("file %s must be formatted like the following example:\n\
	verbosity=1\n\
	bind_addr=127.0.0.1\n\
	bind_port=2222\n\
	\n\
	\n\
	host0_addr=host0.domain\n\
	host0_port=42\n\
	host0_user_root=passworduserrootofhost0\n\
	host0_user_krostar=passwordkrostar\n\
	\n\
	host1_addr=host1.domain\n\
	host1_port=24\n\
	host1_user_root=passworduserrootofhost1\n\
	host1_user_titi=passwordtiti\n\
	\n\
	\n\
	rights_bob=host*_user_bob\n\
	rights_alice=host1_user_titi,host0_user_root\n\
	rights_krostar=host0_user_*\n\
	rights_admin=host*_user_*",
			 file_name);
}

int config_parse_cli(UNUSED int ac, UNUSED char **av)
{
	if (ac == 2)
	{
		if (config_parse_file(av[1]) == CONFIG_RETURN_SUCCESS)
		{
			log_info("config successfully updated");
			return (CONFIG_RETURN_SUCCESS);
		}
		else
			config_usage_file(av[1]);
	}
	else if (ac >= 1)
		config_usage_cli(av[0]);
	else
		config_usage_cli("./shoxy");

	return (CONFIG_RETURN_FAILURE);
}

void config_host_free(config_host_t **hosts)
{
	if (hosts != NULL)
	{
		for (int i = 0; hosts[i] != NULL; i++)
		{
			free(hosts[i]->addr);
			for (int j = 0; hosts[i]->users[j] != NULL; j++)
			{
				free(hosts[i]->users[j]->user);
				free(hosts[i]->users[j]->password);
				free(hosts[i]->users[j]);
			}
			if (hosts[i]->users != NULL)
				free(hosts[i]->users);
			free(hosts[i]);
		}
		free(hosts);
	}
}

void config_free()
{
	if (CONFIG == NULL)
		return;
	free(CONFIG->config_file);
	free(CONFIG->bind_addr);
	free(CONFIG->ssh_key_rsa);
	free(CONFIG->ssh_key_dsa);
	if (CONFIG->rights != NULL)
	{
		for (int i = 0; CONFIG->rights[i] != NULL; i++)
		{
			free(CONFIG->rights[i]->user);
			config_host_free(CONFIG->rights[i]->hosts);
			free(CONFIG->rights[i]);
		}
		free(CONFIG->rights);
	}

	free(CONFIG);
	CONFIG = NULL;
}

int config_parse_file(char *file_name)
{
	log_debug("parsing configuration file: %s...", file_name);

	if ((CONFIG = malloc(sizeof(config_t))) == NULL)
	{
		log_error("unable to allocate memory for configuration");
		return (CONFIG_RETURN_FAILURE);
	}

	CONFIG->config_file = strdup(file_name);
	CONFIG->verbosity = 2;
	CONFIG->bind_addr = strdup("127.0.0.1");
	CONFIG->bind_port = 2222;
	CONFIG->ssh_key_rsa = strdup("./build/keys/ssh_host_rsa_key");
	CONFIG->ssh_key_dsa = strdup("./build/keys/ssh_host_dsa_key");

	if (CONFIG->config_file == NULL || CONFIG->bind_addr == NULL)
	{
		log_error("unable to allocate memory for configuration variables");
		return (CONFIG_RETURN_FAILURE);
	}

	config_host_t **hosts = malloc(sizeof(config_host_t *) * 3);
	hosts[0] = malloc(sizeof(config_host_t));
	hosts[0]->addr = strdup("127.0.0.1");
	hosts[0]->port = 2022;
	hosts[0]->users = malloc(sizeof(config_host_user_t *) * 3);
	hosts[0]->users[0] = malloc(sizeof(config_host_user_t));
	hosts[0]->users[0]->user = strdup("root");
	hosts[0]->users[0]->password = strdup("passworduserrootofhost0");
	hosts[0]->users[1] = malloc(sizeof(config_host_user_t));
	hosts[0]->users[1]->user = strdup("krostar");
	hosts[0]->users[1]->password = strdup("passwordkrostar");
	hosts[0]->users[2] = NULL;
	hosts[1] = malloc(sizeof(config_host_t));
	hosts[1]->addr = strdup("host1.domain");
	hosts[1]->port = 42;
	hosts[1]->users = malloc(sizeof(config_host_user_t *) * 2);
	hosts[1]->users[0] = malloc(sizeof(config_host_user_t));
	hosts[1]->users[0]->user = strdup("titi");
	hosts[1]->users[0]->password = strdup("passwordtiti");
	hosts[1]->users[1] = NULL;
	hosts[2] = NULL;

	CONFIG->rights = malloc(sizeof(config_right_t *) * 3);
	CONFIG->rights[0] = malloc(sizeof(config_right_t));
	CONFIG->rights[0]->user = strdup("bob");
	CONFIG->rights[0]->hosts = malloc(sizeof(config_host_t *) * 2);
	CONFIG->rights[0]->hosts[0] = malloc(sizeof(config_host_t));
	CONFIG->rights[0]->hosts[0]->addr = strdup(hosts[0]->addr);
	CONFIG->rights[0]->hosts[0]->port = hosts[0]->port;
	CONFIG->rights[0]->hosts[0]->users = malloc(sizeof(config_host_user_t *) * 3);
	CONFIG->rights[0]->hosts[0]->users[0] = malloc(sizeof(config_host_user_t));
	CONFIG->rights[0]->hosts[0]->users[0]->user = strdup(hosts[0]->users[0]->user);
	CONFIG->rights[0]->hosts[0]->users[0]->password = strdup(hosts[0]->users[0]->password);
	CONFIG->rights[0]->hosts[0]->users[1] = malloc(sizeof(config_host_user_t));
	CONFIG->rights[0]->hosts[0]->users[1]->user = strdup(hosts[0]->users[1]->user);
	CONFIG->rights[0]->hosts[0]->users[1]->password = strdup(hosts[0]->users[1]->password);
	CONFIG->rights[0]->hosts[0]->users[2] = NULL;
	CONFIG->rights[0]->hosts[1] = NULL;

	CONFIG->rights[1] = malloc(sizeof(config_right_t));
	CONFIG->rights[1]->user = strdup("alice");
	CONFIG->rights[1]->hosts = malloc(sizeof(config_host_t *) * 3);
	CONFIG->rights[1]->hosts[0] = malloc(sizeof(config_host_t));
	CONFIG->rights[1]->hosts[0]->addr = strdup(hosts[0]->addr);
	CONFIG->rights[1]->hosts[0]->port = hosts[0]->port;
	CONFIG->rights[1]->hosts[0]->users = malloc(sizeof(config_host_user_t *) * 2);
	CONFIG->rights[1]->hosts[0]->users[0] = malloc(sizeof(config_host_user_t));
	CONFIG->rights[1]->hosts[0]->users[0]->user = strdup(hosts[0]->users[0]->user);
	CONFIG->rights[1]->hosts[0]->users[0]->password = strdup(hosts[0]->users[0]->password);
	CONFIG->rights[1]->hosts[0]->users[1] = NULL;
	CONFIG->rights[1]->hosts[1] = malloc(sizeof(config_host_t));
	CONFIG->rights[1]->hosts[1]->addr = strdup(hosts[1]->addr);
	CONFIG->rights[1]->hosts[1]->port = hosts[1]->port;
	CONFIG->rights[1]->hosts[1]->users = malloc(sizeof(config_host_user_t *) * 2);
	CONFIG->rights[1]->hosts[1]->users[0] = malloc(sizeof(config_host_user_t));
	CONFIG->rights[1]->hosts[1]->users[0]->user = strdup(hosts[1]->users[0]->user);
	CONFIG->rights[1]->hosts[1]->users[0]->password = strdup(hosts[1]->users[0]->password);
	CONFIG->rights[1]->hosts[1]->users[1] = NULL;
	CONFIG->rights[1]->hosts[2] = NULL;
	CONFIG->rights[2] = NULL;

	config_host_free(hosts);

	return (CONFIG_RETURN_SUCCESS);
}

void config_reload()
{
	if (CONFIG != NULL)
	{
		if (config_parse_file(CONFIG->config_file) != CONFIG_RETURN_SUCCESS)
			log_error("unable to parse file %s", CONFIG->config_file);
	}
	else
		log_info("no config file to reload");
}

int config_get_verbosity()
{
	return (CONFIG != NULL ? CONFIG->verbosity : CONFIG_DEFAULT_VERBOSITY);
}

char *config_get_bind_addr()
{
	return (CONFIG != NULL ? CONFIG->bind_addr : CONFIG_DEFAULT_BIND_ADDR);
}

size_t config_get_bind_port()
{
	return (CONFIG != NULL ? CONFIG->bind_port : CONFIG_DEFAULT_BIND_PORT);
}

char *config_get_ssh_key_rsa()
{
	return (CONFIG != NULL ? CONFIG->ssh_key_rsa : CONFIG_DEFAULT_SSH_KEY_RSA);
}

char *config_get_ssh_key_dsa()
{
	return (CONFIG != NULL ? CONFIG->ssh_key_dsa : CONFIG_DEFAULT_SSH_KEY_DSA);
}

config_right_t **config_get_rights()
{
	return (CONFIG != NULL ? CONFIG->rights : NULL);
}

void config_rights_list(char *output, char *user_logged)
{
	size_t base_len = sprintf(output, "%s is allowed to log in with theses user@host:\n\r", user_logged);
	for (int i = 0; CONFIG->rights[i] != NULL; i++)
	{
		if (strcmp(CONFIG->rights[i]->user, user_logged) != 0)
			continue;
		if (CONFIG->rights[i]->hosts != NULL)
			for (int j = 0; CONFIG->rights[i]->hosts[j] != NULL; j++)
				if (CONFIG->rights[i]->hosts[j]->users != NULL)
					for (int k = 0; CONFIG->rights[i]->hosts[j]->users[k] != NULL; k++)
						sprintf(output + strlen(output), "%s@%s\n\r", CONFIG->rights[i]->hosts[j]->users[k]->user, CONFIG->rights[i]->hosts[j]->addr);
		break;
	}
	if (strlen(output) == base_len)
		sprintf(output, "no combinaison user/host matches %s's rights, contact your administrator!\n\r", user_logged);
}

int config_rights_check(config_connect_remote_t *remote, char *user_logged, char *hostname, char *user_remote)
{
	for (int i = 0; CONFIG->rights[i] != NULL; i++)
	{
		if (strcmp(CONFIG->rights[i]->user, user_logged) != 0)
			continue;
		if (CONFIG->rights[i]->hosts != NULL)
			for (int j = 0; CONFIG->rights[i]->hosts[j] != NULL; j++)
			{
				if (strcmp(CONFIG->rights[i]->hosts[j]->addr, hostname) != 0)
					continue;
				if (CONFIG->rights[i]->hosts[j]->users != NULL)
					for (int k = 0; CONFIG->rights[i]->hosts[j]->users[k] != NULL; k++)
					{
						if (strcmp(CONFIG->rights[i]->hosts[j]->users[k]->user, user_remote) != 0)
							continue;
						remote->addr = CONFIG->rights[i]->hosts[j]->addr;
						remote->port = CONFIG->rights[i]->hosts[j]->port;
						remote->user = CONFIG->rights[i]->hosts[j]->users[k]->user;
						remote->password = CONFIG->rights[i]->hosts[j]->users[k]->password;
						return (0);
					}
				break;
			}
		break;
	}

	return (-1);
}
