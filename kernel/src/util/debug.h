#pragma once

#include <stdio.h>

#define MIN_LOG_LEVEL LVL_DEBUG

enum {
    LVL_DEBUG = 0,
    LVL_INFO = 1,
    LVL_WARN = 2,
    LVL_ERROR = 3,
};

/**
 * Writes the specified message to VFS\_FD\_DBG
 */
void debugf(const char *fmt, ...);

/**
 * Writes the message formatted to VFS\_FD\_DBG
 */
void logf(const char *file, int line, int level, const char *fmt, ...);

/**
* Writes the message formatted to VFS\_FD\_STDOUT (printf)
*/
void log_printf(int level, const char *fmt, ...);

#define log_debug(fmt, ...) logf(__FILE__,  __LINE__, LVL_DEBUG, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...) logf(__FILE__, __LINE__, LVL_INFO, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...) logf(__FILE__, __LINE__, LVL_WARN, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) logf(__FILE__, __LINE__, LVL_ERROR, fmt, ##__VA_ARGS__)
