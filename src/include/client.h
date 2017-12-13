#ifndef SHOXY_CLIENT_H_
#define SHOXY_CLIENT_H_

#include "network.h"
#include "ssh.h"

typedef struct client_s
{
	network_config_t *network;
	ssh_config_t *ssh;
	struct client_s *prev;
	struct client_s *next;
} client_t;

client_t *client_create();
void client_delete(client_t *client);
client_t *client_find_by_socket(client_t *clients, int socket);
void clients_list(client_t *clients);
client_t *clients_add(client_t *clients, client_t *client_to_add);
client_t *clients_remove(client_t *clients, client_t *client_to_remove);
#endif
