#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <hal/fb.h>
#include <hal/hal.h>
#include <hal/pit.h>
#include <hal/rtc.h>
#include <hal/panic.h>
#include <hal/instr.h>
#include <hal/detect.h>

#include <util/debug.h>
#include <util/defines.h>
#include <util/datetime.h>

#include <mem/mem.h>
#include <mem/pmm.h>
#include <mem/vmm.h>
#include <mem/heap.h>

#include <boot/boot.h>
#include <boot/limine.h>

#include <util/logger.h>

void init_mmu(void) {
    mem_init();

    mem_print_layout();

    kinfo("MMU Components Initialized");
}

void init_systems(void) {
    fb_initialize(boot_info.lfb);

    kinfo("Prism v%s on %s", STRINGIFY(OS_VERSION), STRINGIFY(ARCH));

    hal_initialize();
    detect_cpu();
    init_mmu();

    kinfo("HHDM offset: 0x%016llx", boot_info.lhhdmr->offset);

    datetime_t time = rtc_get_datetime();
    kinfo("Current date and time (RTC): %u:%u:%u %u/%u/%u",
           time.hours, time.minutes, time.seconds,
           time.days, time.months, time.years);
}

void kmain(void) {
    init_systems();
    kinfo("Initial setup complete in %llus", pit_get_seconds());
    panic("He!");

    while (1) {
        hlt();
    }
}
