#include <unistd.h>
#include <stdlib.h>
#include "shoxy.h"
#include "client.h"

client_t *client_create()
{
	client_t *client;

	if ((client = malloc(sizeof(client_t))) == NULL)
	{
		log_error("unable to allocate memory for client");
		return NULL;
	}

	if ((client->link = malloc(sizeof(network_t))) == NULL)
	{
		log_error("unable to allocate memory for client link");
		return NULL;
	}

	client->prev = NULL;
	client->next = NULL;

	return (client);
}

void client_delete(client_t *client)
{
	free(client->link);
	free(client);
}

client_t *client_find_by_socket(client_t *clients, int socket)
{
	client_t *client;
	for (client = clients; client != NULL && client->link->socket == socket; client = client->next)
		;

	if (client == NULL)
		log_error("critical devlopment error: a fd without client has changed, wtf ??");
	return (client);
}

void clients_list(client_t *clients)
{
	for (client_t *client = clients; client != NULL; client = client->next)
		log_debugf("client from %s:%lu with fd %d", client->link->host_ip, client->link->local_port, client->link->socket);
}

client_t *clients_add(client_t *clients, client_t *client_to_add)
{
	if (client_to_add == NULL)
	{
		log_error("trying to add a NULL client, refusing");
		return (clients);
	}

	// insert client_to_add to order client by socket number
	for (client_t *client = clients; client != NULL; client = client->next)
	{
		if (client_to_add->link->socket > client->link->socket)
			continue;
		client_to_add->prev = client->prev;
		client_to_add->next = client;
		if (client->prev != NULL)
			client->prev->next = client_to_add;
		client->prev = client_to_add;
	}

	// handle the case where there is no client before this client_to_add
	if (clients == NULL)
		return (client_to_add);

	// handle the case where the socket number is above every other clients socket
	if (client_to_add->next == NULL)
	{
		clients->next = client_to_add;
		client_to_add->prev = clients;
	}

	return (clients);
}

client_t *clients_remove(client_t *clients, client_t *client_to_remove)
{
	if (client_to_remove->prev != NULL)
		client_to_remove->prev->next = client_to_remove->next;
	if (client_to_remove->next != NULL)
		client_to_remove->next->prev = client_to_remove->prev;

	if (clients == client_to_remove)
		clients = clients->next;

	return (clients);
}
