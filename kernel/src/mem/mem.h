#pragma once

#include <stdint.h>
#include <stddef.h>

#include <boot/boot.h>

#define PAGE_BYTE_SIZE (4096ull)

typedef void       *virt_addr_t;
typedef uintptr_t   phys_addr_t;

typedef struct {
    virt_addr_t virt;
    size_t size;
} mem_bitmap_t;

void mem_init(void);
void mem_print_layout(void);
uint64_t mem_get_size(void);

static inline virt_addr_t mem_phys_to_virt(const phys_addr_t phys) {
    return (virt_addr_t)(phys + boot_info.lhhdmr->offset);
}

static inline phys_addr_t mem_virt_to_phys(const virt_addr_t virt) {
    return (phys_addr_t)((uintptr_t)virt - boot_info.lhhdmr->offset);
}
