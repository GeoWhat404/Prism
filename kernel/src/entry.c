#include "kernel.h"

#include <stdbool.h>

#include <hal/panic.h>
#include <boot/boot.h>
#include <boot/limine.h>

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_paging_mode_request paging_mode_request = {
    .id = LIMINE_PAGING_MODE_REQUEST,
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

__attribute__((used, section(".requests")))
static volatile struct limine_smbios_request smbios_request = {
    .id = LIMINE_SMBIOS_REQUEST,
    .revision = 0,
};

__attribute__((used, section(".requests")))
static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0,
};

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

static void hcf(void) {
    panic("halt catch fire");
}

void _start(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    struct limine_paging_mode_response *lpagingr = paging_mode_request.response;
    if (lpagingr->mode != LIMINE_PAGING_MODE_X86_64_4LVL) {
        panic("lvl4 paging not supported");
    }

    if (framebuffer_request.response == 0
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *lfb = framebuffer_request.response->framebuffers[0];
    struct limine_memmap_response *lmmr = memmap_request.response;
    struct limine_kernel_address_response *kernel_addr = kernel_addr_request.response;
    struct limine_kernel_file_response *lkrnl_filer = kernel_file_request.response;
    struct limine_hhdm_response *lhhdmr = hhdm_request.response;
    struct limine_smbios_response *lsmbiosr = smbios_request.response;
    struct limine_smp_response *lsmpr = smp_request.response;

    boot_info.lfb = lfb;
    boot_info.lmmr = lmmr;
    boot_info.lhhdmr = lhhdmr;
    boot_info.lkrnl = lkrnl_filer->kernel_file;
    boot_info.lsmbiosr = lsmbiosr;
    boot_info.kernel_phys_base = kernel_addr->physical_base;
    boot_info.kernel_virt_base = kernel_addr->virtual_base;
    boot_info.cpu_count = lsmpr->cpu_count;

    kmain();

    panic("kmain() returned");
}
