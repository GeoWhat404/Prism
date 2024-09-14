#pragma once

#include <stdbool.h>

#define CMOS_ADDR_REG   0x70
#define CMOS_DATA_REG   0x71

bool cmos_is_busy(void);
