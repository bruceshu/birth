/*
 bruce 2020-7-29
 */


#include <sys/time.h>

#include "tsk_time.h"
#include "tsk_common.h"


int tsk_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	return gettimeofday(tv, tz);
}

uint64_t tsk_gettimeofday_ms()
{
	struct timeval tv;
	tsk_gettimeofday(&tv, tsk_null);
	return (((uint64_t)tv.tv_sec)*(uint64_t)1000) + (((uint64_t)tv.tv_usec)/(uint64_t)1000);
}