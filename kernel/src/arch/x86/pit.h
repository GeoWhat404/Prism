#pragma once

#include <stdint.h>
#include <stdbool.h>

#define PIT_MAX_CALLBACKS   32

#define PIT_COUNTER_0       0x40
#define PIT_CMD             0x43

#define PIT_CMD_BINARY      0x00
#define PIT_CMD_BCD         0x01

#define PIT_CMD_MODE_0      0x00
#define PIT_CMD_MODE_1      0x02
#define PIT_CMD_MODE_2      0x04
#define PIT_CMD_MODE_3      0x06
#define PIT_CMD_MODE_4      0x08
#define PIT_CMD_MODE_5      0x0A

#define PIT_CMD_LATCH       0x00
#define PIT_CMD_RW_LOW      0x10
#define PIT_CMD_RW_HIGH     0x20
#define PIT_CMD_RW_BOTH     0x30

#define PIT_CMD_COUNTER_0   0x00
#define PIT_CMD_COUNTER_1   0x40
#define PIT_CMD_COUNTER_2   0x80
#define PIT_CMD_READBACK    0xC0
#define PIT_FREQUENCY       1193182

typedef void (*pfn_pit_callback)(uint32_t ticks);

void pit_initialize(void);
void pit_reset(void);
void pit_freeze(void);
void pit_unfreeze(void);
bool pit_is_frozen(void);

void pit_set_freq(uint32_t new_freq);

uint64_t pit_get_ticks(void);
uint64_t pit_get_seconds(void);

void pit_register_callback(pfn_pit_callback callback);
