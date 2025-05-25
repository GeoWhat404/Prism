#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <hal/panic.h>

struct _source_location {
    const char* file_name;
    uint32_t line_number;
    const char* function_name;
};

#define CUR_SOURCE_LOCATION (struct _source_location){__FILE__, __LINE__, __func__}

void do_assert(bool expr, const struct _source_location loc, const char *expression);

#define assert(Expr) do_assert(Expr, CUR_SOURCE_LOCATION, #Expr)
