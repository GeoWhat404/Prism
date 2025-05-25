#include "kernel.h"

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
#include <drivers/ps2/keyboard.h>
#include <graphics/graphics.h>
#include <graphics/gfx.h>
#include <graphics/font/font.h>

#include <util/colors.h>

#include <shell.h>

void init_mmu() {
    mem_init();
    kinfo("MMU Components Initialized");
}

void init_systems() {
    enable_sse2();
    init_mmu();
}

void init_keyboard() {
    // for now just do the ps2 for emulator debugging

    bool ps2 = ps2_keyboard_initialize();
    if (ps2) {
        ps2_keyboard_enable();
    }

    if (!ps2) {
        kerror("The system will continue without a keyboard!");
    }
}

void print_mem() {
    mem_print_layout();
    heap_print();
}

void print_info() {
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

void init_graphics() {
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
}

void kmain() {
    init_systems();
    init_graphics();
    print_info();
    init_keyboard();
    shell_launch();

    kinfo("Shell returned");

    while (1) {
        hlt();
    }
}
