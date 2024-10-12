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
    log_info("GDT Initialized");

    idt_initialize();
    log_info("IDT Initialized");

    isr_initialize();
    log_info("ISR Initialized");

    irq_initialize();
    log_info("IRQ Initialized");

    pit_initialize();
    log_info("PIT Initialized");

    rtc_initialize();
    log_info("RTC Initialized");
}
