[BITS 32]

global vga_trampoline_start
global vga_trampoline_end

%define BASE 0x7000

vga_trampoline_start:
    cli
    ; Save ESP, CR0, GDT, IDT
    mov [BASE + 0x00], esp
    sgdt [BASE + 0x10]
    sidt [BASE + 0x20]
    mov eax, cr0
    mov [BASE + 0x30], eax

    ; Disable paging
    and eax, 0x7FFFFFFF
    mov cr0, eax

    ; Get current position
    call .get_ip
.get_ip:
    pop ebx
    sub ebx, .get_ip - vga_trampoline_start

    ; Load real-mode GDT
    lea eax, [ebx + gdt_real_ptr]
    lgdt [eax]

    ; Clear PE bit
    mov eax, cr0
    and al, 0xFE
    mov cr0, eax

    ; Far jump to real mode code
    lea eax, [ebx + jmp_real_mode]
    jmp far [eax]

[BITS 16]
real_mode_code:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Debug: Write 'V' to prove we're in real mode
    mov byte [0xB8000], 'V'
    mov byte [0xB8001], 0x1F

    ; Set VGA mode
    mov ax, 0x13
    int 0x10

    ; Back to protected mode
    lgdt [BASE + 0x10]
    mov eax, [BASE + 0x30]
    or al, 0x01
    mov cr0, eax

    lea eax, [ebx + jmp_protected_mode]
    jmp far [eax]

[BITS 32]
protected_mode_resume:
    mov esp, [BASE + 0x00]
    lidt [BASE + 0x20]
    sti
    ret

gdt_real_mode:
    dq 0                 ; null
gdt_real_code:
    dw 0xFFFF, 0x0000, 0x00, 0x9A, 0x0F, 0x00
gdt_real_data:
    dw 0xFFFF, 0x0000, 0x00, 0x92, 0x0F, 0x00
gdt_real_end:

gdt_real_ptr:
    dw gdt_real_end - gdt_real_mode - 1
    dd BASE + gdt_real_mode - vga_trampoline_start

jmp_real_mode:
    dd real_mode_code
    dw 0x08

jmp_protected_mode:
    dd protected_mode_resume
    dw 0x08

vga_trampoline_end: