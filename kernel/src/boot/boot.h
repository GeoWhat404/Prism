#pragma once

#include <boot/limine.h>

typedef struct {
    struct limine_framebuffer *lfb;
    struct limine_memmap_response *lmmr;
} boot_info_t;
