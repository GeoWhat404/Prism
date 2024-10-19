#pragma once

#include <stdio.h>
#include <util/debug.h>

#define printf_nl(fmt, ...)         \
    printf(fmt, ##__VA_ARGS__);     \
    printf("\n")

#define kinfo(fmt, ...)                         \
    do { log_info(fmt, ##__VA_ARGS__);          \
         printf_nl(fmt, ##__VA_ARGS__);         \
    } while(0)

#define kwarn(fmt, ...)             \
    do { log_warn(fmt, ##__VA_ARGS__);          \
         printf_nl(fmt, ##__VA_ARGS__);         \
    } while(0)

#define kerror(fmt, ...)            \
    do { log_error(fmt, ##__VA_ARGS__);          \
         printf_nl(fmt, ##__VA_ARGS__);         \
    } while(0)

#define kdebug(fmt, ...)            \
    do { log_debug(fmt, ##__VA_ARGS__);          \
         printf_nl(fmt, ##__VA_ARGS__);         \
    } while(0)

