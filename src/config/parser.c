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

	if (nb <= len)
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

		log_debug("new user: %s=%s", config->_hosts[nb]->users[len]->user, config->_hosts[nb]->users[len]->password);
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

	// log_debug("host -> ")
	confg_parser_set_host(config, host_nb, host_key, host_value);

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
	// char *raw = NULL;
	// size_t raw_allocated = 0;
	// int len;
	// FILE *file;
	// int err = 0;

	// config_parser_t parser[] = {
	// 	{"bind_addr", 9, &config_parser_bind_addr},
	// 	{"bind_port", 9, &config_parser_bind_port},
	// 	{"verbosity", 9, &config_parser_verbosity},
	// 	{"ssh_key_rsa", 11, &config_parser_ssh_key_rsa},
	// 	{"ssh_key_dsa", 11, &config_parser_ssh_key_dsa},
	// 	{"host", 4, &config_parser_host},
	// 	{NULL, 0, NULL},
	// };

	// if ((file = fopen(file_name, "r")) == NULL)
	// {
	// 	log_error("unable to open %s: %s", file_name, strerror(errno));
	// 	return (NULL);
	// }

	// log_debug("parsing configuration file: %s...", file_name);

	// if ((parsed = malloc(sizeof(config_t))) == NULL)
	// {
	// 	log_error("unable to allocate memory for configuration");
	// 	return (NULL);
	// }

	// parsed->config_file = strdup(file_name);
	// parsed->rights = NULL;
	// parsed->_hosts = NULL;

	// while ((len = getline(&raw, &raw_allocated, file)) != -1)
	// {
	// 	len -= 1;
	// 	if (len <= 0)
	// 		continue;
	// 	raw[len] = '\0';
	// 	for (int i = 0; parser[i].key != NULL; i++)
	// 	{
	// 		if (strncmp(raw, parser[i].key, parser[i].key_len) != 0)
	// 			continue;
	// 		if (parser[i].fct(parsed, raw + parser[i].key_len, len - parser[i].key_len) == CONFIG_RETURN_FAILURE)
	// 		{
	// 			log_error("fail to parse configuration: \"%s\"", raw);
	// 			err = 1;
	// 		}
	// 		break;
	// 	}
	// }
	// free(raw);
	// fclose(file);

	// if (err == 0)
	// 	return (parsed);
	// config_free_parsed(parsed);

	if ((parsed = malloc(sizeof(config_t))) == NULL)
	{
		log_error("unable to allocate memory for configuration");
		return (NULL);
	}

	parsed->config_file = strdup(file_name);
	parsed->verbosity = LOG_VERBOSITY_DEBUG;
	parsed->bind_addr = strdup("127.0.0.1");
	parsed->bind_port = 2222;
	parsed->ssh_key_rsa = strdup("./build/keys/ssh_host_rsa_key");
	parsed->ssh_key_dsa = strdup("./build/keys/ssh_host_dsa_key");

	if (parsed->config_file == NULL || parsed->bind_addr == NULL)
	{
		log_error("unable to allocate memory for configuration variables");
		return (NULL);
	}

	parsed->_hosts = malloc(sizeof(config_host_t *) * 3);
	parsed->_hosts[0] = malloc(sizeof(config_host_t));
	parsed->_hosts[0]->addr = strdup("127.0.0.1");
	parsed->_hosts[0]->port = 2022;
	parsed->_hosts[0]->users = malloc(sizeof(config_host_user_t *) * 3);
	parsed->_hosts[0]->users[0] = malloc(sizeof(config_host_user_t));
	parsed->_hosts[0]->users[0]->user = strdup("root");
	parsed->_hosts[0]->users[0]->password = strdup("passworduserrootofhost0");
	parsed->_hosts[0]->users[1] = malloc(sizeof(config_host_user_t));
	parsed->_hosts[0]->users[1]->user = strdup("krostar");
	parsed->_hosts[0]->users[1]->password = strdup("passwordkrostar");
	parsed->_hosts[0]->users[2] = NULL;
	parsed->_hosts[1] = malloc(sizeof(config_host_t));
	parsed->_hosts[1]->addr = strdup("host1.domain");
	parsed->_hosts[1]->port = 42;
	parsed->_hosts[1]->users = malloc(sizeof(config_host_user_t *) * 2);
	parsed->_hosts[1]->users[0] = malloc(sizeof(config_host_user_t));
	parsed->_hosts[1]->users[0]->user = strdup("titi");
	parsed->_hosts[1]->users[0]->password = strdup("passwordtiti");
	parsed->_hosts[1]->users[1] = NULL;
	parsed->_hosts[2] = NULL;

	parsed->rights = malloc(sizeof(config_right_t *) * 3);
	parsed->rights[0] = malloc(sizeof(config_right_t));
	parsed->rights[0]->user = strdup("bob");
	parsed->rights[0]->hosts = malloc(sizeof(config_host_t *) * 2);
	parsed->rights[0]->hosts[0] = malloc(sizeof(config_host_t));
	parsed->rights[0]->hosts[0]->addr = strdup(parsed->_hosts[0]->addr);
	parsed->rights[0]->hosts[0]->port = parsed->_hosts[0]->port;
	parsed->rights[0]->hosts[0]->users = malloc(sizeof(config_host_user_t *) * 3);
	parsed->rights[0]->hosts[0]->users[0] = malloc(sizeof(config_host_user_t));
	parsed->rights[0]->hosts[0]->users[0]->user = strdup(parsed->_hosts[0]->users[0]->user);
	parsed->rights[0]->hosts[0]->users[0]->password = strdup(parsed->_hosts[0]->users[0]->password);
	parsed->rights[0]->hosts[0]->users[1] = malloc(sizeof(config_host_user_t));
	parsed->rights[0]->hosts[0]->users[1]->user = strdup(parsed->_hosts[0]->users[1]->user);
	parsed->rights[0]->hosts[0]->users[1]->password = strdup(parsed->_hosts[0]->users[1]->password);
	parsed->rights[0]->hosts[0]->users[2] = NULL;
	parsed->rights[0]->hosts[1] = NULL;

	parsed->rights[1] = malloc(sizeof(config_right_t));
	parsed->rights[1]->user = strdup("krostar");
	parsed->rights[1]->hosts = malloc(sizeof(config_host_t *) * 3);
	parsed->rights[1]->hosts[0] = malloc(sizeof(config_host_t));
	parsed->rights[1]->hosts[0]->addr = strdup(parsed->_hosts[0]->addr);
	parsed->rights[1]->hosts[0]->port = parsed->_hosts[0]->port;
	parsed->rights[1]->hosts[0]->users = malloc(sizeof(config_host_user_t *) * 2);
	parsed->rights[1]->hosts[0]->users[0] = malloc(sizeof(config_host_user_t));
	parsed->rights[1]->hosts[0]->users[0]->user = strdup(parsed->_hosts[0]->users[0]->user);
	parsed->rights[1]->hosts[0]->users[0]->password = strdup(parsed->_hosts[0]->users[0]->password);
	parsed->rights[1]->hosts[0]->users[1] = NULL;
	parsed->rights[1]->hosts[1] = malloc(sizeof(config_host_t));
	parsed->rights[1]->hosts[1]->addr = strdup(parsed->_hosts[1]->addr);
	parsed->rights[1]->hosts[1]->port = parsed->_hosts[1]->port;
	parsed->rights[1]->hosts[1]->users = malloc(sizeof(config_host_user_t *) * 2);
	parsed->rights[1]->hosts[1]->users[0] = malloc(sizeof(config_host_user_t));
	parsed->rights[1]->hosts[1]->users[0]->user = strdup(parsed->_hosts[1]->users[0]->user);
	parsed->rights[1]->hosts[1]->users[0]->password = strdup(parsed->_hosts[1]->users[0]->password);
	parsed->rights[1]->hosts[1]->users[1] = NULL;
	parsed->rights[1]->hosts[2] = NULL;
	parsed->rights[2] = NULL;

	return (parsed);
}
