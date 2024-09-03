#include "paging.h"
#include "mem.h"
#include "pmm.h"

#include <string.h>

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
        panic("Could not load first page directory");
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

void paging_change_page_dir_unsafe(uint64_t *pd) {
    uint64_t targ = virtual_to_physical((size_t)pd);
    if (!targ) {
        panic("paging: could not change pd");
    }
    __asm__ volatile("movq %0, %%cr3" :: "r"(targ));
    page_dir = pd;
}

void paging_change_page_dir(uint64_t *pd) {
    paging_change_page_dir_unsafe(pd);
}

size_t paging_virt_alloc_phys() {
    size_t p = bitmap_allocate_page_frame(&phys);
    void *v = (void *)(p + boot_info.lhhdmr->offset);
    memset(v, 0, PAGE_SIZE);

    return p;
}

void invalidate(uint64_t v) {
    __asm__ volatile("invlpg %0" :: "m"(v));
}

void paging_map(uint64_t virt, uint64_t phys, uint64_t flags) {
    if (virt % PAGE_SIZE) {
        panic("paging: non-aligned address");
    }

    virt = AMD64_MM_STRIPSX(virt);

    uint32_t pml4_index = PML4E(virt);
    uint32_t pdp_index = PDPTE(virt);
    uint32_t pd_index = PDE(virt);
    uint32_t pt_index = PTE(virt);

    spinlock_count_read_acquire(&wlock_paging);
    	if(!(page_dir[pml4_index] & PF_PRESENT)) {
    		size_t target = paging_virt_alloc_phys();
    		page_dir[pml4_index] = target | PF_PRESENT | PF_RW | PF_USER;
    }
    size_t *pdp = (size_t *)(PTE_GET_ADDR(page_dir[pml4_index]) +
                             boot_info.lhhdmr->offset);

	    if(!(pdp[pdp_index] & PF_PRESENT)) {
    		size_t target = paging_virt_alloc_phys();
    		pdp[pdp_index] = target | PF_PRESENT | PF_RW | PF_USER;
    }
    size_t *pd = (size_t *)(PTE_GET_ADDR(pdp[pdp_index]) +
                            boot_info.lhhdmr->offset);

	    if(!(pd[pd_index] & PF_PRESENT)) {
    		size_t target = paging_virt_alloc_phys();
    		pd[pd_index] = target | PF_PRESENT | PF_RW | PF_USER;
    }
    size_t *pt = (size_t *)(PTE_GET_ADDR(pd[pd_index]) +
                            boot_info.lhhdmr->offset);

    if(pt[pt_index] & PF_PRESENT)
        log_warn(MODULE_MMU, "Overwriting without unmapping 0x%016lx->0x%016lx",
                 phys, virt);

    pt[pt_index] = (P_PHYS_ADDR(phys)) | PF_PRESENT | flags;

    invalidate(virt);
    spinlock_count_write_release(&wlock_paging);
}
