/*
copyright bruceshu

author:bruceshu
date:2018-07-05
description:

*/


#ifndef LOG_H
#define LOG_H

#define LOG_LINE_SIZE 1024
#define TAG "bruce >>> "

#define LOG_FATAL    0
#define LOG_ERROR    1
#define LOG_WARNING  2
#define LOG_INFO     3
#define LOG_DEBUG    4
#define LOG_TRACE    5

typedef struct AVClass {
    const char *name;
    const char* (*item_name)(void *av_class);
    const AVOption *option;
}AVClass;

void set_log_level(int level);
void debug_log(int flag, int level, const char *fmt, ...);
const char *av_default_item_name(void *ptr);

#endif
