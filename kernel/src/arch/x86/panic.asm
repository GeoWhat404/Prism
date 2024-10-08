%macro PUSH_ALL 0
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

global asm_dump_regs
extern dump_regs
asm_dump_regs:
    cli

    push 0                  ; to pad out the struct
    push 0                  ; cuz these are intrpt specific

    PUSH_ALL
    mov rbp, ds
    push rbp

    mov rdi, rsp            ; Create a pointer to the registers_t struct
    call dump_regs

    cli
    hlt
