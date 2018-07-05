/*
copyright bruceshu

author:bruceshu
date:2018-07-05
description:

*/


#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "log.h"

static int g_log_level = LOG_INFO;

void set_log_level(int level)
{
	g_log_level = level;
}

void debug_log(int flag, int level, const char *fmt, ...)
{	
	va_list vl;
	char log_buffer[LOG_LINE_SIZE] = {0};
	
	//module switch	
	if (!flag) {
		return;
	}

	//the more serious the problem, the lower the value of level
	if (level > g_log_level) {
		return;
	}

	strcpy(log_buffer, TAG);

	va_start(vl, fmt);
	vsnprintf(log_buffer + strlen(log_buffer), LOG_LINE_SIZE - strlen(log_buffer), fmt, vl);
	va_end(vl);

	printf("%s\n", log_buffer);
}

const char *av_default_item_name(void *ptr)
{
    return (*(AVClass **) ptr)->class_name;
}

