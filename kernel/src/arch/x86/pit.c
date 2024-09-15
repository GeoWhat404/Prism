#include "pit.h"
#include "irq.h"

#include <stdio.h>
#include <util/debug.h>

static uint64_t ticks = 0;
static bool running = true;

static uint32_t callback_count = 0;
static pfn_pit_callback timer_callbacks[PIT_MAX_CALLBACKS];

static void pit_callback(registers_t *regs) {
    if (running) {
        for (uint32_t i = 0; i < callback_count; i++)
            timer_callbacks[i](ticks);
        ticks++;
    }
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

void pit_register_callback(pfn_pit_callback callback) {
    if (callback_count + 1 > PIT_MAX_CALLBACKS) {
        log_warn(MODULE_HAL, "Max PIT callbacks reached");
    }
    timer_callbacks[callback_count++] = callback;
}
