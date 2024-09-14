#include "pit.h"
#include "irq.h"

#include <stdio.h>

static uint64_t ticks = 0;
static bool running = true;

static void pit_callback(registers_t *regs) {
    if (running)
        ticks++;
}

void pit_initialize(void) {
    irq_register_handler(IRQ0, pit_callback);
}

void pit_reset(void) {
    ticks = 0;
}

void pit_freeze(void) {
    running = false;
}

void pit_unfreeze(void) {
    running = true;
}

bool pit_is_frozen(void) {
    return running;
}

uint64_t pit_get_ticks(void) {
    return ticks;
}

uint64_t pit_get_seconds(void) {
    return (uint64_t) (ticks / PIT_FREQ);
}
