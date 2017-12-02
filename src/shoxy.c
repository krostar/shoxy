#include <stdlib.h>
#include <stdio.h>
#include "shoxy.h"
#include "network.h"

int main(int ac, char **av)
{
	if (ac != 3)
	{
		log_error("usage: shoxy [address] [port]");
		return (EXIT_FAILURE);
	}

	if (listen_and_serve(av[1], (size_t)atoi(av[2])) != NETWORK_RETURN_SUCCESS)
	{
		log_error("an error occured while trying to listen and serve clients");
		return (EXIT_FAILURE);
	}

	log_info("server close without errors");
	return (0);
}