#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <hal/fb.h>
#include <hal/hal.h>
#include <hal/pit.h>
#include <hal/rtc.h>
#include <hal/panic.h>
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

void init_mmu(void) {
    mem_init();

    mem_print_layout();

    printf("MMU Components Initialized\n");
    log_info(MODULE_MAIN, "MMU Initialized");
}

void init_systems(void) {
    fb_initialize(boot_info.lfb);

    printf("Prism v%s on %s\n", STRINGIFY(OS_VERSION), STRINGIFY(ARCH));
    log_info(MODULE_MAIN, "Prism Kernel loaded");

    hal_initialize();

    printf("HAL Initialized\n");

    printf("\n--- < cpu detection > ---\n");
    detect_cpu();
    printf("--- < cpu detection > ---\n\n");

    init_mmu();

    datetime_t time = rtc_get_datetime();
    printf("Current date and time (RTC): %u:%u:%u %u/%u/%u\n",
           time.hours, time.minutes, time.seconds,
           time.days, time.months, time.years);
}

void kmain(void) {
    init_systems();
    printf("Initial setup complete in %llus\n", pit_get_seconds());

    for (;;);
}
