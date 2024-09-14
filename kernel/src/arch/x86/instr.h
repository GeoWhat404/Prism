#pragma once

#include <stdint.h>

static inline uint64_t read_cr3() {
    uint64_t val;
    __asm__ volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

static inline void write_cr3(uint64_t cr3) {
    __asm__ volatile("movq %[cr3_val], %%cr3" ::[cr3_val] "r"(cr3));
}

static inline void flush_tlb(uintptr_t virt) {
    __asm__ volatile("invlpg (%0)" ::"r"(virt) : "memory");
}
