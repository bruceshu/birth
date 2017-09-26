#ifndef LOG_H
#define LOG_H

#define LINE_SIZE 1024
#define ERROR 1
#define WARNING 2
#define INFO 3
#define DEBUG 4

void set_log_level(int level);
void debug_log(int level, const char *fmt, ...);

#endif
