#include "paging.h"
#include "mem.h"
#include <boot/boot.h>
#include <hal/panic.h>
#include <util/debug.h>
#include <util/defines.h>
#include <util/spinlock.h>

static uint64_t *page_dir = 0;

void paging_initialize() {
    uint64_t page_dir_phys = 0;
    __asm__ volatile("movq %%cr3, %0" : "=r"(page_dir_phys));
    if (!page_dir_phys) {
        log_error(MODULE_MMU, "Failed to load first page directory");
        panic();
    }
    uint64_t page_dir_virt = page_dir_phys + boot_info.lhhdmr->offset;
    page_dir = (uint64_t *)page_dir_virt;

    log_debug(MODULE_MMU, "Page Directory starts at 0x%llx", page_dir_phys);
}

spinlock_count_t wlock_paging = {0};
size_t virtual_to_physical(size_t virt) {
    if (!page_dir)
        return 0;

    if (virt >= boot_info.lhhdmr->offset &&
        virt <= (boot_info.lhhdmr->offset + memory_get_total())) {
        return virt - boot_info.lhhdmr->offset;
    }

    size_t virt_addr_init = virt;
    virt &= ~0xFFF;
    virt = AMD64_MM_STRIPSX(virt);

    uint32_t pml4_idx   = PML4E(virt);
    uint32_t pdp_idx    = PDPTE(virt);
    uint32_t pd_idx     = PDE(virt);
    uint32_t pt_idx     = PTE(virt);

    spinlock_count_read_acquire(&wlock_paging);
    if (!(page_dir[pml4_idx] & PF_PRESENT))
        goto error;

    size_t *pdp = (size_t *)(PTE_GET_ADDR(page_dir[pml4_idx])
            + boot_info.lhhdmr->offset);

    if (!(pdp[pdp_idx] & PF_PRESENT))
        goto error;

    size_t *pd = (size_t *)(PTE_GET_ADDR(pdp[pdp_idx])
            + boot_info.lhhdmr->offset);

    if (!(pd[pd_idx] & PF_PRESENT))
        goto error;

    size_t *pt = (size_t *)(PTE_GET_ADDR(pd[pd_idx])
                            + boot_info.lhhdmr->offset);

    if (pt[pt_idx] & PF_PRESENT) {
        spinlock_count_read_release(&wlock_paging);
        return (size_t)(PTE_GET_ADDR(pt[pt_idx])
                        + ((size_t)virt_addr_init & 0xFFF));
    }

error:
    spinlock_count_read_release(&wlock_paging);
    return 0;
}
