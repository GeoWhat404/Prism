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

#include <boot/boot.h>
#include <boot/limine.h>

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".requests")))
static volatile struct limine_paging_mode_request paging_request = {
    .id = LIMINE_PAGING_MODE_REQUEST,
    .revision = 0,
    .mode = LIMINE_PAGING_MODE_X86_64_4LVL
};

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_kernel_address_request kernel_addr_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_kernel_file_request kernel_file_request = {
    .id = LIMINE_KERNEL_FILE_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

static void hcf(void) {
    panic("halt catch fire");
}

void map_pages() {
    // framebuffer
}

void init_mmu() {

    mem_bitmap_t bitmap = pmm_initialize();

    vmm_init(bitmap);
    vmm_print_memmap();

    printf("MMU Components Initialized\n");
    log_info(MODULE_MAIN, "MMU Initialized");
}

void init_systems() {
    fb_initialize(boot_info.lfb);

    printf("Prism v%s on %s\n", STRINGIFY(OS_VERSION), STRINGIFY(ARCH));
    log_info(MODULE_MAIN, "Prism Kernel loaded");

    printf("\n--- < cpu detection > ---\n");
    detect_cpu();
    printf("--- < cpu detection > ---\n\n");

    hal_initialize();
    printf("HAL Initialized\n");

    init_mmu();

    datetime_t time = rtc_get_datetime();
    printf("Current date and time (RTC): %u:%u:%u %u/%u/%u\n",
           time.hours, time.minutes, time.seconds,
           time.days, time.months, time.years);

    printf("Initial setup complete in %llus\n", pit_get_seconds());
}

void _start(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    struct limine_paging_mode_response *lpagingr = paging_request.response;
    if (lpagingr->mode != LIMINE_PAGING_MODE_X86_64_4LVL) {
        panic("lvl4 paging not supported");
    }

    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *lfb = framebuffer_request.response->framebuffers[0];
    struct limine_memmap_response *lmmr = memmap_request.response;
    struct limine_kernel_address_response *kernel_addr = kernel_addr_request.response;
    struct limine_kernel_file_response *lkrnl_filer = kernel_file_request.response;
    struct limine_hhdm_response *lhhdmr = hhdm_request.response;

    boot_info.lfb = lfb;
    boot_info.lmmr = lmmr;
    boot_info.lhhdmr = lhhdmr;
    boot_info.lkrnl = lkrnl_filer->kernel_file;
    boot_info.kernel_phys_base = kernel_addr->physical_base;
    boot_info.kernel_virt_base = kernel_addr->virtual_base;

    init_systems();

    for (;;);
}
