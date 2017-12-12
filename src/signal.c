#include <signal.h>
#include "shoxy.h"

void signal_handler(int sig)
{
	log_infof("signal %d has been catched", sig);

	// we only want to handle CTRL+C
	if (sig != SIGINT)
		return;

	log_error("killing signal intercepted, application will stop");
	RUNNING = 0;
}

int signal_catch_interruption()
{
	if (signal(SIGINT, &signal_handler) == SIG_ERR)
	{
		log_error("unable to setup interruption signal catching");
		return (0);
	}
	return (1);
}
