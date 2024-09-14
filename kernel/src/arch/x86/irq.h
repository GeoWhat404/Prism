#pragma once

#include "isr.h"

#define IRQ0 0
#define IRQ1 1
#define IRQ2 2
#define IRQ8 8

typedef void (*pfn_irq_handler)(registers_t *regs);

void irq_handle_interrupt(registers_t *regs);
void irq_initialize(void);
void irq_register_handler(int irq, pfn_irq_handler handler);
