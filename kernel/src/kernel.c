#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <hal/hal.h>
#include <hal/pit.h>
#include <hal/rtc.h>
#include <hal/panic.h>
#include <hal/instr.h>
#include <hal/detect.h>

#include <util/logger.h>
#include <util/defines.h>
#include <util/datetime.h>

#include <mem/mem.h>
#include <mem/pmm.h>
#include <mem/vmm.h>
#include <mem/heap.h>

#include <boot/boot.h>
#include <boot/limine.h>

#include <drivers/fb/fb.h>
#include <graphics/graphics.h>
#include <graphics/font/font.h>

#include <util/colors.h>

static graphics_ctx_t *g_ctx;

void init_mmu(void) {
    mem_init();
    kinfo("MMU Components Initialized");
}

void init_systems(void) {
    enable_sse2();
    init_mmu();
}

void print_mem(void) {
    mem_print_layout();
    heap_print();
}

void print_info(void) {
    kinfo("Prism v%s on %s (compilation: %s)", STRINGIFY(OS_VERSION), STRINGIFY(ARCH), __DATE__);

    hal_initialize();

    detect_cpu();

    print_mem();

    kinfo("HHDM offset: 0x%016llx", boot_info.lhhdmr->offset);

    datetime_t time = rtc_get_datetime();
    kinfo("Current date and time (RTC): %u:%u:%u %u/%u/%u",
           time.hours, time.minutes, time.seconds,
           time.days, time.months, time.years);

    kinfo("Initial setup complete in %llus", pit_get_seconds());
}

void kmain(void) {
    init_systems();

    struct font font;

    if (psf2_load_font(&font) != 0)
        panic("failed to load psf2 font");

    if (graphics_init(&font) != 0)
        panic("failed to initialize the graphics library");

    g_ctx = graphics_get_ctx(DOUBLE, 0, 0, graphics_get_screen_width(),
                                           graphics_get_screen_height());

    if (!g_ctx)
        kmalloc_null_error();
    fb_init(g_ctx, graphics_get_ctx_height(g_ctx) / graphics_get_font_height(),
            graphics_get_ctx_width(g_ctx) / graphics_get_font_width());

    print_info();

    while (1) {
        hlt();
    }
}
