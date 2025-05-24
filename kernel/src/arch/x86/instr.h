#pragma once

#include <stdint.h>

static inline uint64_t read_cr0() {
    uint64_t val;
    __asm__ volatile("mov %%cr0, %0" : "=r"(val));
    return val;
}

static inline void write_cr0(uint64_t cr2) {
    __asm__ volatile("movq %[cr0_val], %%cr0" ::[cr0_val] "r"(cr2));
}

static inline uint64_t read_cr2() {
    uint64_t val;
    __asm__ volatile("mov %%cr2, %0" : "=r"(val));
    return val;
}

static inline void write_cr2(uint64_t cr2) {
    __asm__ volatile("movq %[cr2_val], %%cr2" ::[cr2_val] "r"(cr2));
}

static inline uint64_t read_cr3() {
    uint64_t val;
    __asm__ volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

static inline void write_cr3(uint64_t cr3) {
    __asm__ volatile("movq %[cr3_val], %%cr3" ::[cr3_val] "r"(cr3));
}

static inline uint64_t read_cr4() {
    uint64_t val;
    __asm__ volatile("mov %%cr4, %0" : "=r"(val));
    return val;
}

static inline void write_cr4(uint64_t cr4) {
    __asm__ volatile("movq %[cr4_val], %%cr4" ::[cr4_val] "r"(cr4));
}

static inline void flush_tlb(void *virt) {
    __asm__ volatile("invlpg (%0)" ::"r"((uintptr_t)virt) : "memory");
}

static inline void cli() {
    __asm__ volatile("cli");
}

static inline void sti() {
    __asm__ volatile("sti");
}

static inline void hlt() {
    __asm__ volatile("hlt");
}

static inline void enable_sse2() {
	uint64_t cr0 = read_cr0();
	cr0 &= ~(1 << 2); // Clear CR0.EM bit
	cr0 |= (1 << 1);  // Set CR0.MP bit
	write_cr0(cr0);

	uint64_t cr4 = read_cr4();
	cr4 |= (1 << 9);  // Set CR4.OSFXSR bit
	cr4 |= (1 << 10); // Set CR4.OSXMMEXCPT bit
	write_cr4(cr4);
}
