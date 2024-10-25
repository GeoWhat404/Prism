#pragma once

#include <stdbool.h>
#include <boot/limine.h>

typedef struct {
    struct limine_framebuffer *lfb;
    struct limine_memmap_response *lmmr;
    struct limine_hhdm_response *lhhdmr;
    struct limine_file *lkrnl;
    struct limine_smbios_response *lsmbiosr;

    uint64_t kernel_virt_base;
    uint64_t kernel_phys_base;
    uint32_t cpu_count;

    bool heap;
} boot_info_t;

boot_info_t boot_info;
