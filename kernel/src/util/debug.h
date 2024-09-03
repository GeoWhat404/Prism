#pragma once

#include <stdio.h>

#define MIN_LOG_LEVEL LVL_DEBUG

#define MODULE_MAIN     "KERNEL"
#define MODULE_HAL      "HAL"
#define MODULE_INTRPT   "INTRPT"
#define MODULE_MMU      "MMU"
#define MODULE_VMM      "VMM"
#define MODULE_PMM      "PMM"
#define MODULE_HEAP     "HEAP"
#define MODULE_SPINLOCK "SPINLOCK"

enum {
    LVL_DEBUG = 0,
    LVL_INFO = 1,
    LVL_WARN = 2,
    LVL_ERROR = 3,
    LVL_FATAL = 4,
};

/**
 * Writes the specified message to VFS\_FD\_DBG
 */
void debugf(const char *fmt, ...);

/**
 * Writes the specified msg + va_list in the following format to VFS\_FD\_DBG
 *      [lvl][module]: <formatted text>
 */
void vlogf(const char *module, int level, const char *fmt, va_list ap);
/**
 * Writes the specified message in the following format to VFS\_FD\_DBG
 *      [lvl][module]: msg
 */
void logf(const char *module, int lvl, const char *fmt, ...);

#define log_debug(module, ...) logf(module, LVL_DEBUG, __VA_ARGS__)
#define log_info(module, ...) logf(module, LVL_INFO, __VA_ARGS__)
#define log_warn(module, ...) logf(module, LVL_WARN, __VA_ARGS__)
#define log_error(module, ...) logf(module, LVL_ERROR, __VA_ARGS__)
#define log_fatal(module, ...) logf(module, LVL_FATAL, __VA_ARGS__)
