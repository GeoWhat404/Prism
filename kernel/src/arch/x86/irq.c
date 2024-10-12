#include "irq.h"
#include "isr.h"
#include "port.h"
#include "i8259.h"
#include "instr.h"

#include <stdio.h>
#include <stddef.h>
#include <util/logger.h>

pfn_irq_handler handlers[16];

void irq_handle_interrupt(registers_t *regs) {
    uint32_t irq = regs->interrupt - PIC_REMAP_OFFSET;

    if (handlers[irq] == NULL) {
        kwarn("Unhandled IRQ: %d", irq);
    } else {
        handlers[irq](regs);
    }
    i8259_send_eoi(irq);
}

static void kbd_callback(registers_t *regs) {
    inb(0x60);
}

void irq_initialize(void) {
    i8259_remap(PIC_REMAP_OFFSET);

    i8259_mask_all();

    if (!i8259_probe()) {
        kwarn("No PIC found!");
    } else {
        kinfo("Found an i8258 PIC!");
    }

    // They should be already enabled but just in case...
    sti();

    for (int i = 0; i < 16; i++)
        isr_register_handler(i + PIC_REMAP_OFFSET, irq_handle_interrupt);

    i8259_unmask(IRQ0);
    i8259_unmask(IRQ1);
    i8259_unmask(IRQ2);
    i8259_unmask(IRQ8);

    irq_register_handler(IRQ1, kbd_callback);
    outb(0x64, 0xAE);
    io_wait();
}

void irq_register_handler(int irq, pfn_irq_handler handler) {
    handlers[irq] = handler;
}
