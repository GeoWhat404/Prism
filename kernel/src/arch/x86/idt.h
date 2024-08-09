#pragma once
#include <stdint.h>

#define IDT_ENTRIES 256

void idt_initialize();
void idt_load();
void idt_set_gate(int interrupt, uint64_t handler, uint8_t flags);
