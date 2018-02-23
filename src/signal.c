#include <signal.h>
#include "shoxy.h"
#include "config.h"

void signal_handler(int sig)
{
	log_info("signal %d has been catched", sig);

	switch (sig)
	{
	case SIGINT:
		log_error("killing signal intercepted, application will stop");
		RUNNING = 0;
		break;
	case SIGHUP:
		config_reload();
		break;
	default:
		log_error("unhandled signal %d", sig);
	}
}

int signal_catch_interruption()
{
	if (signal(SIGINT, &signal_handler) == SIG_ERR)
	{
		log_error("unable to setup interruption signal catching");
		return (0);
	}
	if (signal(SIGHUP, &signal_handler) == SIG_ERR)
	{
		log_error("unable to setup interruption signal catching");
		return (0);
	}
	if (signal(SIGPIPE, &signal_handler) == SIG_ERR)
	{
		log_error("unable to setup pipe error signal catching");
		return (0);
	}
	return (1);
}
