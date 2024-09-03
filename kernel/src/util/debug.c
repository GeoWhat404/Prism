#include "debug.h"

#include <stdio.h>

static const char *const log_severity_colors[] = {
    [LVL_DEBUG]         = "\033[34m",
    [LVL_INFO]          = "\033[32m",
    [LVL_WARN]          = "\033[33m",
    [LVL_ERROR]         = "\033[31m",
    [LVL_FATAL]         = "\033[39;41m",
};

static const char *const log_severity_names[] = {
    [LVL_DEBUG]         = "DEBUG",
    [LVL_INFO]          = "INFO",
    [LVL_WARN]          = "WARN",
    [LVL_ERROR]         = "ERROR",
    [LVL_FATAL]         = "FATAL",
};

static const char *const color_reset = "\033[0m";

void debugf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vfprintf(VFS_FD_DEBUG, fmt, args);

    va_end(args);
}

void vlogf(const char *module, int level, const char *fmt, va_list ap) {
    if (level < MIN_LOG_LEVEL)
        return;

    // change color to the one associateed with the severity of the log
    fputs(log_severity_colors[level], VFS_FD_DEBUG);

    // print the severity and module
    fprintf(VFS_FD_DEBUG, "[%s][%s]: ", log_severity_names[level], module);

    // print the message
    vfprintf(VFS_FD_DEBUG, fmt, ap);

    // reset the color
    fprintf(VFS_FD_DEBUG, "%s\n", color_reset);

}

void logf(const char *module, int level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vlogf(module, level, fmt, args);

    va_end(args);
}
