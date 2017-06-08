#include "include/typedef.h"

void log_printf(int level, char *tag, ...)
{
	int ret = OK;
	va_list vl;
	va_start(vl, tag);

	switch(level)
	{
		case VERBOSE:
			pirntf("verbose/%s(%s %s %d)", tag, __FILE__, __FUNCTION__, __LINE__);
			break;
		default:
			return ERR;
	}

	printf(vl);
	va_end(vl);

	return ret;
}
