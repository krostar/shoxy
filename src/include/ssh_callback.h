#ifndef SHOXY_SSH_CALLBACK_H_
#define SHOXY_SSH_CALLBACK_H_

#include <libssh/libssh.h>

int ssh_callback_request_message(ssh_session session, ssh_message msg, void *data);
int ssh_callback_channel_on_data(ssh_session session, ssh_channel channel, void *data, uint32_t len, int is_stderr, void *userdata);
void ssh_callback_channel_on_close(ssh_session session, ssh_channel channel, void *userdata);

#endif
