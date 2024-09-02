#include "hal.h"

#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "pit.h"
#include "rtc.h"

#include <mem/paging.h>
#include <util/debug.h>

void hal_initialize() {
    gdt_initialize();
    log_debug(MODULE_HAL, "GDT Initialized");

    idt_initialize();
    log_debug(MODULE_HAL, "IDT Initialized");

    isr_initialize();
    log_debug(MODULE_HAL, "ISR Initialized");

    irq_initialize();
    log_debug(MODULE_HAL, "IRQ Initialized");

    pit_initialize();
    log_debug(MODULE_HAL, "PIT Initialized");

    rtc_initialize();
    log_debug(MODULE_HAL, "RTC Initialized");

    //paging_initialize();
    log_debug(MODULE_HAL, "Paging Initialized");
}
