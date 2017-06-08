#ifndef LOG_H
#define LOG_H

#define VERBOSE 1
#define DEBUG   2
#define INFO    3
#define WARN    4
#define ERROR   5
#define FATAL   6

#ifdef DEBUG_1
#include "debug.h"
#define LOG(level, TAG, ...) ((void)log_printf(level, TAG, __VA_ARGS__))

#else
#include <stdio.h>
#define LOG(VERBOSE, LOG_TAG, __VA_ARGS__) ((void)printf(__VA_ARGS__))

#endif

#define LOG_TAG "test_bruce"

#define LOGV(...) LOG(VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGD(...) LOG(DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) LOG(INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) LOG(WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) LOG(ERROR, LOG_TAG, __VA_ARGS__)

#endif
