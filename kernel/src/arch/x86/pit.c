#include "pit.h"
#include "irq.h"

#include <stdio.h>

static uint64_t ticks = 0;
static bool running = true;

static void pit_callback(registers_t *regs) {
    if (running)
        ticks++;
}

void pit_initialize() {
    irq_register_handler(IRQ0, pit_callback);
}

void pit_reset() {
    ticks = 0;
}

void pit_freeze() {
    running = false;
}

void pit_unfreeze() {
    running = true;
}

bool pit_is_frozen() {
    return running;
}

uint64_t pit_get_ticks() {
    return ticks;
}

uint64_t pit_get_seconds() {
    return (uint64_t) (ticks / PIT_FREQ);
}
