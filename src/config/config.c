#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shoxy.h"
#include "config.h"

static config_t *CONFIG = NULL;

void config_free()
{
	config_free_parsed(CONFIG);
	CONFIG = NULL;
}

void config_show(config_t *config)
{
	log_debug("\tConfiguration file: %s", config->config_file);
	log_debug("\tVerbosity: %d", config->verbosity);
	log_debug("\tBind address: %s", config->bind_addr);
	log_debug("\tBind port: %d", config->bind_port);
	log_debug("\tSSH key rsa path: %s", config->ssh_key_rsa);
	log_debug("\tSSH key dsa path: %s", config->ssh_key_dsa);

	for (int i = 0; config->_hosts != NULL && config->_hosts[i] != NULL; i++)
	{
		log_debug("\tHost (%s:%d)", config->_hosts[i]->addr, config->_hosts[i]->port);
		for (int j = 0; config->_hosts[i]->users != NULL && config->_hosts[i]->users[j] != NULL; j++)
			log_debug("\t\tUser (%s:%s)", config->_hosts[i]->users[j]->user, config->_hosts[i]->users[j]->password);
	}

	for (int i = 0; config->rights != NULL && config->rights[i] != NULL; i++)
	{
		log_debug("\tUser %s", config->rights[i]->user);
		for (int j = 0; config->rights[i]->hosts != NULL && config->rights[i]->hosts[j] != NULL; j++)
		{
			log_debug("\t\ton host (%s:%d)", config->rights[i]->hosts[j]->addr, config->rights[i]->hosts[j]->port);
			for (int k = 0; config->rights[i]->hosts[j]->users != NULL && config->rights[i]->hosts[j]->users[k] != NULL; k++)
				log_debug("\t\t\tcan log as (%s:%s)", config->rights[i]->hosts[j]->users[k]->user, config->rights[i]->hosts[j]->users[k]->password);
		}
	}
}

int config_parse_cli(UNUSED int ac, UNUSED char **av)
{
	if (ac == 2)
	{
		if ((CONFIG = config_parse_file(av[1])) != NULL)
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

void config_reload()
{
	config_t *new;
	if (CONFIG != NULL)
	{
		if ((new = config_parse_file(CONFIG->config_file)) != NULL)
			log_error("unable to parse file %s", CONFIG->config_file);
		free(CONFIG);
		CONFIG = new;
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
	return (CONFIG != NULL ? (CONFIG->bind_addr != NULL ? CONFIG->bind_addr : CONFIG_DEFAULT_BIND_ADDR) : CONFIG_DEFAULT_BIND_ADDR);
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
