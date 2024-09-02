#pragma once

#include <stdint.h>
#include <stdbool.h>

#define PIT_FREQ 18

void pit_initialize();
void pit_reset();
void pit_freeze();
void pit_unfreeze();
bool pit_is_frozen();

uint64_t pit_get_ticks();
uint64_t pit_get_seconds();
