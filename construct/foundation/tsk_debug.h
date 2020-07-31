/*
 bruce 2020-7-28
 */


#ifndef TSK_DEBUG_H
#define TSK_DEBUG_H

#include "tsk_config.h"

TSK_BEGIN_DECLS

#define DEBUG_LEVEL_INFO 4
#define DEBUG_LEVEL_WARN 3
#define DEBUG_LEVEL_ERROR 2
#define DEBUG_LEVEL_FATAL 1

typedef int (*tsk_debug_f)(const void *arg, const char *fmt, ...);

#define TSK_DEBUG_INFO(FMT, ...)                                                                            \
    if (tsk_debug_get_level () >= DEBUG_LEVEL_INFO)                                                         \
    {                                                                                                       \
        if (tsk_debug_get_info_cb ())                                                                       \
        {                                                                                                   \
            tsk_debug_get_info_cb ()(tsk_debug_get_arg_data (), "*[YOUME INFO]: " FMT "\n", ##__VA_ARGS__); \
        }                                                                                                   \
        else                                                                                                \
        {                                                                                                   \
            TSK_LOG (TSK_LOG_INFO, "" FMT, ##__VA_ARGS__);                                                  \
        }                                                                                                   \
    }


#define TSK_DEBUG_WARN(FMT, ...)                                                                   \
    if (tsk_debug_get_level () >= DEBUG_LEVEL_WARN)                                                \
    {                                                                                              \
        if (tsk_debug_get_warn_cb ())                                                              \
        {                                                                                          \
            tsk_debug_get_warn_cb ()(tsk_debug_get_arg_data (), "**[YOUME WARN]: function: "       \
                                                                "\"%s()\" \nfile: \"%s\" \nline: " \
                                                                "\"%u\" \nMSG: " FMT "\n",         \
                                     __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);             \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            TSK_LOG (TSK_LOG_WARNING, "" FMT, ##__VA_ARGS__);                                      \
        }                                                                                          \
    }


#define TSK_DEBUG_ERROR(FMT, ...)                                                                                       \
    if (tsk_debug_get_level () >= DEBUG_LEVEL_ERROR)                                                                    \
    {                                                                                                                   \
        if (tsk_debug_get_error_cb ())                                                                                  \
        {                                                                                                               \
            tsk_debug_get_error_cb ()(tsk_debug_get_arg_data (), "***[YOUME ERROR]: function: \"%s()\" \nfile: \"%s\" " \
                                                                 "\nline: \"%u\" \nMSG: " FMT "\n",                     \
                                      __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                                 \
        }                                                                                                               \
        else                                                                                                            \
        {                                                                                                               \
            TSK_LOG (TSK_LOG_ERROR, "" FMT, ##__VA_ARGS__);                                                             \
        }                                                                                                               \
    }


#define TSK_DEBUG_FATAL(FMT, ...)                                                                                        \
    if (tsk_debug_get_level () >= DEBUG_LEVEL_FATAL)                                                                     \
    {                                                                                                                    \
        if (tsk_debug_get_fatal_cb ())                                                                                   \
        {                                                                                                                \
            tsk_debug_get_fatal_cb ()(tsk_debug_get_arg_data (), "****[YOUME FATAL]: function: \"%s()\" \nfile: \"%s\" " \
                                                                 "\nline: \"%u\" \nMSG: " FMT "\n",                      \
                                      __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);                                  \
        }                                                                                                                \
        else                                                                                                             \
        {                                                                                                                \
            TSK_LOG (TSK_LOG_FATAL, "" FMT, ##__VA_ARGS__);                                                              \
        }                                                                                                                \
    }


TSK_END_DECLS


#endif