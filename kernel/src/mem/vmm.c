#include "vmm.h"
#include "pmm.h"

#include <string.h>

#include <mem/mem.h>
#include <mem/paging.h>

#include <util/math.h>
#include <util/debug.h>
#include <util/spinlock.h>

#include <boot/boot.h>

#include <hal/panic.h>

#define VMM_POS_ENSURE 0x40000000

void vmm_initialize() {
    size_t target_pos = CEIL(boot_info.lhhdmr->offset -
                             memory_get_total() -
                             VMM_POS_ENSURE,
                             PAGE_SIZE) * PAGE_SIZE;
    virt.intialized = false;
    virt.mem_start = target_pos;
    virt.block_size = CEIL(memory_get_total(), BLOCK_SIZE);
    virt.byte_size = CEIL(virt.block_size, BLOCKS_PER_BYTE);

    uint64_t pages_required = CEIL(virt.byte_size, BLOCK_SIZE);
    virt.bytes = (uint8_t *)vmm_allocate(pages_required);

    memset(virt.bytes, 0, virt.byte_size);

    virt.intialized = true;
}

spinlock_t lock_vmm = ATOMIC_FLAG_INIT;
void *vmm_allocate(int pages) {
    spinlock_acquire(&lock_vmm);
    size_t p = (size_t) bitmap_allocate(&phys, pages);
    spinlock_release(&lock_vmm);

    if (!p) {
        log_error(MODULE_VMM, "Physical kernel memory ran out");
        panic("Out of memory");
    }

    uint64_t output = p + boot_info.lhhdmr->offset;
    return (void *)output;
}

void *vmm_allocate_contiguous(int pages) {
    return vmm_allocate(pages);
}

bool vmm_free(void *ptr, int pages) {
    size_t p = virtual_to_physical((size_t)ptr);

    if (!p) {
        log_error(MODULE_VMM, "Could not find physical address for %lx", ptr);
        panic("Unkown phys addr for 0x%016lx", ptr);
    }
    spinlock_acquire(&lock_vmm);
    bitmap_set_region(&phys, (void *)p, pages * BLOCK_SIZE, 0);
    spinlock_release(&lock_vmm);

    return true;
}
