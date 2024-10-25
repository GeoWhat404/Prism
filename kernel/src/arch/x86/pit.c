#include "pit.h"
#include "irq.h"
#include "i8259.h"
#include "port.h"
#include "instr.h"

#include <util/logger.h>

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

    cli();

    uint32_t hz = 1000;
    uint32_t divisor = PIT_FREQUENCY / hz;

    irq_register_handler(IRQ0, pit_callback);
    i8259_unmask(IRQ0);

    outb(PIT_CMD, PIT_CMD_BINARY |
                  PIT_CMD_MODE_3 |
                  PIT_CMD_RW_BOTH |
                  PIT_CMD_COUNTER_0);
    outb(PIT_COUNTER_0, divisor);
    outb(PIT_COUNTER_0, divisor >> 8);

    sti();
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

void pit_set_freq(uint32_t new_freq) {
    cli();

    outb(0x40, new_freq & 0xFF);
    outb(0x40, (new_freq & 0xFF) >> 8);

    sti();
}

uint64_t pit_get_ticks(void) {
    return ticks;
}

uint64_t pit_get_seconds(void) {
    return (uint64_t) (ticks / (PIT_FREQUENCY / 1000));
}

void pit_register_callback(pfn_pit_callback callback) {
    if (callback_count + 1 > PIT_MAX_CALLBACKS) {
        kwarn("Max PIT callbacks reached");
    }
    timer_callbacks[callback_count++] = callback;
}
