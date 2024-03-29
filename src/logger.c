#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "shoxy.h"
#include "config.h"
#include "client.h"

char *format_with_endline(const char *format, const char *verbosity)
{
	char *new_format;
	int format_len = strlen(format);
	int verbosity_len = strlen(verbosity);
	time_t now = time(0);

	// allocate some space for date and time, verbosity and carriage return
	if ((new_format = malloc(sizeof(char) * (20 + format_len + verbosity_len + 6))) == NULL)
		return (NULL);
	new_format = memset(new_format, '\0', 20 + format_len + verbosity_len + 6);

	strftime(new_format, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));

	new_format = strcat(new_format, " |");
	new_format = strcat(new_format, verbosity);
	new_format = strcat(new_format, "| ");
	new_format = strcat(new_format, format);
	new_format = strcat(new_format, "\n");
	return (new_format);
}

void log_verror(const char *format, va_list args)
{
	char *new_format;

	if (config_get_verbosity() < LOG_VERBOSITY_ERROR)
		return;
	new_format = format_with_endline(format, "ERROR");
	vdprintf(STDERR_FILENO, new_format, args);
	free(new_format);
}

void log_vinfo(const char *format, va_list args)
{
	char *new_format;

	if (config_get_verbosity() < LOG_VERBOSITY_INFO)
		return;
	new_format = format_with_endline(format, " INFO");
	vdprintf(STDOUT_FILENO, new_format, args);
	free(new_format);
}

void log_vdebug(const char *format, va_list args)
{
	char *new_format;

	if (config_get_verbosity() < LOG_VERBOSITY_DEBUG)
		return;
	new_format = format_with_endline(format, "DEBUG");
	vdprintf(STDOUT_FILENO, new_format, args);
	free(new_format);
}

void log_error(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	log_verror(format, args);
	va_end(args);
}

void log_info(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	log_vinfo(format, args);
	va_end(args);
}

void log_debug(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	log_vdebug(format, args);
	va_end(args);
}

char *log_format_client(client_t *client, const char *format)
{
	char *new_format;

	if ((new_format = malloc(sizeof(char) * (16 + sizeof(int) + sizeof(long unsigned int) + sizeof(int) + strlen(format) + 1))) == NULL)
		return (NULL);

	sprintf(new_format, "client %s:%lu (fd %d): %s", client->network->host_ip, client->network->local_port, client->network->socket, format);
	return (new_format);
}

void log_client_error(client_t *client, const char *format, ...)
{
	char *new_format = log_format_client(client, format);
	va_list args;

	if (new_format == NULL)
		puts("LOGGING ERROR");

	va_start(args, format);
	log_verror(new_format, args);
	va_end(args);
	free(new_format);
}

void log_client_info(client_t *client, const char *format, ...)
{
	char *new_format = log_format_client(client, format);
	va_list args;

	if (new_format == NULL)
		puts("LOGGING ERROR");

	va_start(args, format);
	log_vinfo(new_format, args);
	va_end(args);
	free(new_format);
}

void log_client_debug(client_t *client, const char *format, ...)
{
	char *new_format = log_format_client(client, format);
	va_list args;

	if (new_format == NULL)
		puts("LOGGING ERROR");

	va_start(args, format);
	log_vdebug(new_format, args);
	va_end(args);
	free(new_format);
}
