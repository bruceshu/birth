/*
 bruce 2020-7-28
 */


#ifndef TSK_LOG_H
#define TSK_LOG_H


enum TSK_LOG_LEVEL
{
    TSK_LOG_DISABLED = 0,
    TSK_LOG_FATAL = 1,
    TSK_LOG_ERROR = 10,
    TSK_LOG_WARNING = 20,
    TSK_LOG_INFO = 40,
    TSK_LOG_DEBUG = 50,
    TSK_LOG_VERBOSE = 60
};


#define TSK_LOG(level, ...) tsk_log_imp (__FUNCTION__, __FILE__, __LINE__, (level), __VA_ARGS__)







#endif