#include "shoxy.h"

void config_usage_cli(char *binary_name)
{
	log_info("usage: %s [config_file]\nexample: %s /etc/shoxy/shoxy.cfg", binary_name, binary_name);
}

void config_usage_file(char *file_name)
{
	log_info("file %s must be formatted like the following example:\n\
	verbosity=1\n\
	bind_addr=127.0.0.1\n\
	bind_port=2222\n\
	\n\
	\n\
	host0_addr=host0.domain\n\
	host0_port=42\n\
	host0_user_root=passworduserrootofhost0\n\
	host0_user_krostar=passwordkrostar\n\
	\n\
	host1_addr=host1.domain\n\
	host1_port=24\n\
	host1_user_root=passworduserrootofhost1\n\
	host1_user_titi=passwordtiti\n\
	\n\
	\n\
	rights_bob=host*_user_bob\n\
	rights_alice=host1_user_titi,host0_user_root\n\
	rights_krostar=host0_user_*\n\
	rights_admin=host*_user_*",
			 file_name);
}
