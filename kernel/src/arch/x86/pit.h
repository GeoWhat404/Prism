#pragma once

#include <stdint.h>
#include <stdbool.h>

#define PIT_FREQ 18

void pit_initialize(void);
void pit_reset(void);
void pit_freeze(void);
void pit_unfreeze(void);
bool pit_is_frozen(void);

uint64_t pit_get_ticks(void);
uint64_t pit_get_seconds(void);
