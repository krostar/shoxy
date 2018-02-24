#include <stdlib.h>
#include <string.h>
#include <security/pam_appl.h>

int pam_callback_conv(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr)
{
	const struct pam_message *msg_ptr = *msg;
	*resp = calloc(sizeof(struct pam_response), num_msg);
	for (int x = 0; x < num_msg; x++, msg_ptr++)
		switch (msg_ptr->msg_style)
		{
		case PAM_PROMPT_ECHO_OFF:
			resp[x]->resp = strdup(appdata_ptr);
			break;
		}
	return PAM_SUCCESS;
}

int pam_authenticate_user(const char *username, const char *password)
{
	struct pam_conv conv;
	conv.conv = pam_callback_conv;
	conv.appdata_ptr = (char *)password;
	pam_handle_t *handle;
	int authenticated;

	if (pam_start("passwd", username, &conv, &handle) != PAM_SUCCESS)
		return (-1);

	authenticated = pam_authenticate(handle, PAM_SILENT) == PAM_SUCCESS ? 0 : -1;

	if (pam_end(handle, 0) != PAM_SUCCESS)
		return (-1);

	return (authenticated);
}
