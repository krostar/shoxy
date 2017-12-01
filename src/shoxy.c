#include <stdlib.h>
#include <stdio.h>
#include "shoxy.h"
#include "network.h"

int main(int ac, char **av)
{
	if (ac != 3)
	{
		perror("usage: shoxy [address] [port]");
		return (EXIT_FAILURE);
	}

	if (listen_and_serve(av[1], (size_t)atoi(av[2])) != NETWORK_RETURN_SUCCESS)
	{
		perror("an error occured while trying to listen and serve clients");
		return (EXIT_FAILURE);
	}

	return (0);
}
