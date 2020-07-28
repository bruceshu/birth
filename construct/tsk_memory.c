/*
 bruce 2020-7-28
 */


#include <stdio.h>
#include "tsk_common.h"

void tsk_free(void** ptr)
{
	if(ptr && *ptr){
		free(*ptr);
		*ptr = tsk_null;
	}
}

void* tsk_malloc(tsk_size_t size)
{
	void *ret = malloc(size);
	if(!ret){
		printf("Memory allocation failed\n");
	}

	return ret;
}

void* tsk_realloc (void* ptr, tsk_size_t size)
{
	void *ret = tsk_null;
	
	if(size) {
		if(ptr){
			if(!(ret = realloc(ptr, size))){
				printf("Memory reallocation failed\n");
			}
		}
		else{
			if(!(ret = calloc(size, 1))){
				printf("Memory allocation (%u) failed\n", (unsigned)size);
			}
		}
	}

	return ret;
}

void* tsk_calloc(tsk_size_t num, tsk_size_t size)
{
	void* ret = tsk_null;
	if(num && size){
		ret = calloc(num, size);
		if(!ret){
			TSK_DEBUG_ERROR("Memory allocation failed. num=%u and size=%u", (unsigned)num, (unsigned)size);
		}
	}

	return ret;
}
