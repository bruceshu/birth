/*
 bruce 2020-7-29
 */

#include "tsk_debug.h"
#include "tsk_common.h"


static const void *tsk_debug_arg_data = tsk_null;
static tsk_debug_f tsk_debug_info_cb = tsk_null;
static tsk_debug_f tsk_debug_warn_cb = tsk_null;
static tsk_debug_f tsk_debug_error_cb = tsk_null;
static tsk_debug_f tsk_debug_fatal_cb = tsk_null;
static int tsk_debug_level = DEBUG_LEVEL_INFO;


void tsk_debug_set_level (int level)
{
    tsk_debug_level = level;
}

int tsk_debug_get_level ()
{
    return tsk_debug_level;
}

void tsk_debug_set_info_cb (tsk_debug_f cb)
{
    tsk_debug_info_cb = cb;
}

tsk_debug_f tsk_debug_get_info_cb ()
{
    return tsk_debug_info_cb;
}

void tsk_debug_set_warn_cb (tsk_debug_f cb)
{
    tsk_debug_warn_cb = cb;
}

tsk_debug_f tsk_debug_get_warn_cb ()
{
    return tsk_debug_warn_cb;
}

void tsk_debug_set_error_cb (tsk_debug_f cb)
{
    tsk_debug_error_cb = cb;
}

tsk_debug_f tsk_debug_get_error_cb ()
{
    return tsk_debug_error_cb;
}

void tsk_debug_set_fatal_cb (tsk_debug_f cb)
{
    tsk_debug_fatal_cb = cb;
}

tsk_debug_f tsk_debug_get_fatal_cb ()
{
    return tsk_debug_fatal_cb;
}

void tsk_debug_set_arg_data (const void *arg_data)
{
    tsk_debug_arg_data = arg_data;
}

const void *tsk_debug_get_arg_data ()
{
    return tsk_debug_arg_data;
}