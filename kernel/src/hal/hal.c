#include "hal.h"

#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"

#include <util/debug.h>

void hal_initialize() {
    gdt_initialize();
    log_debug(MODULE_HAL, "GDT Initialized");

    isr_initialize();
    log_debug(MODULE_HAL, "ISR Initialized");

    idt_initialize();
    log_debug(MODULE_HAL, "IDT Initialized");

    irq_initialize();
    log_debug(MODULE_HAL, "IRQ Initialized");
}
