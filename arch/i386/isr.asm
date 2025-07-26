[bits 32]
%macro ISR_NOERR 1
global isr%1
isr%1:
    cli
    push dword 0
    push dword %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERR 1
global isr%1
isr%1:
    cli
    push dword %1
    jmp isr_common_stub
%endmacro

section .text

; CPU exceptions
ISR_NOERR 0   ; Divide by zero
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8   ; Double fault
ISR_NOERR 9
ISR_ERR  10
ISR_ERR  11
ISR_ERR  12
ISR_ERR  13
ISR_ERR  14   ; Page fault
ISR_NOERR 15
ISR_NOERR 16
ISR_NOERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

extern isr_handler

isr_common_stub:
    pusha                ; Push all general-purpose registers
    mov eax, [esp + 32]  ; Get the interrupt number from the stack
    mov ebx, [esp + 36]  ; Get the error code from the stack
    push ebx             ; Push error code as the second argument for the C handler
    push eax             ; Push interrupt number as the first argument
    call isr_handler     ; Call the C interrupt handler
    add esp, 8           ; Clean up the pushed arguments
    popa                 ; Pop all general-purpose registers
    add esp, 8           ; Clean up the interrupt number and error code
    sti                  ; Re-enable interrupts
    iret                 ; Return from the interrupt
