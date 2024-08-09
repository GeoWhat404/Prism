#include "irq.h"
#include "isr.h"
#include "pic.h"
#include "port.h"

#include <stdio.h>
#include <stddef.h>
#include <util/debug.h>

pfn_irq_handler handlers[16];

void irq_handle_interrupt(registers_t *regs) {
    log_debug(MODULE_INTRPT, "Called to handle - Returning to abandon (PLEASE SEND HELP)");
    uint32_t irq = regs->interrupt - PIC_REMAP_OFFSET;

    if (handlers[irq] == NULL) {
        printf("Unhandled IRQ: %d\n", irq);
        log_warn(MODULE_INTRPT, "Unhandled IRQ: %d", irq);
    } else {
        handlers[irq](regs);
    }
    pic_send_eoi(irq);
}

static void pit_timer(registers_t *regs) {
    (void)regs;

    log_debug(MODULE_INTRPT, "TIMER!");
    printf("TICK!");
}

void irq_initialize() {
    pic_unmask_all();

    outb(0x43, (uint8_t) ((3 << 4) | (1 << 2) | 0));

    if (!pic_probe()) {
        log_error(MODULE_INTRPT, "No PIC found!");
    }

    // They should be already enabled but just in case...
    __asm__ volatile("sti");

    for (int i = PIC_REMAP_OFFSET; i < PIC_REMAP_OFFSET + 15; i++)
        isr_register_handler(i, irq_handle_interrupt);

    irq_register_handler(IRQ0, pit_timer);
    irq_register_handler(32, pit_timer);
}

void irq_register_handler(int irq, pfn_irq_handler handler) {
    handlers[irq] = handler;
}
