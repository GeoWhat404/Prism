#pragma once

#include <stdint.h>
#include <util/datetime.h>

void rtc_initialize();
datetime_t rtc_get_datetime();
