#ifndef SHOXY_SSH_PROXIFY_H_
#define SHOXY_SSH_PROXIFY_H_

#include "client.h"

int ssh_proxify(client_t *client, char *user, char *address, int port);

#endif
