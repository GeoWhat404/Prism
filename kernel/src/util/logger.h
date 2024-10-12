#pragma once

#include <stdio.h>
#include <util/debug.h>

#define printf_nl(fmt, ...)         \
    printf(fmt, ##__VA_ARGS__);     \
    printf("\n")

#define kinfo(fmt, ...)             \
    log_info(fmt, ##__VA_ARGS__);   \
    printf_nl(fmt, ##__VA_ARGS__)

#define kwarn(fmt, ...)             \
    log_warn(fmt, ##__VA_ARGS__);   \
    printf_nl(fmt, ##__VA_ARGS__)

#define kerror(fmt, ...)            \
    log_error(fmt, ##__VA_ARGS__);  \
    printf_nl(fmt, ##__VA_ARGS__)

#define kdebug(fmt, ...)            \
    log_debug(fmt, ##__VA_ARGS__);  \
    printf_nl(fmt, ##__VA_ARGS__)
