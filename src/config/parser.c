#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "shoxy.h"
#include "config.h"

int config_parser_bind_addr(config_t *config, char *remaining, int remaining_len)
{
	if (remaining_len <= 1)
		return (CONFIG_RETURN_FAILURE);
	remaining++;
	config->bind_addr = strdup(remaining);
	return (CONFIG_RETURN_SUCCESS);
}

int config_parser_bind_port(config_t *config, char *remaining, int remaining_len)
{
	if (remaining_len <= 1)
		return (CONFIG_RETURN_FAILURE);
	remaining++;
	config->bind_port = atoi(remaining);
	return (CONFIG_RETURN_SUCCESS);
}

int config_parser_verbosity(config_t *config, char *remaining, int remaining_len)
{
	if (remaining_len <= 1)
		return (CONFIG_RETURN_FAILURE);
	remaining++;
	config->verbosity = atoi(remaining);
	return (CONFIG_RETURN_SUCCESS);
}

int config_parser_ssh_key_rsa(config_t *config, char *remaining, int remaining_len)
{
	if (remaining_len <= 1)
		return (CONFIG_RETURN_FAILURE);
	remaining++;
	config->ssh_key_rsa = strdup(remaining);
	return (CONFIG_RETURN_SUCCESS);
}

int config_parser_ssh_key_dsa(config_t *config, char *remaining, int remaining_len)
{
	if (remaining_len <= 1)
		return (CONFIG_RETURN_FAILURE);
	remaining++;
	config->ssh_key_dsa = strdup(remaining);
	return (CONFIG_RETURN_SUCCESS);
}

void confg_parser_set_host(config_t *config, int nb, char *key, char *value)
{
	int len;

	for (len = 0; config->_hosts != NULL && config->_hosts[len] != NULL; len++)
		;

	if (nb == len)
	{
		config->_hosts = realloc(config->_hosts, sizeof(config_host_t *) * (nb + 2));
		config->_hosts[nb] = NULL;
		config->_hosts[nb + 1] = NULL;
	}

	// we are now sure we have enough space in the array

	if (config->_hosts[nb] == NULL)
	{
		config->_hosts[nb] = malloc(sizeof(config_host_t));
		config->_hosts[nb]->users = NULL;
	}

	// we are now sure we can write on host[nb]

	if (strcmp(key, "addr") == 0)
		config->_hosts[nb]->addr = strdup(value);
	else if (strcmp(key, "port") == 0)
		config->_hosts[nb]->port = atoi(value);
	else if (strncmp(key, "user_", 5) == 0)
	{
		for (len = 0; config->_hosts[nb]->users != NULL && config->_hosts[nb]->users[len] != NULL; len++)
			;

		config->_hosts[nb]->users = realloc(config->_hosts[nb]->users, sizeof(config_host_user_t *) * (len + 2));
		config->_hosts[nb]->users[len + 1] = NULL;
		config->_hosts[nb]->users[len] = malloc(sizeof(config_host_user_t));
		config->_hosts[nb]->users[len]->user = strdup(key + 5);
		config->_hosts[nb]->users[len]->password = strdup(value);
	}
}

int config_parser_host(config_t *config, char *remaining, int remaining_len)
{
	int host_nb;
	char host_key[42];
	char host_value[42];
	int i;
	int j;

	if (remaining_len <= 3)
		return (CONFIG_RETURN_FAILURE);

	host_nb = atoi(remaining);
	for (i = 0; remaining[i] != '_'; i++)
		;
	i++;
	for (j = 0; remaining[i + j] != '='; j++)
		;
	strncpy(host_key, remaining + i, j);
	strcpy(host_value, remaining + i + j + 1);
	host_key[j] = '\0';

	confg_parser_set_host(config, host_nb, host_key, host_value);

	return (CONFIG_RETURN_SUCCESS);
}

void conf_parser_set_right2(config_right_t *right, config_host_t *host, char *user_to_match)
{
	int len;
	int len2;
	for (len = 0; right->hosts != NULL && right->hosts[len] != NULL; len++)
		;
	config_host_t *already_host;

	for (int i = 0; host->users != NULL && host->users[i] != NULL; i++)
	{
		if (user_to_match[0] == '*' || strcmp(user_to_match, host->users[i]->user) == 0)
		{
			already_host = NULL;
			for (int j = 0; right->hosts != NULL && right->hosts[j] != NULL; j++)
				if (strcmp(right->hosts[j]->addr, host->addr) == 0 && right->hosts[j]->port == host->port)
					already_host = right->hosts[j];
			if (already_host == NULL)
			{
				right->hosts = realloc(right->hosts, sizeof(config_host_t *) * (len + 2));
				right->hosts[len + 1] = NULL;
				right->hosts[len] = malloc(sizeof(config_host_t));
				right->hosts[len]->addr = strdup(host->addr);
				right->hosts[len]->port = host->port;
				right->hosts[len]->users = NULL;
				already_host = right->hosts[len];
			}

			for (len2 = 0; already_host->users != NULL && already_host->users[len2] != NULL; len2++)
				;

			already_host->users = realloc(already_host->users, sizeof(config_host_user_t *) * (len2 + 2));
			already_host->users[len2 + 1] = NULL;
			already_host->users[len2] = malloc(sizeof(config_host_user_t));
			already_host->users[len2]->user = strdup(host->users[i]->user);
			already_host->users[len2]->password = strdup(host->users[i]->password);
		}
	}
}

void conf_parser_set_right(config_t *config, char *username, char *rights)
{
	int len;
	char *right;
	int nb;
	int i;
	int count;
	char user[42];

	for (len = 0; config->rights != NULL && config->rights[len] != NULL; len++)
		;
	config->rights = realloc(config->rights, sizeof(config_right_t *) * (len + 2));
	config->rights[len + 1] = NULL;
	config->rights[len] = malloc(sizeof(config_right_t));
	config->rights[len]->user = strdup(username);
	config->rights[len]->hosts = NULL;

	if ((right = strtok(rights, ",")) != NULL)
	{
		do
		{
			nb = atoi(right + 4);
			count = 0;
			for (i = 5; count != 2; i++)
				if (right[i] == '_')
					count++;
			strcpy(user, right + i);

			if (right[4] == '*')
				for (int j = 0; config->_hosts[j] != NULL; j++)
				{
					conf_parser_set_right2(config->rights[len], config->_hosts[j], user);
				}
			else
			{
				conf_parser_set_right2(config->rights[len], config->_hosts[nb], user);
			}
		} while ((right = strtok(NULL, ",")) != NULL);
	}
}

int config_parser_right(config_t *config, char *remaining, UNUSED int remaining_len)
{
	int i;
	char user[42];
	char rights[512];

	for (i = 0; remaining[i] != '='; i++)
		;
	strncpy(user, remaining, i);
	strcpy(rights, remaining + i + 1);
	user[i] = '\0';

	conf_parser_set_right(config, user, rights);

	return (CONFIG_RETURN_SUCCESS);
}

void config_free_parsed(config_t *config)
{
	if (config == NULL)
		return;
	config_host_free(config->_hosts);
	free(config->config_file);
	free(config->bind_addr);
	free(config->ssh_key_rsa);
	free(config->ssh_key_dsa);
	if (config->rights != NULL)
	{
		for (int i = 0; config->rights[i] != NULL; i++)
		{
			free(config->rights[i]->user);
			config_host_free(config->rights[i]->hosts);
			free(config->rights[i]);
		}
		free(config->rights);
	}

	free(config);
}

config_t *config_parse_file(char *file_name)
{
	config_t *parsed;
	char *raw = NULL;
	size_t raw_allocated = 0;
	int len;
	FILE *file;
	int err = 0;

	config_parser_t parser[] = {
		{"bind_addr", 9, &config_parser_bind_addr},
		{"bind_port", 9, &config_parser_bind_port},
		{"verbosity", 9, &config_parser_verbosity},
		{"ssh_key_rsa", 11, &config_parser_ssh_key_rsa},
		{"ssh_key_dsa", 11, &config_parser_ssh_key_dsa},
		{"host", 4, &config_parser_host},
		{"rights_", 7, &config_parser_right},
		{NULL, 0, NULL},
	};

	if ((file = fopen(file_name, "r")) == NULL)
	{
		log_error("unable to open %s: %s", file_name, strerror(errno));
		return (NULL);
	}

	log_debug("parsing configuration file: %s...", file_name);

	if ((parsed = malloc(sizeof(config_t))) == NULL)
	{
		log_error("unable to allocate memory for configuration");
		return (NULL);
	}

	parsed->config_file = strdup(file_name);
	parsed->rights = NULL;
	parsed->_hosts = NULL;

	while ((len = getline(&raw, &raw_allocated, file)) != -1)
	{
		len -= 1;
		if (len <= 0)
			continue;
		raw[len] = '\0';
		for (int i = 0; parser[i].key != NULL; i++)
		{
			if (strncmp(raw, parser[i].key, parser[i].key_len) != 0)
				continue;
			if (parser[i].fct(parsed, raw + parser[i].key_len, len - parser[i].key_len) == CONFIG_RETURN_FAILURE)
			{
				log_error("fail to parse configuration: \"%s\"", raw);
				err = 1;
			}
			break;
		}
	}
	free(raw);
	fclose(file);

	if (err == 0)
	{
		config_show(parsed);
		return (parsed);
	}

	config_free_parsed(parsed);
	return (NULL);
}
