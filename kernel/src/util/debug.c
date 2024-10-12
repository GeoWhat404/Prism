#include "debug.h"

#include <stdio.h>

static const char *const log_severity_colors[] = {
    [LVL_DEBUG]         = B_MAG,
    [LVL_INFO]          = B_CYN,
    [LVL_WARN]          = B_YEL,
    [LVL_ERROR]         = B_RED,
};

static const char *const log_severity_names[] = {
    [LVL_DEBUG]         = "KDEBUG",
    [LVL_INFO]          = "KINFO",
    [LVL_WARN]          = "KWARN",
    [LVL_ERROR]         = "KERROR",
};

static const char *const color_reset = "\033[0m";

void debugf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vfprintf(VFS_FD_DEBUG, fmt, args);

    va_end(args);
}

void logf(const char *file, int line, int level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    if (level < MIN_LOG_LEVEL)
        return;

    // change color to the one associateed with the severity of the log
    fputs(log_severity_colors[level], VFS_FD_DEBUG);

    // print the severity and module
    fprintf(VFS_FD_DEBUG, "[%s] " RES, log_severity_names[level], file, line);
    fprintf(VFS_FD_DEBUG, B_WHT "%s line: %d: " WHT, file, line);

    // print the message
    vfprintf(VFS_FD_DEBUG, fmt, args);

    // reset the color
    fprintf(VFS_FD_DEBUG, "%s\n", color_reset);

    va_end(args);
}
