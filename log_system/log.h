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

void set_log_level(int level);
void debug_log(int flag, int level, const char *fmt, ...);

#endif
