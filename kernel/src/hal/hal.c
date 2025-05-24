#include "hal.h"

#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "pit.h"
#include "rtc.h"

#include <util/logger.h>

void hal_initialize() {
    gdt_initialize();
    kinfo("GDT Initialized");

    idt_initialize();
    kinfo("IDT Initialized");

    isr_initialize();
    kinfo("ISR Initialized");

    irq_initialize();
    kinfo("IRQ Initialized");

    pit_initialize();
    kinfo("PIT Initialized");

    rtc_initialize();
    kinfo("RTC Initialized");
}
