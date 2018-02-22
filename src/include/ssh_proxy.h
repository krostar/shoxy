#ifndef SHOXY_SSH_PROXY_H_
#define SHOXY_SSH_PROXY_H_

#include <libssh/libssh.h>
#include <libssh/callbacks.h>

// defined in client.h
typedef struct client_s client_t;

typedef struct ssh_proxy_config_s
{
	ssh_session session;

	ssh_channel channel;
	struct ssh_channel_callbacks_struct channel_cb;

	char *exec_answer_buffer;
	int exec_answer_buffer_len;
} ssh_proxy_config_t;

int ssh_proxify(client_t *client, char *user, char *address, int port);
void ssh_unproxify(client_t *client);
int ssh_proxy_callback_channel_on_data(ssh_session session, ssh_channel channel, void *data, uint32_t len, int is_stderr, void *userdata);
void ssh_proxy_callback_channel_on_close(ssh_session session, ssh_channel channel, void *userdata);

#endif
