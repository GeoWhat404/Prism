#include "idt.h"
#include "gdt.h"
#include "port.h"
#include <stdint.h>
#include <stddef.h>
#include <util/debug.h>
#include <util/binary.h>
#include <util/defines.h>

typedef struct {
    uint16_t offset_1;          // Offset bits 0->15
    uint16_t selector;          // Code segment selector in the GDT
    uint8_t ist;                // Interrupt Stack Table
    uint8_t attribs;            // gate type, DPL, and P fields
    uint16_t offset_2;          // offset bits 16->31
    uint32_t offset_3;          // offset bits 32->63
    uint32_t reserved;          // always 0
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idt_descriptor_t;

idt_entry_t idt[IDT_ENTRIES];
extern uint64_t _isr1;

void idt_load() {
    idt_descriptor_t idt_desc;
    idt_desc.base = (size_t)&idt;
    idt_desc.limit = IDT_ENTRIES * sizeof(idt_entry_t) - 1;
    // Load the new IDT
    __asm__ volatile("lidt %0" : : "m"(idt_desc));

    // Set the interrupt flag
    __asm__ volatile("sti");
}

void idt_set_gate(int interrupt, uint64_t handler, uint8_t flags) {
    if (idt[interrupt].selector != 0)
        log_warn(MODULE_INTRPT, "Overriding previous IDT entry for 0x%x", interrupt);
    idt[interrupt].offset_1 = handler & 0xFFFF;
    idt[interrupt].selector = GDT_KERNEL_CODE;
    idt[interrupt].ist = 0;
    idt[interrupt].attribs = flags;
    idt[interrupt].offset_2 = (handler >> 16) & 0xFFFF;
    idt[interrupt].offset_3 = (handler >> 32) & 0xFFFFFFFF;
    idt[interrupt].reserved = 0;
}

void idt_initialize() {
    idt_load();
}

