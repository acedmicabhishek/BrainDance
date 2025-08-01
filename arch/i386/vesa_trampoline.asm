[BITS 32]

global vesa_trampoline_start
global vesa_trampoline_end
global vesa_trampoline_mode_ptr
global return_to_kernel

%define TRAMPOLINE_BASE 0x1000

; Offsets for data within the trampoline
%define saved_esp_offset (saved_esp - vesa_trampoline_start)
%define saved_cr0_offset (saved_cr0 - vesa_trampoline_start)
%define gdt_ptr_32_offset (gdt_ptr_32 - vesa_trampoline_start)
%define idt_ptr_32_offset (idt_ptr_32 - vesa_trampoline_start)
%define gdt_real_mode_ptr_offset (gdt_real_mode_ptr - vesa_trampoline_start)
%define mode_ptr_offset (vesa_trampoline_mode_ptr - vesa_trampoline_start)
%define gdt_real_mode_offset (gdt_real_mode - vesa_trampoline_start)
%define real_mode_code_offset (real_mode_code - vesa_trampoline_start)
%define protected_mode_code_offset (protected_mode_code - vesa_trampoline_start)

vesa_trampoline_start:
    cli ; Disable interrupts

    ; 1. Save protected mode state
    pushad
    mov [TRAMPOLINE_BASE + saved_esp_offset], esp

    sgdt [TRAMPOLINE_BASE + gdt_ptr_32_offset]
    sidt [TRAMPOLINE_BASE + idt_ptr_32_offset]

    mov eax, cr0
    mov [TRAMPOLINE_BASE + saved_cr0_offset], eax

    ; 2. Disabling paging.
    and eax, 0x7FFFFFFF ; Clear PG bit
    mov cr0, eax

    ; 3. Loading a temporary GDT for real mode.
    lgdt [TRAMPOLINE_BASE + gdt_real_mode_ptr_offset]

    ; 4. Switching to real mode.
    mov eax, cr0
    and al, 0xFE ; Clear PE bit
    mov cr0, eax
    jmp 0x08:(TRAMPOLINE_BASE + real_mode_code_offset) ; Far jump to flush pipeline

[BITS 16]
real_mode_code:
    mov ax, 0x10 ; Data segment selector for real mode
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9000 ; Set up a temporary stack

    ; 5. Calling BIOS interrupt 0x10 to set video mode.
    ; mov ax, 0x4F02
    ; mov bx, [TRAMPOLINE_BASE + mode_ptr_offset]
    ; int 0x10

    ; 6. Switching back to protected mode.
    lgdt [TRAMPOLINE_BASE + gdt_ptr_32_offset]

    mov eax, cr0
    or al, 0x01 ; Set PE bit
    mov cr0, eax

    jmp 0x08:(TRAMPOLINE_BASE + protected_mode_code_offset)

[BITS 32]
protected_mode_code:
    ; Restore segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; 7. Restoring protected mode state.
    mov eax, [TRAMPOLINE_BASE + saved_cr0_offset]
    or eax, 0x80000000 ; Re-enable paging
    mov cr0, eax

    lidt [TRAMPOLINE_BASE + idt_ptr_32_offset]

    mov esp, [TRAMPOLINE_BASE + saved_esp_offset]
    popad

    sti ; Re-enable interrupts
    ; 8. Returning to the caller.
    jmp return_to_kernel

; Data section
saved_esp: dd 0
saved_cr0: dd 0
gdt_ptr_32:
    dw 0
    dd 0
idt_ptr_32:
    dw 0
    dd 0

vesa_trampoline_mode_ptr: dw 0

; GDT for real mode transition
gdt_real_mode:
    dq 0 ; Null descriptor
gdt_real_code:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x9A
    db 0x0F
    db 0x00
gdt_real_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92
    db 0x0F
    db 0x00
gdt_real_mode_end:

gdt_real_mode_ptr:
    dw gdt_real_mode_end - gdt_real_mode - 1
    dd TRAMPOLINE_BASE + gdt_real_mode_offset

vesa_trampoline_end:

return_to_kernel:
    mov dword [0x7E00], 0xCAFEBABE
    cli
    hlt