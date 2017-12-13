#ifndef SHOXY_SSH_H_
#define SHOXY_SSH_H_

#include <libssh/server.h>

// this is actually used in others parts of the
// project so the define is prefixed with the
// namespace 'SSH' to avoid names conflicts.
#define SSH_RETURN_SUCCESS 0
#define SSH_RETURN_FAILURE -1

// defined in client.h
typedef struct client_s client_t;

typedef struct ssh_config_s
{
	ssh_session session;
	ssh_bind bind;
} ssh_config_t;

int ssh_create_session(client_t *client);
void ssh_terminate_session(client_t *client);

#endif
