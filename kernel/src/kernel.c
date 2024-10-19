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

static graphics_ctx_t *g_ctx;

void init_mmu(void) {
    mem_init();
    heap_print();

    mem_print_layout();

    kinfo("MMU Components Initialized");
}

void init_systems(void) {
    enable_sse2();
    init_mmu();

    // NOTE: for whatever reason when i call this code from a seperate function
    // the _psf2_putc function pointer gets messed up.
    // so maybe fix this soontm?
    struct font font;
    if (psf2_load_font(&font) != 0) {
        panic("failed to load psf2 font");
    }

    if (graphics_init(&font) != 0) {
        panic("failed to initialize the graphics library");
    }

    g_ctx = graphics_get_ctx(DOUBLE, 0, 0, graphics_get_screen_width(),
                             graphics_get_screen_height());

    if (!g_ctx) {
        panic("graphics context");
    }
    fb_init(g_ctx, graphics_get_ctx_height(g_ctx) / graphics_get_font_height(),
            graphics_get_ctx_width(g_ctx) / graphics_get_font_width());

    kinfo("Prism v%s on %s", STRINGIFY(OS_VERSION), STRINGIFY(ARCH));

    hal_initialize();

    detect_cpu();

    kinfo("HHDM offset: 0x%016llx", boot_info.lhhdmr->offset);

    datetime_t time = rtc_get_datetime();
    kinfo("Current date and time (RTC): %u:%u:%u %u/%u/%u",
           time.hours, time.minutes, time.seconds,
           time.days, time.months, time.years);
}

void kmain(void) {
    init_systems();
    kinfo("Initial setup complete in %llus", pit_get_seconds());

    graphics_swap_buffer(g_ctx);
    while (1) {
        hlt();
    }
}
