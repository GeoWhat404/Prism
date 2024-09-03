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

    ; Save instruction pointer, code segment, and error code
    ; Note: pushq %cs and pushq %rip in AT&T assembly are not directly translatable.
    ; Typically, these are captured via the interrupt/exception mechanism and pushed automatically.
    ; However, assuming this is for an interrupt handler where RIP and CS are already pushed:
    ; This part needs to be customized as per your interrupt mechanism setup.

    ; Assuming %rip and %cs are pushed automatically by the CPU on an interrupt.
    ; Also assuming that an error code is pushed (you may need to adjust depending on your use case).

    ; If needed, manually adjust the stack pointer to align with the registers_t structure.
    ; mov rdi, rsp  ; Create a pointer to the registers_t struct and call panic
    mov rdi, rsp
    call dump_regs

    cli
    hlt

