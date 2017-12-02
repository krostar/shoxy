#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

char *format_with_endline(const char *format)
{
	char *new_format;

	if ((new_format = malloc(sizeof(char) * (strlen(format) + 2))) == NULL)
	{
		return (strdup(format));
	}
	new_format = memset(new_format, '\0', strlen(format) + 2);
	new_format = strcpy(new_format, format);
	new_format = strcat(new_format, "\n");
	return (new_format);
}

void log_errorf(const char *format, ...)
{
	va_list args;
	char *new_format;

	new_format = format_with_endline(format);
	va_start(args, format);
	vdprintf(STDERR_FILENO, new_format, args);
	va_end(args);
	free(new_format);
}

void log_error(const char *message)
{
	log_errorf(message);
}

void log_infof(const char *format, ...)
{
	va_list args;
	char *new_format;

	new_format = format_with_endline(format);
	va_start(args, format);
	vdprintf(STDOUT_FILENO, new_format, args);
	va_end(args);
	free(new_format);
}

void log_info(const char *message)
{
	log_infof(message);
}

void log_debugf(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	log_infof(format, args);
	va_end(args);
}

void log_debug(const char *message)
{
	log_debugf(message);
}
