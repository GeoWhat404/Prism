#pragma once

#include <boot/limine.h>

typedef struct {
    struct limine_framebuffer *lfb;
    struct limine_memmap_response *lmmr;

    uint64_t kernel_virt_base;
    uint64_t kernel_phys_base;
} boot_info_t;

boot_info_t boot_info;
