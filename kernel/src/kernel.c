#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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

#include <drivers/fb.h>
#include <graphics/graphics.h>
#include <graphics/font/font.h>

void init_graphics() {

    }

void init_mmu(void) {
    mem_init();
    heap_print();

    mem_print_layout();

    kinfo("MMU Components Initialized");
}

void init_systems(void) {
    enable_sse2();
    init_mmu();
    struct font font;
    if (psf2_load_font(&font) != 0) {
        panic("This wont show 99%");
    }

    if (graphics_init(&font) != 0) {
        panic("Will this even show?");
    }

    GRAPHICS_CONTEXT *ctx = graphics_get_ctx(DOUBLE, 0, 0, get_screen_width(), get_screen_height());
    if (ctx == 0)
        log_error("CTX ZERO!");
    fb_init(ctx, get_ctx_height(ctx) / get_font_height(),
            get_ctx_width(ctx) / get_font_width());



    kinfo("Prism v%s on %s", STRINGIFY(OS_VERSION), STRINGIFY(ARCH));

    hal_initialize();

    detect_cpu();

    kinfo("HHDM offset: 0x%016llx", boot_info.lhhdmr->offset);

    datetime_t time = rtc_get_datetime();
    kinfo("Current date and time (RTC): %u:%u:%u %u/%u/%u",
           time.hours, time.minutes, time.seconds,
           time.days, time.months, time.years);

    while (1) {
        hlt();
    }
}

void kmain(void) {
    init_systems();
    kinfo("Initial setup complete in %llus", pit_get_seconds());

    while (1) {
        hlt();
    }
}
