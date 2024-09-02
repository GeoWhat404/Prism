#pragma once

#include <stdint.h>

typedef struct {
    uint32_t seconds;
    uint32_t minutes;
    uint32_t hours;
    uint32_t days;
    uint32_t months;
    uint32_t years;
} datetime_t;
