#pragma once
#include <stdint.h>

#define IDT_ENTRIES 256

#define IDT_DESCRIPTOR_X16_INTERRUPT	0x06
#define IDT_DESCRIPTOR_X16_TRAP 		0x07
#define IDT_DESCRIPTOR_X32_TASK 		0x05
#define IDT_DESCRIPTOR_X32_INTERRUPT  	0x0E
#define IDT_DESCRIPTOR_X32_TRAP			0x0F
#define IDT_DESCRIPTOR_RING1  			0x40
#define IDT_DESCRIPTOR_RING2  			0x20
#define IDT_DESCRIPTOR_RING3  			0x60
#define IDT_DESCRIPTOR_PRESENT			0x80

#define IDT_DESCRIPTOR_EXCEPTION        (IDT_DESCRIPTOR_PRESENT | IDT_DESCRIPTOR_X32_INTERRUPT)
#define IDT_SYSCALL                     0x80

void idt_initialize(void);
void idt_load(void);
void idt_set_gate(int interrupt, uint64_t handler, uint8_t flags);
