#pragma once

#include <stdio.h>
#include <util/debug.h>

#define kinfo(fmt, ...)                             \
    do { log_info(fmt, ##__VA_ARGS__);              \
         log_printf(LVL_INFO, fmt, ##__VA_ARGS__);  \
    } while(0)

#define kwarn(fmt, ...)                             \
    do { log_warn(fmt, ##__VA_ARGS__);              \
         log_printf(LVL_WARN, fmt, ##__VA_ARGS__);  \
    } while(0)

#define kerror(fmt, ...)                            \
    do { log_error(fmt, ##__VA_ARGS__);             \
         log_printf(LVL_ERROR, fmt, ##__VA_ARGS__); \
    } while(0)

#define kdebug(fmt, ...)                            \
    do { log_debug(fmt, ##__VA_ARGS__);             \
         log_printf(LVL_DEBUG, fmt, ##__VA_ARGS__); \
    } while(0)

