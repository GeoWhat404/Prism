#include "hal.h"

#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "pit.h"
#include "rtc.h"

#include <util/debug.h>

void hal_initialize(void) {
    gdt_initialize();
    log_info(MODULE_HAL, "GDT Initialized");

    idt_initialize();
    log_info(MODULE_HAL, "IDT Initialized");

    isr_initialize();
    log_info(MODULE_HAL, "ISR Initialized");

    irq_initialize();
    log_info(MODULE_HAL, "IRQ Initialized");

    pit_initialize();
    log_info(MODULE_HAL, "PIT Initialized");

    rtc_initialize();
    log_info(MODULE_HAL, "RTC Initialized");
}
