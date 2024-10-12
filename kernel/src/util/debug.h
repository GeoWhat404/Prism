#pragma once

#include <stdio.h>

#define MIN_LOG_LEVEL LVL_DEBUG

// colors
#define RES "\e[0m"

#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"

// bold
#define B_BLK "\e[1;30m"
#define B_RED "\e[1;31m"
#define B_GRN "\e[1;32m"
#define B_YEL "\e[1;33m"
#define B_BLU "\e[1;34m"
#define B_MAG "\e[1;35m"
#define B_CYN "\e[1;36m"
#define B_WHT "\e[1;37m"


// foreground
#define B_RED_RED "\e[31;49;1m"

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
 * Writes the specified message in the following format to VFS\_FD\_DBG
 *      [lvl][module]: msg
 */
void logf(const char *file, int line, int level, const char *fmt, ...);

#define log_debug(fmt, ...) logf(__FILE__,  __LINE__, LVL_DEBUG, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...) logf(__FILE__, __LINE__, LVL_INFO, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...) logf(__FILE__, __LINE__, LVL_WARN, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) logf(__FILE__, __LINE__, LVL_ERROR, fmt, ##__VA_ARGS__)
