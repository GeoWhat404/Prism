#pragma once

#include <stdint.h>
#include <util/datetime.h>

void rtc_initialize(void);
datetime_t rtc_get_datetime(void);
