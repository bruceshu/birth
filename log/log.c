#include <stdio.h>
#include <stdarg.h>


#include "log.h"

static int g_log_level = INFO;

void set_log_level(int level)
{
	g_log_level = level;
}

static void vlog(int level, const char *fmt, va_list vl)
{
	char line[LINE_SIZE];
	if(level > g_log_level)
	{
		return;
	}

	snprintf(line, sizeof(line), fmt, vl);
	printf("bruce >>>>>> %s", line);
}

void debug_log(int level, const char *fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);

	vlog(level, fmt, vl);
	va_end(vl);
}
