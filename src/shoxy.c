#include <stdlib.h>
#include <stdio.h>
#include "shoxy.h"
#include "config.h"
#include "network.h"

int main(int ac, char **av)
{
	if (!signal_catch_interruption())
	{
		log_error("unable to catch interruption signal");
		return (EXIT_FAILURE);
	}

	if (config_parse_cli(ac, av) != CONFIG_RETURN_SUCCESS)
	{
		log_error("configuration parser failed");
		config_free();
		return (EXIT_FAILURE);
	}

	if (network_listen_and_serve(config_get_bind_addr(), config_get_bind_port()) != NETWORK_RETURN_SUCCESS)
	{
		log_error("an error occured while trying to listen and serve clients");
		config_free();
		return (EXIT_FAILURE);
	}

	log_info("server close without errors");
	config_free();
	return (EXIT_SUCCESS);
}
