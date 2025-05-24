#include "idt.h"
#include "isr.h"
#include "panic.h"

#include <util/logger.h>

extern void *isr128;
extern void *isr_stub_table[];

pfn_isr_handler isr_handlers[ISR_HANDLER_COUNT];

char *exceptions[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"};

void isr_initialize() {
    // ISR exceptions 0 - 31
    for (int i = 0; i < 48; i++) {
        idt_set_gate(i, (uint64_t)isr_stub_table[i], IDT_DESCRIPTOR_EXCEPTION);
    }

    // Syscalls having DPL 3
    idt_set_gate(IDT_SYSCALL, (uint64_t)isr128, 0xEE);
}

void isr_handle_interrupt(uint64_t rsp) {
    registers_t *cpu = (registers_t *)rsp;

    if (cpu->interrupt == IDT_SYSCALL) {
        // ...
        panic("Operation not implemented");
    }

    if (isr_handlers[cpu->interrupt] == 0) {
        panic("%s (0x%llx)",
              exceptions[cpu->interrupt], cpu->interrupt);
    }
    isr_handlers[cpu->interrupt](cpu);
}

void isr_register_handler(int interrupt, pfn_isr_handler handler) {
    if (handler == 0) {
        kwarn("Tried to register a null isr handler");
        return;
    }

    if (interrupt > ISR_HANDLER_COUNT) {
        kwarn("Interrupt exceeds bounds of isr handler array");
        return;
    }

    isr_handlers[interrupt] = handler;
}
