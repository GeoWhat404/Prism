#include "isr.h"
#include "idt.h"
#include "irq.h"

#include "pic.h"
#include "port.h"

#include <stdio.h>
#include <util/debug.h>

extern void asm_isr_exit();
extern void *isr_stub_table[];
extern void isr128();

static pfn_isr_handler handlers[32];

const char *exception_msgs[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
	"Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
	"No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
	"Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
	"Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
	"Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
	"Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
	"Reserved",
    "Reserved"
};

void reg_dump(registers_t *regs) {
    printf("RIP=0x%llx\n", regs->rip);
    printf("RSI=0x%llx\n", regs->rsi);
    printf("--\n");
    printf("RAX=0x%llx\tR8=0x%llx\n"
           "RBX=0x%llx\tR9=0x%llx\n"
           "RCX=0x%llx\tR10=0x%llx\n"
           "RDX=0x%llx\tR11=0x%llx\n"
           "RBP=0x%llx\tR12=0x%llx\n"
           "CS=0x%llx\tR13=0x%llx\n"
           "DS=0x%llx\tR14=0x%llx\n"
           "RF=0x%llx\tR15=0x%llx\n",
           regs->rax, regs->r8,
           regs->rbx, regs->r9,
           regs->rcx, regs->r10,
           regs->rdx, regs->r11,
           regs->rbp, regs->r12,
           regs->cs, regs->r13,
           regs->ds, regs->r14,
           regs->rflags, regs->r15
    );
}

void panic(registers_t *regs) {
    const char *exception = exception_msgs[regs->interrupt];

    uint64_t error_pos;
    __asm__ volatile("movq %%cr2, %0" : "=r"(error_pos));

    printf("---- [ begin panic ] ----\n");
    printf("Interrupt: 0x%x\n", regs->interrupt);
    printf("Unhandled Exception: %s\n", exception);
    printf("At CR2=0x%llx\n");
    printf("-- [ register dump ] --\n");
    reg_dump(regs);
    printf("---- [ end panic ] ----\n");

    log_fatal(MODULE_INTRPT, "!!! KERNEL PANIC !!!");
    log_fatal(MODULE_INTRPT, "Interrupt: 0x%x", regs->interrupt);
    log_fatal(MODULE_INTRPT, "Exception: %s", exception);
    __asm__("cli; hlt");
}

void hndlr() {
    log_debug(MODULE_INTRPT, "test");
}

void isr_initialize() {
    pic_remap(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 0x08);
    pic_mask_all();
    pic_unmask(IRQ2);

    for (int i = 0; i < 48; i++) {
        idt_set_gate(i, (uint64_t)(isr_stub_table[i]), 0x8E);
    }

    idt_set_gate(0x80, (uint64_t)isr128, 0xEE);
}

void isr_handle_interrupt(uint64_t rsp) {
    log_debug(MODULE_INTRPT, "isr_handle_interrupt");
    registers_t *regs = (registers_t *)rsp;
    if (handlers[regs->interrupt] == 0)
        panic(regs);
    handlers[regs->interrupt](regs);
}

void isr_register_handler(int interrupt, pfn_isr_handler handler) {
    if (handlers[interrupt]) {
        log_warn(MODULE_INTRPT, "Overriding old ISR handler for int: 0x%x",
                 interrupt);
    }
    handlers[interrupt] = handler;
}
